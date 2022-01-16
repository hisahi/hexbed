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
// ui/plugins/export/html.cc -- impl for builtin HTML exporter

#include "ui/plugins/export/html.hh"

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

#include <bit>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>

#include "app/config.hh"
#include "app/plugin.hh"
#include "common/charconv.hh"
#include "common/hexconv.hh"
#include "common/intconv.hh"
#include "common/logger.hh"
#include "common/version.hh"
#include "ui/config.hh"
#include "ui/plugins/dialog.hh"
#include "ui/settings/validate.hh"
#include "ui/string.hh"

namespace hexbed {

namespace plugins {

class ExportPluginHTMLDialog : public PluginConfigureDialog<true> {
  private:
    ExportPluginHTMLSettings* settings_;

  public:
    ExportPluginHTMLDialog(wxWindow* parent, ExportPluginHTMLSettings& settings,
                           bufsize columns)
        : PluginConfigureDialog<true>(parent, wxID_ANY, _("HTML"),
                                      wxDefaultPosition, wxDefaultSize,
                                      wxDEFAULT_DIALOG_STYLE),
          settings_(&settings) {
        SetReturnCode(wxID_CANCEL);

        wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);

        wxStdDialogButtonSizer* buttons = new wxStdDialogButtonSizer();
        wxButton* okButton = new wxButton(this, wxID_OK);
        wxButton* cancelButton = new wxButton(this, wxID_CANCEL);
        okButton->Bind(wxEVT_BUTTON, &ExportPluginHTMLDialog::OnOK, this);

        buttons->SetAffirmativeButton(okButton);
        buttons->SetNegativeButton(cancelButton);
        buttons->Realize();

        wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
        wxSpinCtrl* spin =
            new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                           wxDefaultSize, wxSP_ARROW_KEYS, 1, MAX_COLUMNS, 16);
        top->Add(new wxStaticText(this, wxID_ANY, _("Column count"),
                                  wxDefaultPosition, wxDefaultSize,
                                  wxST_ELLIPSIZE_END),
                 wxSizerFlags().Expand());
        top->Add(new wxRadioButton(
                     this, wxID_ANY,
                     wxString::Format(_("From editor (%s)"),
                                      wxString::Format("%lu", columns)),
                     wxDefaultPosition, wxDefaultSize, wxRB_GROUP,
                     hexbed::ui::RadioValidator<bool>(&settings.customColumns,
                                                      false)),
                 wxSizerFlags().Expand());
        row->Add(new wxRadioButton(this, wxID_ANY, _("Custom"),
                                   wxDefaultPosition, wxDefaultSize, 0,
                                   hexbed::ui::RadioValidator<bool>(
                                       &settings.customColumns, true)),
                 wxSizerFlags().Expand());
        row->AddStretchSpacer();
        row->Add(spin, wxSizerFlags().Proportion(1));
        spin->SetValidator(hexbed::ui::GenericCustomValidator<unsigned, int>(
            &settings.columns));
        top->Add(row);
        top->Add(buttons, wxSizerFlags().Expand());

        SetSizer(top);
        Fit();
        Layout();
    }

    bool CheckValid() noexcept override { return true; }
};

ExportPluginHTML::ExportPluginHTML(pluginid id)
    : LocalizableExportPlugin(id, TAG("HTML")) {}

wxString ExportPluginHTML::getFileFilter() const {
    return _("HTML file (*.html, *.htm)") + "|*.html;*.htm|" +
           ExportPlugin::getFileFilter();
}

bool ExportPluginHTML::configureExport(wxWindow* parent,
                                       const std::filesystem::path& filename,
                                       bufsize actualOffset, bufsize size,
                                       const ExportDetails& details) {
    ExportPluginHTMLDialog dial(parent, settings_, details.columns);
    dial.TransferDataToWindow();
    int result = dial.ShowModal();
    if (result != wxID_OK) return false;
    dial.TransferDataFromWindow();
    return true;
}

static void encodeOneUTF8Char(char* buf, char32_t c) {
    buf[mbcs_utf8
            .encode(
                charEncodeFromArray(1, &c),
                charEncodeToArray(MBCS_CHAR_MAX, reinterpret_cast<byte*>(buf)))
            .wroteBytes] = 0;
}

template <typename T>
static void htmlString(std::ofstream& f, const std::basic_string<T>& string) {
    // char utfBuffer[MBCS_CHAR_MAX + 1];
    for (T c : string) {
        if (c == CHAR('&')) {
            f << "&amp;";
            continue;
        } else if (c == CHAR('<')) {
            f << "&lt;";
            continue;
        } else if (c == CHAR('>')) {
            f << "&gt;";
            continue;
        }
        /*encodeOneUTF8Char(utfBuffer, c);
        f << utfBuffer;*/
        f << c;
    }
}

template <typename T>
static void htmlStringQuote(std::ofstream& f,
                            const std::basic_string<T>& string) {
    char utfBuffer[MBCS_CHAR_MAX + 1];
    for (T c : string) {
        if (c == CHAR('&')) {
            f << "&amp;";
            continue;
        } else if (c == CHAR('<')) {
            f << "&lt;";
            continue;
        } else if (c == CHAR('>')) {
            f << "&gt;";
            continue;
        }
        if (c == CHAR('"') || c == CHAR('\\')) f << '\\';
        encodeOneUTF8Char(utfBuffer, c);
        f << utfBuffer;
    }
}

static void htmlColor(std::ofstream& f, long color) {
    f << "#" << std::hex;
    f << std::setw(2) << std::setfill('0') << (color & 0xFF);
    f << std::setw(2) << std::setfill('0') << ((color >> 8) & 0xFF);
    f << std::setw(2) << std::setfill('0') << ((color >> 16) & 0xFF);
}

static void htmlFont(std::ofstream& f, const wxFont& font) {
    switch (font.GetStyle()) {
    case wxFONTSTYLE_NORMAL:
    default:
        f << "normal";
        break;
    case wxFONTSTYLE_ITALIC:
        f << "italic";
        break;
    case wxFONTSTYLE_SLANT:
        f << "oblique";
        break;
    }
    f << ' ';
    switch (font.GetWeight()) {
    case wxFONTWEIGHT_NORMAL:
    default:
        f << "normal";
        break;
    case wxFONTWEIGHT_LIGHT:
        f << "lighter";
        break;
    case wxFONTWEIGHT_BOLD:
        f << "bold";
        break;
    }
    f << ' ';
    f << std::dec << static_cast<unsigned>(font.GetPointSize()) << "pt ";
    f << '"';
    htmlStringQuote(f, u32StringFromWx(font.GetFaceName()));
    f << '"';
}

void ExportPluginHTML::doExport(HexBedTask& task,
                                const std::filesystem::path& filename,
                                std::function<bufsize(bufsize, bytespan)> read,
                                bufsize actualOffset, bufsize size,
                                const ExportDetails& details) {
    std::ofstream f(filename);

    unsigned bc = settings_.customColumns ? settings_.columns : details.columns;
    unsigned gs = config().groupSize;
    unsigned ugs = hexbed::ui::configUtfGroupSize();
    bc -= bc % gs;
    bufsize maxOffset = actualOffset + size - 1, offsetWidth = 0;
    bufsize finalByte = maxOffset + 1;
    unsigned offsetRadix = config().offsetRadix;
    unsigned mask = gs - 1;
    bool hasHex, hasText;
    bufsize tmp = maxOffset;
    while (tmp) {
        ++offsetWidth;
        tmp /= offsetRadix;
    }
    offsetWidth = std::max<bufsize>(offsetWidth, 8);
    byte buf[MAX_COLUMNS];
    char utfBuffer[MBCS_CHAR_MAX + 1];

    hasText = config().showColumnTypes & 1;
    hasHex = config().showColumnTypes & 2;

    bc -= bc % ugs;
    if (!bc) bc = ugs;

    f << "<!DOCTYPE html>\n";
    f << "<html>\n";
    f << "<head>\n";
    f << "<meta charset=\"utf-8\">\n";
    f << "<meta name=\"generator\" content=\"HexBed v" HEXBED_VER_STRING
         "\">\n";
    f << "<title>";
    htmlString<strchar>(f, details.filename);
    f << "</title>\n";
    f << "<style type=\"text/css\">\n";
    f << "html { background-color: ";
    htmlColor(f, config().backgroundColor);
    f << "; color: ";
    htmlColor(f, config().hexColorEven);
    f << "; }\npre { font: ";
    htmlFont(f, hexbed::ui::configFont());
    f << ", monospace; }\n.datatext { color: ";
    htmlColor(f, config().textColor);
    f << "; }\n.datahex { color: ";
    htmlColor(f, config().hexColorOdd);
    f << "; }\n.offset { color: ";
    htmlColor(f, config().offsetColor);
    f << "; }\n";
    f << "</style>\n";
    f << "</head>\n";
    f << "<body>\n";

    if (config().uppercase)
        f << std::uppercase;
    else
        f << std::nouppercase;

    if (hasHex) {
        f << "<pre>\n";
        f << std::setfill(' ') << std::setw(offsetWidth) << ' ';
        f << "   ";
        // top row
        switch (offsetRadix) {
        case 8:
            f << std::oct;
            break;
        case 10:
            f << std::dec;
            break;
        case 16:
            f << std::hex;
            break;
        }
        f << "<span class=\"offset\">";
        for (unsigned i = 0; i < bc; i += gs) {
            f << std::setfill('0') << std::setw(2) << i;
            f << std::setfill(' ') << std::setw(2 * gs - 1) << ' ';
        }
        f << "</span>\n";
        f << "</pre>\n";
    }

    f << "<pre>\n";
    unsigned utf = config().utfMode;
    for (bufsize row = actualOffset - actualOffset % bc;
         !task.isCancelled() && row <= maxOffset; row += bc) {
        unsigned startOffset = row > actualOffset ? 0 : actualOffset - row;
        unsigned endOffset = row + bc <= finalByte ? bc : finalByte - row;
        bufsize n = read((row + startOffset) - actualOffset,
                         bytespan{buf + startOffset, endOffset - startOffset});
        endOffset = startOffset + n;

        switch (offsetRadix) {
        case 8:
            f << std::oct;
            break;
        case 10:
            f << std::dec;
            break;
        case 16:
            f << std::hex;
            break;
        }
        f << "<span class=\"offset\">";
        f << std::setfill('0') << std::setw(offsetWidth) << row;
        f << "</span>   ";
        if (hasHex) {
            f << std::hex;
            f << "<span class=\"datahex\">";
            for (unsigned i = 0; i < bc; ++i) {
                if (i && !(i & mask)) f << ' ';
                if (i < startOffset || i >= endOffset)
                    f << "  ";
                else
                    f << std::setfill('0') << std::setw(2)
                      << static_cast<unsigned>(buf[i]);
            }
            f << "</span>";
            if (hasText) f << "   ";
        }
        if (hasText) {
            f << "<span class=\"datatext\">";
            for (unsigned i = 0; i < bc; i += ugs) {
                if (i < startOffset || i >= endOffset)
                    f << ' ';
                else {
                    char32_t c = convertCharFrom(utf, bc - i, &buf[i]);
                    if (!c)
                        f << '.';
                    else if (c == '&')
                        f << "&amp;";
                    else if (c == '<')
                        f << "&lt;";
                    else if (c == '>')
                        f << "&gt;";
                    else {
                        encodeOneUTF8Char(utfBuffer, c);
                        f << utfBuffer;
                    }
                }
            }
            f << "</span>";
        }
        f << "\n";
    }
    f << "</pre>\n";

    f << "<p>Generated with HexBed v" HEXBED_VER_STRING "</p>\n";
    f << "</body>\n";
    f << "</html>\n";
}

};  // namespace plugins

};  // namespace hexbed
