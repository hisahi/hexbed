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
// ui/settings/appearance.hh -- header for the appearance preferences page

#ifndef HEXBED_UI_SETTINGS_APPEARANCE_HH
#define HEXBED_UI_SETTINGS_APPEARANCE_HH

#include <wx/fontpicker.h>

#include "common/config.hh"
#include "ui/settings.hh"

namespace hexbed {

namespace ui {

class HexBedPrefsAppearancePage : public wxPreferencesPage {
  public:
    HexBedPrefsAppearancePage(ConfigurationValues* cfg);

    wxString GetName() const override;
    wxBitmap GetLargeIcon() const override;
    wxWindow* CreateWindow(wxWindow* parent);

  private:
    ConfigurationValues* cfg_;
};

class HexBedPrefsAppearance : public HexBedPrefsPage {
  public:
    HexBedPrefsAppearance(wxWindow* parent, ConfigurationValues* cfg);

  private:
    void OnFontPicked(wxFontPickerEvent& event);
};

};  // namespace ui

};  // namespace hexbed

#endif /* HEXBED_UI_SETTINGS_APPEARANCE_HH */
