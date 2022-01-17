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
// plugins/export/intelhex.cc -- impl for builtin Intel HEX exporter

#include "plugins/export/intelhex.hh"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/msgdlg.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/translation.h>
#include <wx/valgen.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>

#include "app/config.hh"
#include "common/hexconv.hh"
#include "common/intconv.hh"
#include "common/logger.hh"
#include "plugins/dialog.hh"
#include "ui/settings/validate.hh"

namespace hexbed {

namespace plugins {

constexpr bufsize SEGMENTED_MAX_LENGTH = 0x10FFF0;

class ExportPluginIntelHEXDialog : public PluginConfigureDialog<true> {
  private:
    ExportPluginIntelHEXSettings* settings_;
    bufsize off_;
    bufsize sz_;

  public:
    ExportPluginIntelHEXDialog(wxWindow* parent,
                               ExportPluginIntelHEXSettings& settings,
                               bufsize actualOffset, bufsize size)
        : PluginConfigureDialog<true>(parent, wxID_ANY, _("Intel HEX"),
                                      wxDefaultPosition, wxDefaultSize,
                                      wxDEFAULT_DIALOG_STYLE),
          settings_(&settings),
          off_(actualOffset),
          sz_(size) {
        SetReturnCode(wxID_CANCEL);

        wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);

        wxStdDialogButtonSizer* buttons = new wxStdDialogButtonSizer();
        wxButton* okButton = new wxButton(this, wxID_OK);
        wxButton* cancelButton = new wxButton(this, wxID_CANCEL);
        okButton->Bind(wxEVT_BUTTON, &ExportPluginIntelHEXDialog::OnOK, this);

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
        spin->SetValidator(hexbed::ui::GenericCustomValidator<unsigned, int>(
            &settings.columns));
        top->Add(row, wxSizerFlags().Expand());

        top->Add(new wxStaticText(this, wxID_ANY, _("Mode"), wxDefaultPosition,
                                  wxDefaultSize, wxST_ELLIPSIZE_END),
                 wxSizerFlags().Expand());
        top->Add(new wxRadioButton(
                     this, wxID_ANY, _("I8 (8080)"), wxDefaultPosition,
                     wxDefaultSize, wxRB_GROUP,
                     hexbed::ui::RadioValidator<ExportPluginIntelHEXMode>(
                         &settings.mode, ExportPluginIntelHEXMode::I8)),
                 wxSizerFlags().Expand());
        top->Add(new wxRadioButton(
                     this, wxID_ANY, _("I16 (8086)"), wxDefaultPosition,
                     wxDefaultSize, 0,
                     hexbed::ui::RadioValidator<ExportPluginIntelHEXMode>(
                         &settings.mode, ExportPluginIntelHEXMode::I16)),
                 wxSizerFlags().Expand());
        top->Add(new wxRadioButton(
                     this, wxID_ANY, _("I32 (80386)"), wxDefaultPosition,
                     wxDefaultSize, 0,
                     hexbed::ui::RadioValidator<ExportPluginIntelHEXMode>(
                         &settings.mode, ExportPluginIntelHEXMode::I32)),
                 wxSizerFlags().Expand());

        top->Add(new wxStaticText(this, wxID_ANY, _("Offset base"),
                                  wxDefaultPosition, wxDefaultSize,
                                  wxST_ELLIPSIZE_END),
                 wxSizerFlags().Expand());
        row = new wxBoxSizer(wxHORIZONTAL);
        top->Add(new wxRadioButton(
                     this, wxID_ANY, _("Zero"), wxDefaultPosition,
                     wxDefaultSize, wxRB_GROUP,
                     hexbed::ui::RadioValidator<ExportPluginIntelHEXOffsetBase>(
                         &settings.offsetBase,
                         ExportPluginIntelHEXOffsetBase::Zero)),
                 wxSizerFlags().Expand());
        top->Add(new wxRadioButton(
                     this, wxID_ANY,
                     wxString::Format(_("Selection (%s)"),
                                      wxString::Format("0x%llx", actualOffset)),
                     wxDefaultPosition, wxDefaultSize, 0,
                     hexbed::ui::RadioValidator<ExportPluginIntelHEXOffsetBase>(
                         &settings.offsetBase,
                         ExportPluginIntelHEXOffsetBase::Real)),
                 wxSizerFlags().Expand());
        row->Add(new wxRadioButton(
                     this, wxID_ANY, _("Custom:"), wxDefaultPosition,
                     wxDefaultSize, 0,
                     hexbed::ui::RadioValidator<ExportPluginIntelHEXOffsetBase>(
                         &settings.offsetBase,
                         ExportPluginIntelHEXOffsetBase::Custom)),
                 wxSizerFlags());
        row->AddStretchSpacer();
        wxSpinCtrl* spin2 =
            new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                           wxDefaultSize, wxSP_ARROW_KEYS, 0,
                           static_cast<int>(std::min<bufsize>(
                               std::numeric_limits<int>::max(), 0xFFFFFFFFUL)),
                           0);
        row->Add(spin2);
        spin2->SetValidator(hexbed::ui::GenericCustomValidator<bufsize, int>(
            &settings.offsetCustom));
        spin2->SetBase(config().offsetRadix);
        top->Add(row, wxSizerFlags().Expand());

        top->Add(buttons, wxSizerFlags().Expand());

        SetSizer(top);
        Fit();
        Layout();
    }

    bool CheckValid() noexcept override {
        using enum ExportPluginIntelHEXMode;
        using enum ExportPluginIntelHEXOffsetBase;
        bufsize maxOffset;
        switch (settings_->mode) {
        case I8:
            maxOffset = 0x10000UL;
            break;
        case I16:
            maxOffset = SEGMENTED_MAX_LENGTH;
            break;
        case I32:
            maxOffset = 0x100000000ULL;
            break;
        }
        bufsize offset;
        switch (settings_->offsetBase) {
        case Zero:
            offset = 0;
            break;
        case Real:
            offset = off_;
            break;
        case Custom:
            offset = settings_->offsetCustom;
            break;
        }
        if (offset + sz_ > maxOffset) {
            wxMessageBox(_("The exported offset would be out of range for the "
                           "given amount of data.\nDecrease the offset, try "
                           "another mode or select less data to export."),
                         "HexBed", wxOK | wxICON_ERROR);
            return false;
        }
        return true;
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

bool ExportPluginIntelHEX::configureExport(
    wxWindow* parent, const std::filesystem::path& filename,
    bufsize actualOffset, bufsize size, const ExportDetails& details) {
    if (size >> 32) {
        wxMessageBox(_("The exported section is 4 GiB (2^32 bytes) or longer, "
                       "which is too long for Intel HEX files."),
                     "HexBed", wxOK | wxICON_ERROR);
        return false;
    }
    ExportPluginIntelHEXDialog dial(parent, settings_, actualOffset, size);
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
    HexBedTask& task, const std::filesystem::path& filename,
    std::function<bufsize(bufsize, bytespan)> read, bufsize actualOffset,
    bufsize size, const ExportDetails& details) {
    using enum ExportPluginIntelHEXMode;
    using enum ExportPluginIntelHEXOffsetBase;
    bool segmented = settings_.mode == I16 && size < SEGMENTED_MAX_LENGTH;
    std::ofstream f(filename);
    byte row[261];
    unsigned bc = settings_.columns & 255;
    if (!bc) bc = 16;
    std::size_t s = static_cast<std::size_t>(size);
    constexpr std::size_t pageSize = 0x10000UL;
    constexpr std::size_t pageMask = pageSize - 1;
    bool wideMode = settings_.mode != I8;
    std::size_t base;

    switch (settings_.offsetBase) {
    case Zero:
        base = 0;
        break;
    case Real:
        base = static_cast<std::size_t>(actualOffset);
        break;
    case Custom:
        base = static_cast<std::size_t>(settings_.offsetCustom);
        break;
    }
    s += base;

    for (std::size_t page = base; !task.isCancelled() && page < s;
         page = (page + pageSize) & ~pageMask) {
        if (wideMode) {
            if (segmented)
                outRow(f, row, prepareSegOffRow(row, page));
            else
                outRow(f, row, prepareHiOffRow(row, page));
        }
        std::size_t pageEnd = std::min(s, (page + pageSize) & ~pageMask);
        for (std::size_t offset = page; offset < pageEnd;) {
            std::size_t remaining = std::min<std::size_t>(bc, pageEnd - offset);
            std::size_t o = prepareOutRow(row, remaining, offset, 0);
            std::size_t n = read(offset - base, bytespan{row + o, remaining});
            outRow(f, row, o + n);
            offset += n;
            if (!n) throw std::runtime_error("unknown read error");
        }
    }
    outRow(f, row, prepareOutRow(row, 0, 0, 1));
}

};  // namespace plugins

};  // namespace hexbed
