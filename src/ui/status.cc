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
// ui/status.cc -- impl for the status bar

#include <wx/translation.h>

#include <cstdarg>
#include <cstring>
#include <cwchar>

#include "app/config.hh"
#include "ui/hexbed.hh"
#include "ui/hexedit.hh"

namespace hexbed {

namespace menu {

void populateStatusBar(wxStatusBar* sbar) {
    int widths[] = {-3, 80, -1, -2, -1};
    sbar->SetFieldsCount(sizeof(widths) / sizeof(int), widths);
}

static void updateStatusBarBase(wxStatusBar* statusBar,
                                const EditorState& state) {
    if (state.insert)
        statusBar->SetStatusText(_("Insert"), 1);
    else
        statusBar->SetStatusText(_("Overwrite"), 1);
}

void updateStatusBarNoFile(wxStatusBar* statusBar, const EditorState& state) {
    updateStatusBarBase(statusBar, state);
    for (int i = 2, e = statusBar->GetFieldsCount(); i < e; ++i)
        statusBar->SetStatusText(wxEmptyString, i);
}

};  // namespace menu

namespace ui {

static strchar
    nbuf[(std::bit_width(std::numeric_limits<bufsize>::max()) + 2) / 3];
static strchar buf[101 + sizeof(nbuf)];

template <typename T>
static int snprintf(T* out, std::size_t outSize, const T* fmt, ...);

template <>
[[maybe_unused]] int snprintf<char>(char* out, std::size_t outSize,
                                    const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int n = std::vsnprintf(out, outSize, fmt, args);
    va_end(args);
    return n;
}

template <>
[[maybe_unused]] int snprintf<wchar_t>(wchar_t* out, std::size_t outSize,
                                       const wchar_t* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int n = std::vswprintf(out, outSize, fmt, args);
    va_end(args);
    return n;
}

template <typename T>
const T* cify(const wxString& string);

template <>
[[maybe_unused]] const char* cify<char>(const wxString& string) {
    return string.c_str();
}

template <>
[[maybe_unused]] const wchar_t* cify<wchar_t>(const wxString& string) {
    return string.wc_str();
}

void HexEditor::UpdateStatusBar() {
    if (!sbar_) return;
    hexbed::menu::updateStatusBarBase(sbar_, context().state);

    strchar fc;
    switch (config().offsetRadix) {
    case 8:
        fc = CHAR('o');
        break;
    case 10:
        fc = CHAR('u');
        break;
    case 16:
    default:
        fc = config().uppercase ? CHAR('X') : CHAR('x');
        break;
    }

    strchar fmt[] = STRING("%llx");
    strchar fmt2[] = STRING("%llx : %llx");
    fmt2[10] = fmt2[3] = fmt[3] = fc;

    int i = 1;
    snprintf(nbuf, sizeof(nbuf), fmt, cur_);
    snprintf(buf, sizeof(buf), cify<strchar>(_("Offset: %s")), nbuf);
    sbar_->SetStatusText(buf, ++i);

    if (seln_) {
        snprintf(nbuf, sizeof(nbuf), fmt2, sel_, sel_ + seln_ - 1);
        snprintf(buf, sizeof(buf), cify<strchar>(_("Block: %s")), nbuf);
        sbar_->SetStatusText(buf, ++i);
        snprintf(nbuf, sizeof(nbuf), fmt, seln_);
        snprintf(buf, sizeof(buf), cify<strchar>(_("Length: %s")), nbuf);
        sbar_->SetStatusText(buf, ++i);
    } else {
        sbar_->SetStatusText(wxEmptyString, ++i);
        sbar_->SetStatusText(wxEmptyString, ++i);
    }
}

};  // namespace ui
};  // namespace hexbed
