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
// ui/settings/layout.hh -- header for the layout preferences page

#ifndef HEXBED_UI_SETTINGS_LAYOUT_HH
#define HEXBED_UI_SETTINGS_LAYOUT_HH

#include "common/config.hh"
#include "ui/settings.hh"

namespace hexbed {

namespace ui {

class HexBedPrefsLayoutPage : public wxPreferencesPage {
  public:
    HexBedPrefsLayoutPage(ConfigurationValues* cfg);

    wxString GetName() const override;
    wxBitmap GetLargeIcon() const override;
    wxWindow* CreateWindow(wxWindow* parent) override;

  private:
    ConfigurationValues* cfg_;
};

class HexBedPrefsLayout : public HexBedPrefsPage {
  public:
    HexBedPrefsLayout(wxWindow* parent, ConfigurationValues* cfg);
};

};  // namespace ui

};  // namespace hexbed

#endif /* HEXBED_UI_SETTINGS_LAYOUT_HH */
