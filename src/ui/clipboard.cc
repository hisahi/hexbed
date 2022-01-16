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
// ui/clipboard.cc -- impl for the clipboard functionality

#include "ui/clipboard.hh"

#include <wx/clipbrd.h>
#include <wx/intl.h>
#include <wx/msgdlg.h>

#include "app/config.hh"
#include "common/buffer.hh"
#include "common/charconv.hh"
#include "common/hexconv.hh"
#include "ui/config.hh"
#include "ui/string.hh"

namespace hexbed {
namespace clip {

bool HasClipboard() {
    bool flag = false;
    if (wxTheClipboard->Open()) {
        flag = wxTheClipboard->IsSupported(wxDF_TEXT);
        wxTheClipboard->Close();
    }
    return flag;
}

void CopyBytes(HexBedDocument& document, bufsize off, bufsize cnt, bool text) {
    if (!wxTheClipboard->Open()) throw ClipboardError();
    bufsize q = 0, qq;
    byte buf[BUFFER_SIZE];
    if (text) {
        std::u32string result;
        std::size_t sz = sizeof(buf);
        unsigned utf = config().utfMode;
        sz -= sz % hexbed::ui::configUtfGroupSize();
        HEXBED_ASSERT(sz);

        while (cnt) {
            qq = std::min<bufsize>(cnt, sz);
            q = document.read(off, bytespan{buf, qq});
            result += convertCharsFrom(utf, q, buf, false);
            if (q < qq) break;
            off += qq, cnt -= qq;
        }

        wxTheClipboard->SetData(
            new wxTextDataObject(u32stringToWstring(result)));
    } else {
        string result;
        bool cont = false;

        while (cnt) {
            qq = std::min<bufsize>(cnt, sizeof(buf));
            q = document.read(off, bytespan{buf, qq});
            result += hexFromBytes(q, buf, config().uppercase, cont);
            if (q < qq) break;
            off += qq, cnt -= qq;
            cont = true;
        }

        wxTheClipboard->SetData(new wxTextDataObject(result));
    }
    wxTheClipboard->Close();
}

bool PasteBytes(HexBedDocument& document, bool insert, bufsize off, bufsize cnt,
                bool text, bufsize& len) {
    bool flag = true;
    if (!wxTheClipboard->Open()) throw ClipboardError();
    if (!wxTheClipboard->IsSupported(wxDF_TEXT)) {
        wxMessageBox(_("Cannot paste the current clipboard contents, "
                       "because it does not contain text data."),
                     "HexBed", wxOK | wxICON_ERROR);
        goto fail;
    } else {
        wxTextDataObject data;
        wxTheClipboard->GetData(data);
        std::vector<byte> bytes;

        if (text) {
            if (off % hexbed::ui::configUtfGroupSize()) {
                wxMessageBox(_("Cannot paste Unicode text out of alignment."),
                             "HexBed", wxOK | wxICON_ERROR);
                goto fail;
            }
            std::u32string s =
                wstringToU32string(data.GetText().ToStdWstring());
            unsigned utf = config().utfMode;
            bytes.resize(s.size() * (utf ? 4 : 1));
            len = bytes.size();
            if (!convertCharsTo(utf, len, bytes.data(), s)) {
                wxMessageBox(
                    _("Cannot paste the current clipboard contents, "
                      "because it contains characters that the currently "
                      "selected character encoding cannot represent."),
                    "HexBed", wxOK | wxICON_ERROR);
                goto fail;
            }
            bytes.resize(len);
        } else {
            string s = stringFromWx(data.GetText());
            len = 0;
            bool ok = hexToBytes(len, nullptr, s);
            if (!ok) goto fail;
            bytes.resize(len);
            if (!hexToBytes(len, bytes.data(), s)) {
                wxMessageBox(
                    _("Cannot paste the current clipboard contents, "
                      "because it does not contain valid hexadecimal "
                      "data. Text data should be a string of hexadecimal "
                      "bytes."),
                    "HexBed", wxOK | wxICON_ERROR);
                goto fail;
            }
        }

        if (insert)
            document.replace(off, cnt, bytespan(bytes.begin(), bytes.end()));
        else
            document.impose(off, bytespan(bytes.begin(), bytes.end()));
        len = bytes.size();
    }

    goto finish;
fail:
    flag = false;
finish:
    wxTheClipboard->Close();
    return flag;
}

};  // namespace clip
};  // namespace hexbed
