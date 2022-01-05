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
// ui/settings/general.cc -- impl for the general preferences page

#include "ui/settings/general.hh"

#include "ui/config.hh"
#include "ui/settings/controls.hh"

namespace hexbed {

namespace ui {

HexBedPrefsGeneralPage::HexBedPrefsGeneralPage(ConfigurationValues* cfg)
    : wxStockPreferencesPage(Kind_General), cfg_(cfg) {}

wxWindow* HexBedPrefsGeneralPage::CreateWindow(wxWindow* parent) {
    return new HexBedPrefsGeneral(parent, cfg_);
}

HexBedPrefsGeneral::HexBedPrefsGeneral(wxWindow* parent,
                                       ConfigurationValues* cfg)
    : HexBedPrefsPage(parent, cfg) {
    PREFS_MAKECOLUMN(col);
    PREFS_HEADING(col, _("Undo"));
    PREFS_LABEL(col,
                _("Note that the undo history may be shorter, as older undo "
                  "history will be dropped in case of insufficient memory."));
    PREFS_SETTING_INT(col, _("Maximum length of undo history"),
                      undoHistoryMaximum, 1, std::numeric_limits<int>::max());
    PREFS_HEADING(col, _("Backup"));
    PREFS_SETTING_BOOL(col, _("Back up files (.bak) before overwriting"),
                       backupFiles);
    PREFS_FINISHCOLUMN(col);
}

};  // namespace ui

};  // namespace hexbed
