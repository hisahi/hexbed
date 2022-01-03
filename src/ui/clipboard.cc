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

#include "app/config.hh"
#include "common/charconv.hh"
#include "common/hexconv.hh"

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
    if (!wxTheClipboard->Open())
        throw std::runtime_error(
            _("Failed to open the clipboard").ToStdString());
    bufsize q = 0, qq;
    byte buf[256];
    if (text) {
        std::wstring result;

        while (cnt) {
            qq = std::min<bufsize>(cnt, sizeof(buf));
            q = document.read(off, bytespan{buf, qq});
            result += sbcsFromBytes(q, buf);
            if (q < qq) break;
            off += qq, cnt -= qq;
        }

        wxTheClipboard->SetData(new wxTextDataObject(result));
    } else {
        std::string result;
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
    if (!wxTheClipboard->Open())
        throw std::runtime_error(
            _("Failed to open the clipboard").ToStdString());
    if (!wxTheClipboard->IsSupported(wxDF_TEXT))
        goto fail;
    else {
        wxTextDataObject data;
        wxTheClipboard->GetData(data);
        std::vector<byte> bytes;

        if (text) {
            std::wstring s = data.GetText().ToStdWstring();
            len = 0;
            bool ok = sbcsToBytes(len, nullptr, s);
            if (!ok) goto fail;
            bytes.resize(len);
            if (!sbcsToBytes(len, bytes.data(), s)) goto fail;
        } else {
            std::string s = data.GetText().ToStdString();
            len = 0;
            bool ok = hexToBytes(len, nullptr, s);
            if (!ok) goto fail;
            bytes.resize(len);
            if (!hexToBytes(len, bytes.data(), s)) goto fail;
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
