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

#include <cstring>

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

static char nbuf[(std::bit_width(std::numeric_limits<bufsize>::max()) + 2) / 3];
static char buf[101 + sizeof(nbuf)];

void HexEditor::UpdateStatusBar() {
    if (!sbar_) return;
    hexbed::menu::updateStatusBarBase(sbar_, context().state);

    char fc;
    switch (config().offsetRadix) {
    case 8:
        fc = 'o';
        break;
    case 10:
        fc = 'u';
        break;
    case 16:
    default:
        fc = config().uppercase ? 'X' : 'x';
        break;
    }

    char fmt[] = "%llx";
    char fmt2[] = "%llx : %llx";
    fmt2[10] = fmt2[3] = fmt[3] = fc;

    int i = 1;
    std::snprintf(nbuf, sizeof(nbuf), fmt, cur_);
    std::snprintf(buf, sizeof(buf), _("Offset: %s").c_str(), nbuf);
    sbar_->SetStatusText(buf, ++i);

    if (seln_) {
        std::snprintf(nbuf, sizeof(nbuf), fmt2, sel_, sel_ + seln_ - 1);
        std::snprintf(buf, sizeof(buf), _("Block: %s").c_str(), nbuf);
        sbar_->SetStatusText(buf, ++i);
        std::snprintf(nbuf, sizeof(nbuf), fmt, seln_);
        std::snprintf(buf, sizeof(buf), _("Length: %s").c_str(), nbuf);
        sbar_->SetStatusText(buf, ++i);
    } else {
        sbar_->SetStatusText(wxEmptyString, ++i);
        sbar_->SetStatusText(wxEmptyString, ++i);
    }
}

};  // namespace ui
};  // namespace hexbed
