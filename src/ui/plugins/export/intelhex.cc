/****************************************************************************/
/*                                                                          */
/* HexBed -- Hex editor                                                     */
/* Copyright (c) 2021-2022 Sampo Hippeläinen (hisahi)                       */
/*                                                                          */
/* This program is free software: you can redistribute it and/or modify     */
/* it under the terms of the GNU General Public License as published by     */
/* the Free Software Foundation, either version 3 of the License, or        */
/* (at your option) any later version.                                      */
/*                                                                          */
/* This program is distributed in the hope that it will be useful,          */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/* GNU General Public License for more details.                             */
/*                                                                          */
/* You should have received a copy of the GNU General Public License        */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>.   */
/*                                                                          */
/****************************************************************************/
// ui/plugins/export/intelhex.cc -- impl for builtin Intel HEX exporter

#include "ui/plugins/export/intelhex.hh"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/translation.h>
#include <wx/valgen.h>

#include <fstream>
#include <iomanip>
#include <iostream>

#include "common/hexconv.hh"
#include "common/intconv.hh"
#include "common/logger.hh"

namespace hexbed {

namespace plugins {

constexpr bufsize SEGMENTED_MAX_LENGTH = 0x10FFF0;

class ExportPluginIntelHEXDialog : public wxDialog {
  public:
    ExportPluginIntelHEXDialog(wxWindow* parent,
                               ExportPluginIntelHEXSettings& settings,
                               bufsize size)
        : wxDialog(parent, wxID_ANY, _("Intel HEX"), wxDefaultPosition,
                   wxDefaultSize, wxDEFAULT_DIALOG_STYLE) {
        SetReturnCode(wxID_CANCEL);

        wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);

        wxStdDialogButtonSizer* buttons = new wxStdDialogButtonSizer();
        wxButton* okButton = new wxButton(this, wxID_OK);
        wxButton* cancelButton = new wxButton(this, wxID_CANCEL);

        buttons->SetAffirmativeButton(okButton);
        buttons->SetNegativeButton(cancelButton);
        buttons->Realize();

        wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
        wxSpinCtrl* spin =
            new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                           wxDefaultSize, wxSP_ARROW_KEYS, 1, 256, 16);
        row->Add(new wxStaticText(this, wxID_ANY, _("Bytes per row"),
                                  wxDefaultPosition, wxDefaultSize,
                                  wxST_ELLIPSIZE_END),
                 wxSizerFlags().Center().Proportion(1));
        row->AddStretchSpacer();
        row->Add(spin, wxSizerFlags().Proportion(1));
        spin->SetValidator(
            wxGenericValidator(reinterpret_cast<int*>(&settings.columns)));
        top->Add(row, wxSizerFlags().Expand());

        if (size < SEGMENTED_MAX_LENGTH)
            top->Add(new wxCheckBox(this, wxID_ANY, _("x86 segment mode"),
                                    wxDefaultPosition, wxDefaultSize, 0,
                                    wxGenericValidator(&settings.segmentMode)),
                     wxSizerFlags().Proportion(1));
        else {
            wxCheckBox* check =
                new wxCheckBox(this, wxID_ANY, _("x86 segment mode"),
                               wxDefaultPosition, wxDefaultSize, 0);
            top->Add(check, wxSizerFlags().Proportion(1));
            check->Enable(false);
        }
        top->Add(buttons);

        SetSizer(top);
        Fit();
        Layout();
    }
};

ExportPluginIntelHEX::ExportPluginIntelHEX(pluginid id)
    : LocalizableExportPlugin(
          /// format name, perhaps should not be translated
          id, TAG("Intel HEX")) {}

wxString ExportPluginIntelHEX::getFileFilter() const {
    return _("Intel HEX file (*.hex, *.h86)") + "|*.hex;*.h86|" +
           ExportPlugin::getFileFilter();
}

bool ExportPluginIntelHEX::configureExport(wxWindow* parent,
                                           const std::string& filename,
                                           bufsize size) {
    if (size >> 32) {
        wxMessageBox(_("The exported section is 4 GiB (2^32 bytes) or longer, "
                       "which is too long for Intel HEX files."),
                     "HexBed", wxOK | wxICON_ERROR);
        return false;
    }
    ExportPluginIntelHEXDialog dial(parent, settings_, size);
    dial.TransferDataToWindow();
    int result = dial.ShowModal();
    if (result != wxID_OK) return false;
    dial.TransferDataFromWindow();
    return true;
}

static void outRow(std::ofstream& f, byte* row, std::size_t len) {
    HEXBED_ASSERT(len < 261);
    byte checksum = 0;
    for (std::size_t i = 0; i < len; ++i) checksum += row[i];
    row[len] = -checksum;
    f << ':';
    for (std::size_t i = 0; i <= len; ++i)
        f << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
          << static_cast<unsigned>(row[i]);
    f << '\n';
}

static std::size_t prepareOutRow(byte* row, byte size, bufsize address,
                                 byte type) {
    row[0] = size;
    uintToBytes<std::uint16_t>(2, &row[1], address & 0xFFFFU, false);
    row[3] = type;
    return 4;
}

static std::size_t prepareSegOffRow(byte* row, bufsize address) {
    std::size_t n = prepareOutRow(row, 4, 0, 2);
    uintToBytes<std::uint16_t>(2, &row[n], address >> 4, false);
    return n + 2;
}

static std::size_t prepareHiOffRow(byte* row, bufsize address) {
    std::size_t n = prepareOutRow(row, 4, 0, 4);
    uintToBytes<std::uint16_t>(2, &row[n], address >> 16, false);
    return n + 2;
}

void ExportPluginIntelHEX::doExport(
    HexBedTask& task, const std::string& filename,
    std::function<bufsize(bufsize, bytespan)> read, bufsize actualOffset,
    bufsize size) {
    bool segmented = settings_.segmentMode && size < SEGMENTED_MAX_LENGTH;
    std::ofstream f(filename);
    byte row[261];
    unsigned bc = settings_.columns & 255;
    if (!bc) bc = 16;
    std::size_t s = static_cast<std::size_t>(size);
    constexpr std::size_t pageSize = 0x10000UL;
    bool wideMode = size >> 16;

    for (std::size_t page = 0; !task.isCancelled() && page < s;
         page += pageSize) {
        if (wideMode) {
            if (segmented)
                outRow(f, row, prepareSegOffRow(row, page));
            else
                outRow(f, row, prepareHiOffRow(row, page));
        }
        std::size_t pageEnd = std::min(s, page + pageSize);
        for (std::size_t offset = page; offset < pageEnd;) {
            std::size_t remaining = std::min<std::size_t>(bc, pageEnd - offset);
            std::size_t o = prepareOutRow(row, remaining, offset, 0);
            std::size_t n = read(offset, bytespan{row + o, remaining});
            outRow(f, row, o + n);
            offset += n;
            if (!n) throw std::runtime_error("unknown read error");
        }
    }
    outRow(f, row, prepareOutRow(row, 0, 0, 1));
}

};  // namespace plugins

};  // namespace hexbed
