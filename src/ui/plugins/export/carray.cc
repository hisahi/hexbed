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
// ui/plugins/export/carray.cc -- impl for builtin C source code exporter

#include "ui/plugins/export/carray.hh"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/msgdlg.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/translation.h>
#include <wx/valgen.h>
#include <wx/valtext.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>

#include "app/config.hh"
#include "common/hexconv.hh"
#include "common/intconv.hh"
#include "common/logger.hh"
#include "ui/plugins/dialog.hh"
#include "ui/settings/validate.hh"

namespace hexbed {

namespace plugins {

static constexpr unsigned C_MAX_COLUMNS = 1000;
static constexpr unsigned C_MAX_BYTES_PER_LINE = 256;

class ExportPluginCDialog : public PluginConfigureDialog<true> {
  private:
    ExportPluginCSettings* settings_;
    bufsize off_;
    bufsize sz_;
    wxString variableNameWx_;

  public:
    ExportPluginCDialog(wxWindow* parent, ExportPluginCSettings& settings,
                        bufsize actualOffset, bufsize size)
        : PluginConfigureDialog<true>(parent, wxID_ANY, _("C source code"),
                                      wxDefaultPosition, wxDefaultSize,
                                      wxDEFAULT_DIALOG_STYLE),
          settings_(&settings),
          variableNameWx_(settings.variableName) {
        SetReturnCode(wxID_CANCEL);

        wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);

        wxStdDialogButtonSizer* buttons = new wxStdDialogButtonSizer();
        wxButton* okButton = new wxButton(this, wxID_OK);
        wxButton* cancelButton = new wxButton(this, wxID_CANCEL);
        okButton->Bind(wxEVT_BUTTON, &ExportPluginCDialog::OnOK, this);

        buttons->SetAffirmativeButton(okButton);
        buttons->SetNegativeButton(cancelButton);
        buttons->Realize();

        top->Add(new wxStaticText(
                     this, wxID_ANY, _("signed assumes two's complement."),
                     wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END),
                 wxSizerFlags().Expand());
        wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
        row->Add(new wxCheckBox(this, wxID_ANY, _("Signed"), wxDefaultPosition,
                                wxDefaultSize, 0,
                                wxGenericValidator(&settings.signedChar)),
                 wxSizerFlags().Center());
        row->AddStretchSpacer();
        row->Add(new wxTextCtrl(
            this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0,
            wxTextValidator(wxFILTER_EMPTY, &variableNameWx_)));
        top->Add(row, wxSizerFlags().Expand());

        wxSpinCtrl* spin;
        top->Add(new wxStaticText(this, wxID_ANY, _("Determine width by"),
                                  wxDefaultPosition, wxDefaultSize,
                                  wxST_ELLIPSIZE_END),
                 wxSizerFlags().Expand());
        row = new wxBoxSizer(wxHORIZONTAL);
        row->Add(new wxRadioButton(this, wxID_ANY, _("Bytes per line"),
                                   wxDefaultPosition, wxDefaultSize, wxRB_GROUP,
                                   hexbed::ui::RadioValidator<bool>(
                                       &settings.useBytesPerLine, true)),
                 wxSizerFlags().Center());
        row->AddStretchSpacer();
        spin = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                              wxDefaultSize, wxSP_ARROW_KEYS, 1,
                              C_MAX_BYTES_PER_LINE, 16);
        row->Add(spin, wxSizerFlags().Proportion(1));
        spin->SetValidator(hexbed::ui::GenericCustomValidator<unsigned, int>(
            &settings.bytesPerLine));
        top->Add(row, wxSizerFlags().Expand());

        row = new wxBoxSizer(wxHORIZONTAL);
        row->Add(new wxRadioButton(this, wxID_ANY, _("Columns per line"),
                                   wxDefaultPosition, wxDefaultSize, 0,
                                   hexbed::ui::RadioValidator<bool>(
                                       &settings.useBytesPerLine, false)),
                 wxSizerFlags().Center());
        row->AddStretchSpacer();
        spin = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                              wxDefaultSize, wxSP_ARROW_KEYS, 1, C_MAX_COLUMNS,
                              80);
        row->Add(spin, wxSizerFlags().Proportion(1));
        spin->SetValidator(hexbed::ui::GenericCustomValidator<unsigned, int>(
            &settings.columns));
        top->Add(row, wxSizerFlags().Expand());

        top->Add(new wxStaticText(this, wxID_ANY, _("Indentation"),
                                  wxDefaultPosition, wxDefaultSize,
                                  wxST_ELLIPSIZE_END),
                 wxSizerFlags().Expand());

        top->Add(new wxCheckBox(this, wxID_ANY, _("Use tab instead of spaces"),
                                wxDefaultPosition, wxDefaultSize, 0,
                                wxGenericValidator(&settings.indentTabs)),
                 wxSizerFlags().Expand());

        wxSpinCtrl* spin2 =
            new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                           wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0);
        row = new wxBoxSizer(wxHORIZONTAL);
        row->Add(new wxStaticText(this, wxID_ANY, _("Spaces to indent/per tab"),
                                  wxDefaultPosition, wxDefaultSize,
                                  wxST_ELLIPSIZE_END),
                 wxSizerFlags().Center());
        row->AddStretchSpacer();
        row->Add(spin2);
        spin2->SetValidator(hexbed::ui::GenericCustomValidator<unsigned, int>(
            &settings.indentSpace));
        top->Add(row, wxSizerFlags().Expand());

        top->Add(
            new wxStaticText(this, wxID_ANY, _("Format"), wxDefaultPosition,
                             wxDefaultSize, wxST_ELLIPSIZE_END),
            wxSizerFlags().Expand());

        top->Add(new wxCheckBox(this, wxID_ANY, _("Encode bytes as hex"),
                                wxDefaultPosition, wxDefaultSize, 0,
                                wxGenericValidator(&settings.useHex)),
                 wxSizerFlags().Expand());
        top->Add(new wxCheckBox(this, wxID_ANY,
                                _("Use C99 stdint type instead of char"),
                                wxDefaultPosition, wxDefaultSize, 0,
                                wxGenericValidator(&settings.stdint)),
                 wxSizerFlags().Expand());
        top->Add(new wxCheckBox(this, wxID_ANY, _("static (internal linkage)"),
                                wxDefaultPosition, wxDefaultSize, 0,
                                wxGenericValidator(&settings.isStatic)),
                 wxSizerFlags().Expand());
        top->Add(buttons, wxSizerFlags().Expand());

        SetSizer(top);
        Fit();
        Layout();
    }

    bool CheckValid() noexcept override {
        unsigned byteWidth = settings_->useHex
                                 ? (settings_->signedChar ? 7 : 6)
                                 : (settings_->signedChar ? 5 : 4);
        if ((!settings_->useBytesPerLine &&
             settings_->indentSpace + byteWidth >= settings_->columns) ||
            (settings_->useBytesPerLine && settings_->bytesPerLine < 1)) {
            wxMessageBox(
                _("There are not enough columns for the current settings to "
                  "fit a single byte on a line.\nIncrease the number of "
                  "columns or switch to using bytes per line instead."),
                "HexBed", wxOK | wxICON_ERROR);
            return false;
        }
        if (variableNameWx_.empty()) {
            wxMessageBox(_("The variable name may not be empty."), "HexBed",
                         wxOK | wxICON_ERROR);
            return false;
        }
        const char* c_str = variableNameWx_.c_str();
        if (!*c_str) {
            wxMessageBox(_("The current variable name contains characters that "
                           "the system is unable to presently convert into a "
                           "multibyte string.\nPlease change the name so that "
                           "it only uses ASCII characters."),
                         "HexBed", wxOK | wxICON_ERROR);
            return false;
        }
        settings_->variableName = std::string(c_str);
        return true;
    }
};

ExportPluginC::ExportPluginC(pluginid id)
    : LocalizableExportPlugin(id, TAG("C source code")) {}

wxString ExportPluginC::getFileFilter() const {
    return _("C source code file (*.c)") + "|*.c|" +
           ExportPlugin::getFileFilter();
}

bool ExportPluginC::configureExport(wxWindow* parent,
                                    const std::filesystem::path& filename,
                                    bufsize actualOffset, bufsize size,
                                    const ExportDetails& details) {
    ExportPluginCDialog dial(parent, settings_, actualOffset, size);
    dial.TransferDataToWindow();
    int result = dial.ShowModal();
    if (result != wxID_OK) return false;
    dial.TransferDataFromWindow();
    return true;
}

static std::int8_t resign(std::uint8_t x) {
    return (x & 0x80) ? static_cast<std::int8_t>(static_cast<int>(x) - 0x100)
                      : static_cast<std::int8_t>(x);
}

static unsigned unsign(std::int8_t x) {
    return static_cast<unsigned>(x < 0 ? -static_cast<int>(x) : x);
}

template <bool ByteHex, bool ByteSigned>
static void byteToTextRigid(std::ofstream& f, byte b);

template <>
void byteToTextRigid<false, false>(std::ofstream& f, byte b) {
    f << std::dec << std::setw(3) << std::setfill(' ')
      << static_cast<unsigned>(b);
}

template <>
void byteToTextRigid<false, true>(std::ofstream& f, byte b) {
    f << std::dec << std::setw(4) << std::setfill(' ')
      << static_cast<int>(resign(b));
}

template <>
void byteToTextRigid<true, false>(std::ofstream& f, byte b) {
    f << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
      << static_cast<unsigned>(b);
}

template <>
void byteToTextRigid<true, true>(std::ofstream& f, byte b) {
    std::int8_t x = resign(b);
    f << (x < 0 ? '-' : ' ');
    f << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
      << static_cast<unsigned>(unsign(x));
}

template <bool ByteHex, bool ByteSigned>
static void byteToTextFlexible(std::ofstream& f, byte b);

template <bool ByteHex, bool ByteSigned>
static unsigned byteToTextFlexibleWidth(byte b);

template <>
void byteToTextFlexible<false, false>(std::ofstream& f, byte b) {
    f << std::dec << static_cast<unsigned>(b);
}

template <>
unsigned byteToTextFlexibleWidth<false, false>(byte b) {
    return b >= 100 ? 3 : b >= 10 ? 2 : 1;
}

template <>
void byteToTextFlexible<false, true>(std::ofstream& f, byte b) {
    f << std::dec << static_cast<int>(resign(b));
}

template <>
unsigned byteToTextFlexibleWidth<false, true>(byte b) {
    std::int8_t x = resign(b);
    if (x >= 0)
        return x >= 100 ? 3 : x >= 10 ? 2 : 1;
    else
        return x <= -100 ? 4 : x <= -10 ? 3 : 2;
}

template <>
void byteToTextFlexible<true, false>(std::ofstream& f, byte b) {
    f << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
      << static_cast<unsigned>(b);
}

template <>
unsigned byteToTextFlexibleWidth<true, false>(byte b) {
    return 4;
}

template <>
void byteToTextFlexible<true, true>(std::ofstream& f, byte b) {
    std::int8_t x = resign(b);
    f << (x < 0 ? '-' : ' ');
    f << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
      << static_cast<unsigned>(unsign(x));
}

template <>
unsigned byteToTextFlexibleWidth<true, true>(byte b) {
    return 5;
}

static unsigned indent(std::ofstream& f,
                       const ExportPluginCSettings& settings) {
    if (settings.indentTabs) {
        f << '\t';
    } else if (settings.indentSpace) {
        f << std::setfill(' ') << std::setw(settings.indentSpace) << ' ';
    }
    return settings.indentSpace;
}

static unsigned comma(std::ofstream& f, bool endOfLine, bool endOfText) {
    if (!endOfText) {
        if (endOfLine) {
            f << ",";
            return 1;
        } else {
            f << ", ";
            return 2;
        }
    }
    return 0;
}

template <bool ByteHex, bool ByteSigned>
static void doExportRigid_(HexBedTask& task, std::ofstream& f,
                           std::function<bufsize(bufsize, bytespan)> read,
                           bufsize size,
                           const ExportPluginCSettings& settings) {
    bufsize o = 0;
    byte row[C_MAX_BYTES_PER_LINE];
    while (!task.isCancelled() && o < size) {
        bufsize n = read(o, bytespan{row, settings.bytesPerLine});
        indent(f, settings);
        for (unsigned i = 0; i < n; ++i) {
            byteToTextRigid<ByteHex, ByteSigned>(f, row[i]);
            comma(f, i + 1 == n, o + i + 1 == size);
        }
        o += n;
        f << '\n';
    }
}

template <bool ByteHex, bool ByteSigned>
static void doExportFlexible_(HexBedTask& task, std::ofstream& f,
                              std::function<bufsize(bufsize, bytespan)> read,
                              bufsize size,
                              const ExportPluginCSettings& settings) {
    bufsize o = 0;
    byte row[C_MAX_COLUMNS / 3];
    while (!task.isCancelled() && o < size) {
        bufsize n = read(o, bytespan{row, sizeof(row)});
        unsigned i;
        unsigned w = settings.columns - indent(f, settings);
        for (i = 0; i < n; ++i) {
            bool wide =
                i + 1 < n &&
                w >= byteToTextFlexibleWidth<ByteHex, ByteSigned>(row[i]) + 3 +
                         byteToTextFlexibleWidth<ByteHex, ByteSigned>(
                             row[i + 1]);
            byteToTextFlexible<ByteHex, ByteSigned>(f, row[i]);
            w -= byteToTextFlexibleWidth<ByteHex, ByteSigned>(row[i]);
            w -= comma(f, !wide, o + i + 1 == size);
            if (!wide) break;
        }
        o += i + 1;
        f << '\n';
    }
}

static void doExportFlexible(HexBedTask& task, std::ofstream& f,
                             std::function<bufsize(bufsize, bytespan)> read,
                             bufsize size,
                             const ExportPluginCSettings& settings) {
    if (settings.useHex) {
        if (settings.signedChar)
            doExportFlexible_<true, true>(task, f, read, size, settings);
        else
            doExportFlexible_<true, false>(task, f, read, size, settings);
    } else {
        if (settings.signedChar)
            doExportFlexible_<false, true>(task, f, read, size, settings);
        else
            doExportFlexible_<false, false>(task, f, read, size, settings);
    }
}

static void doExportRigid(HexBedTask& task, std::ofstream& f,
                          std::function<bufsize(bufsize, bytespan)> read,
                          bufsize size, const ExportPluginCSettings& settings) {
    if (settings.useHex) {
        if (settings.signedChar)
            doExportRigid_<true, true>(task, f, read, size, settings);
        else
            doExportRigid_<true, false>(task, f, read, size, settings);
    } else {
        if (settings.signedChar)
            doExportRigid_<false, true>(task, f, read, size, settings);
        else
            doExportRigid_<false, false>(task, f, read, size, settings);
    }
}

void ExportPluginC::doExport(HexBedTask& task,
                             const std::filesystem::path& filename,
                             std::function<bufsize(bufsize, bytespan)> read,
                             bufsize actualOffset, bufsize size,
                             const ExportDetails& details) {
    std::ofstream f(filename);
    f << "\n";
    if (settings_.isStatic) f << "static ";
    if (settings_.signedChar)
        f << (settings_.stdint ? "int8_t " : "signed char ");
    else
        f << (settings_.stdint ? "uint8_t " : "unsigned char ");
    f << settings_.variableName;
    f << '[';
    f << size;
    f << "] = {\n";
    if (settings_.useBytesPerLine)
        doExportRigid(task, f, read, size, settings_);
    else
        doExportFlexible(task, f, read, size, settings_);
    f << "};\n";
}

};  // namespace plugins

};  // namespace hexbed
