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
// plugins/export/srec.cc -- impl for builtin Motorola S-record exporter

#include "plugins/export/srec.hh"

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
#include "app/plugin.hh"
#include "common/hexconv.hh"
#include "common/intconv.hh"
#include "common/logger.hh"
#include "plugins/dialog.hh"
#include "ui/settings/validate.hh"

namespace hexbed {

namespace plugins {

class ExportPluginMotorolaSRECDialog : public PluginConfigureDialog<true> {
  private:
    ExportPluginMotorolaSRECSettings* settings_;
    bufsize off_;
    bufsize sz_;

  public:
    ExportPluginMotorolaSRECDialog(wxWindow* parent,
                                   ExportPluginMotorolaSRECSettings& settings,
                                   bufsize actualOffset, bufsize size)
        : PluginConfigureDialog<true>(parent, wxID_ANY, _("Motorola S-record"),
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
        okButton->Bind(wxEVT_BUTTON, &ExportPluginMotorolaSRECDialog::OnOK,
                       this);

        buttons->SetAffirmativeButton(okButton);
        buttons->SetNegativeButton(cancelButton);
        buttons->Realize();

        wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
        wxSpinCtrl* spin =
            new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                           wxDefaultSize, wxSP_ARROW_KEYS, 1, 240, 16);
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
                     this, wxID_ANY, _("S19 (16-bit)"), wxDefaultPosition,
                     wxDefaultSize, wxRB_GROUP,
                     hexbed::ui::RadioValidator<ExportPluginMotorolaSRECMode>(
                         &settings.mode, ExportPluginMotorolaSRECMode::S19)),
                 wxSizerFlags().Expand());
        top->Add(new wxRadioButton(
                     this, wxID_ANY, _("S28 (24-bit)"), wxDefaultPosition,
                     wxDefaultSize, 0,
                     hexbed::ui::RadioValidator<ExportPluginMotorolaSRECMode>(
                         &settings.mode, ExportPluginMotorolaSRECMode::S28)),
                 wxSizerFlags().Expand());
        top->Add(new wxRadioButton(
                     this, wxID_ANY, _("S37 (32-bit)"), wxDefaultPosition,
                     wxDefaultSize, 0,
                     hexbed::ui::RadioValidator<ExportPluginMotorolaSRECMode>(
                         &settings.mode, ExportPluginMotorolaSRECMode::S37)),
                 wxSizerFlags().Expand());

        top->Add(new wxStaticText(this, wxID_ANY, _("Offset base"),
                                  wxDefaultPosition, wxDefaultSize,
                                  wxST_ELLIPSIZE_END),
                 wxSizerFlags().Center());
        top->Add(
            new wxRadioButton(
                this, wxID_ANY, _("Zero"), wxDefaultPosition, wxDefaultSize,
                wxRB_GROUP,
                hexbed::ui::RadioValidator<ExportPluginMotorolaSRECOffsetBase>(
                    &settings.offsetBase,
                    ExportPluginMotorolaSRECOffsetBase::Zero)),
            wxSizerFlags().Expand());
        top->Add(
            new wxRadioButton(
                this, wxID_ANY,
                wxString::Format(_("Selection (%s)"),
                                 wxString::Format("0x%llx", actualOffset)),
                wxDefaultPosition, wxDefaultSize, 0,
                hexbed::ui::RadioValidator<ExportPluginMotorolaSRECOffsetBase>(
                    &settings.offsetBase,
                    ExportPluginMotorolaSRECOffsetBase::Real)),
            wxSizerFlags().Expand());
        row = new wxBoxSizer(wxHORIZONTAL);
        row->Add(
            new wxRadioButton(
                this, wxID_ANY, _("Custom:"), wxDefaultPosition, wxDefaultSize,
                0,
                hexbed::ui::RadioValidator<ExportPluginMotorolaSRECOffsetBase>(
                    &settings.offsetBase,
                    ExportPluginMotorolaSRECOffsetBase::Custom)),
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
        using enum ExportPluginMotorolaSRECMode;
        using enum ExportPluginMotorolaSRECOffsetBase;
        bufsize maxOffset;
        switch (settings_->mode) {
        case S19:
            maxOffset = 0x10000UL;
            break;
        case S28:
            maxOffset = 0x1000000UL;
            break;
        case S37:
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

ExportPluginMotorolaSREC::ExportPluginMotorolaSREC(pluginid id)
    : LocalizableExportPlugin(
          /// format name, perhaps should not be translated
          id, TAG("Motorola S-record")) {}

wxString ExportPluginMotorolaSREC::getFileFilter() const {
    return _("Motorola S-record file (*.s19, *.s28, *.s37)") +
           "|*.s19;*.s28;*.s37|" + ExportPlugin::getFileFilter();
}

bool ExportPluginMotorolaSREC::configureExport(
    wxWindow* parent, const std::filesystem::path& filename,
    bufsize actualOffset, bufsize size, const ExportDetails& details) {
    if (size >> 32) {
        wxMessageBox(_("The exported section is 4 GiB (2^32 bytes) or longer, "
                       "which is too long for Motorola S-record files."),
                     "HexBed", wxOK | wxICON_ERROR);
        return false;
    }
    ExportPluginMotorolaSRECDialog dial(parent, settings_, actualOffset, size);
    if (checkCaseInsensitiveFileExtension(filename, STRING(".s19")))
        settings_.mode = ExportPluginMotorolaSRECMode::S19;
    else if (checkCaseInsensitiveFileExtension(filename, STRING(".s28")))
        settings_.mode = ExportPluginMotorolaSRECMode::S28;
    else if (checkCaseInsensitiveFileExtension(filename, STRING(".s37")))
        settings_.mode = ExportPluginMotorolaSRECMode::S37;
    dial.TransferDataToWindow();
    int result = dial.ShowModal();
    if (result != wxID_OK) return false;
    dial.TransferDataFromWindow();
    return true;
}

#define OUTHEX(x)                                                   \
    std::hex << std::uppercase << std::setw(2) << std::setfill('0') \
             << static_cast<unsigned>(x)

template <std::size_t N>
static void outRow(std::ofstream& f, unsigned type, std::size_t len,
                   std::size_t addr, const byte* row) {
    HEXBED_ASSERT(type < 10 && len < 254 - N);
    byte truelen = len + N + 1;
    byte checksum = truelen;
    byte addrb[N];
    for (std::size_t i = 0; i < sizeof(addrb); ++i)
        checksum += (addrb[i] = static_cast<byte>(
                         addr >> (8 * (sizeof(addrb) - i - 1))));
    for (std::size_t i = 0; i < len; ++i) checksum += row[i];
    checksum = ~checksum;
    f << 'S' << type;
    f << OUTHEX(truelen);
    for (std::size_t i = 0; i < sizeof(addrb); ++i) f << OUTHEX(addrb[i]);
    for (std::size_t i = 0; i < len; ++i) f << OUTHEX(row[i]);
    f << OUTHEX(checksum);
    f << '\n';
}

static void outRow16(std::ofstream& f, unsigned type, std::size_t len,
                     std::size_t addr, const byte* row) {
    HEXBED_ASSERT(type == 0 || type == 1 || type == 5 || type == 9);
    outRow<2>(f, type, len, addr, row);
}

static void outRow24(std::ofstream& f, unsigned type, std::size_t len,
                     std::size_t addr, const byte* row) {
    HEXBED_ASSERT(type == 2 || type == 6 || type == 8);
    outRow<3>(f, type, len, addr, row);
}

static void outRow32(std::ofstream& f, unsigned type, std::size_t len,
                     std::size_t addr, const byte* row) {
    HEXBED_ASSERT(type == 3 || type == 7);
    outRow<4>(f, type, len, addr, row);
}

static void outRowRecord(std::ofstream& f, std::size_t records) {
    if (records < 0x10000UL)
        outRow16(f, 5, 0, records, nullptr);
    else if (records < 0x1000000UL)
        outRow24(f, 6, 0, records, nullptr);
}

void ExportPluginMotorolaSREC::doExport(
    HexBedTask& task, const std::filesystem::path& filename,
    std::function<bufsize(bufsize, bytespan)> read, bufsize actualOffset,
    bufsize size, const ExportDetails& details) {
    using enum ExportPluginMotorolaSRECMode;
    using enum ExportPluginMotorolaSRECOffsetBase;
    std::ofstream f(filename);
    byte row[256];
    unsigned bc = settings_.columns;
    if (bc > 240 || !bc) bc = 16;
    std::size_t s = static_cast<std::size_t>(size);
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

    outRow16(f, 0,
             strlen(strcpy(reinterpret_cast<char*>(row), "HexBed export")), 0,
             row);
    std::size_t records = 0;
    switch (settings_.mode) {
    case S19:
        for (std::size_t offset = 0; offset < s;) {
            std::size_t remaining = std::min<std::size_t>(bc, s - offset);
            std::size_t n = read(offset, bytespan{row, remaining});
            outRow16(f, 1, n, offset + base, row);
            ++records;
            offset += n;
            if (!n) throw std::runtime_error("unknown read error");
        }
        outRowRecord(f, records);
        outRow16(f, 9, 0, 0, nullptr);
        break;
    case S28:
        for (std::size_t offset = 0; offset < s;) {
            std::size_t remaining = std::min<std::size_t>(bc, s - offset);
            std::size_t n = read(offset, bytespan{row, remaining});
            outRow24(f, 2, n, offset + base, row);
            ++records;
            offset += n;
            if (!n) throw std::runtime_error("unknown read error");
        }
        outRowRecord(f, records);
        outRow24(f, 8, 0, 0, nullptr);
        break;
    case S37:
        for (std::size_t offset = 0; offset < s;) {
            std::size_t remaining = std::min<std::size_t>(bc, s - offset);
            std::size_t n = read(offset, bytespan{row, remaining});
            outRow32(f, 3, n, offset + base, row);
            ++records;
            offset += n;
            if (!n) throw std::runtime_error("unknown read error");
        }
        outRowRecord(f, records);
        outRow32(f, 7, 0, 0, nullptr);
        break;
    }
}

};  // namespace plugins

};  // namespace hexbed
