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
// ui/plugins/import/srec.cc -- impl for builtin Motorola S-record importer

#include "ui/plugins/import/srec.hh"

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

class ImportPluginMotorolaSRECDialog : public wxDialog {
  public:
    ImportPluginMotorolaSRECDialog(wxWindow* parent,
                                   ImportPluginMotorolaSRECSettings& settings)
        : wxDialog(parent, wxID_ANY, _("Motorola S-record"), wxDefaultPosition,
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

ImportPluginMotorolaSREC::ImportPluginMotorolaSREC(pluginid id)
    : LocalizableImportPlugin(
          /// format name, perhaps should not be translated
          id, TAG("Motorola S-record")) {}

wxString ImportPluginMotorolaSREC::getFileFilter() const {
    return _("Motorola S-record file (*.s19, *.s28, *.s37)") +
           "|*.s19;*.s28;*.s37|" + ImportPlugin::getFileFilter();
}

bool ImportPluginMotorolaSREC::configureImport(
    wxWindow* parent, const std::filesystem::path& filename) {
    ImportPluginMotorolaSRECDialog dial(parent, settings_);
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
    row[n++] = readByte(lineno, &text);
    for (int i = 0; i < row[0]; ++i) row[n++] = readByte(lineno, &text);
    return n;
}

void ImportPluginMotorolaSREC::doImport(
    HexBedTask& task, const std::filesystem::path& filename,
    std::function<void(bufsize, const_bytespan)> output) {
    std::ifstream f(filename);
    byte row[256];
    std::size_t lineno = 1;
    for (std::string line; std::getline(f, line); ++lineno) {
        if (line[0] != 'S') continue;
        int type = hexDigitToNum(line[1]);
        if (type >= 10) type = -1;
        if (type < 0)
            throw ImportFileInvalidError(TAG("Invalid record type."), lineno);
        std::size_t n = readRow(lineno, row, line.c_str() + 2);
        if (settings_.checksums) {
            byte b = 0;
            for (std::size_t i = 0; i < n; ++i) b += row[i];
            if (~b & 255)
                throw ImportFileInvalidError(TAG("Checksum error."), lineno);
        }
        std::size_t addr = 0;
        unsigned offset, count = row[0];
        switch (type) {
        case 0:  // header, skip
            break;
        case 1:  // data 16-bit addr
            addr |= row[1] << 8;
            addr |= row[2];
            offset = 3;
            count -= offset;
            output(addr, const_bytespan{row + offset, count});
            break;
        case 2:  // data 24-bit addr
            addr |= row[1] << 16;
            addr |= row[2] << 8;
            addr |= row[3];
            offset = 4;
            count -= offset;
            output(addr, const_bytespan{row + offset, count});
            break;
        case 3:  // data 32-bit addr
            addr |= row[1] << 24;
            addr |= row[2] << 16;
            addr |= row[3] << 8;
            addr |= row[4];
            offset = 5;
            count -= offset;
            output(addr, const_bytespan{row + offset, count});
            break;
        case 4:  // invalid, skip
            break;
        case 5:  // count 16-bit, skip
            break;
        case 6:  // count 24-bit, skip
            break;
        case 7:  // exec addr 32-bit, skip
            break;
        case 8:  // exec addr 24-bit, skip
            break;
        case 9:  // exec addr 16-bit, skip
            break;
        default:
            throw ImportFileInvalidError(TAG("Unrecognized record type."),
                                         lineno);
        }
    }
}

};  // namespace plugins

};  // namespace hexbed
