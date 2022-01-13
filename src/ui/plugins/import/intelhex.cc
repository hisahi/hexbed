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
// ui/plugins/import/intelhex.cc -- impl for builtin Intel HEX importer

#include "ui/plugins/import/intelhex.hh"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/translation.h>
#include <wx/valgen.h>

#include <fstream>
#include <iostream>
#include <string>

#include "common/hexconv.hh"
#include "common/intconv.hh"

namespace hexbed {

namespace plugins {

class ImportPluginIntelHEXDialog : public wxDialog {
  public:
    ImportPluginIntelHEXDialog(wxWindow* parent,
                               ImportPluginIntelHEXSettings& settings)
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

        top->Add(new wxCheckBox(this, wxID_ANY, _("Verify checksums"),
                                wxDefaultPosition, wxDefaultSize, 0,
                                wxGenericValidator(&settings.checksums)),
                 wxSizerFlags().Proportion(1));
        top->Add(buttons);

        SetSizer(top);
        Fit();
        Layout();
    }
};

ImportPluginIntelHEX::ImportPluginIntelHEX(pluginid id)
    : LocalizableImportPlugin(
          /// format name, perhaps should not be translated
          id, TAG("Intel HEX")) {}

wxString ImportPluginIntelHEX::getFileFilter() const {
    return _("Intel HEX file (*.hex, *.h86)") + "|*.hex;*.h86|" +
           ImportPlugin::getFileFilter();
}

bool ImportPluginIntelHEX::configureImport(wxWindow* parent,
                                           const std::string& filename) {
    ImportPluginIntelHEXDialog dial(parent, settings_);
    dial.TransferDataToWindow();
    int result = dial.ShowModal();
    if (result != wxID_OK) return false;
    dial.TransferDataFromWindow();
    return true;
}

static unsigned readNibble(std::size_t lineno, const char** text) {
    char c = *(*text)++;
    if (!c)
        throw ImportFileInvalidError(
            TAG("Unexpected end-of-file while reading line."), lineno);
    int d = hexDigitToNum(c);
    if (d < 0)
        throw ImportFileInvalidError(
            TAG("Invalid character while reading line; expected hex digit."),
            lineno);
    return static_cast<unsigned>(d);
}

static byte readByte(std::size_t lineno, const char** text) {
    int h = readNibble(lineno, text);
    int l = readNibble(lineno, text);
    return static_cast<byte>((h << 4) | l);
}

static std::size_t readRow(std::size_t lineno, byte* row, const char* text) {
    std::size_t n = 0;
    for (int i = 0; i < 4; ++i) row[n++] = readByte(lineno, &text);
    for (int i = 0; i < row[0]; ++i) row[n++] = readByte(lineno, &text);
    row[n++] = readByte(lineno, &text);
    return n;
}

void ImportPluginIntelHEX::doImport(
    HexBedTask& task, const std::string& filename,
    std::function<void(bufsize, const_bytespan)> output) {
    std::ifstream f(filename);
    byte row[261];
    std::size_t lineno = 1;
    bufsize mask = 0;
    for (std::string line; std::getline(f, line); ++lineno) {
        std::size_t index = line.find(':');
        if (index == std::string::npos) continue;
        std::size_t n = readRow(lineno, row, line.c_str() + index + 1);
        if (settings_.checksums) {
            byte b = 0;
            for (std::size_t i = 0; i < n; ++i) b += row[i];
            if (b) throw ImportFileInvalidError(TAG("Checksum error."), lineno);
        }
        auto addr = uintFromBytes<std::uint16_t>(2, &row[1], false);
        switch (row[3]) {
        case 0:  // data
            output(mask + addr, const_bytespan{&row[4], row[0]});
            break;
        case 1:  // end of file
            return;
        case 2:  // data segment
            mask = addr << 4;
            break;
        case 3:  // exec segment
            break;
        case 4:  // data hiword
            mask = addr << 16;
            break;
        case 5:  // exec hiword
            break;
        default:
            throw ImportFileInvalidError(TAG("Unrecognized record type."),
                                         lineno);
        }
    }
}

};  // namespace plugins

};  // namespace hexbed
