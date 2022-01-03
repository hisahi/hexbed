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
// ui/settings.hh -- header for the HexBed preferences dialog

#ifndef HEXBED_UI_SETTINGS_HH
#define HEXBED_UI_SETTINGS_HH

#include <wx/dialog.h>
#include <wx/intl.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/preferences.h>

#include "common/config.hh"
#include "common/flags.hh"
#include "ui/hexbed.hh"

namespace hexbed {

namespace ui {

class HexBedPrefsPage : public wxPanel {
  public:
    HexBedPrefsPage(wxWindow* parent, ConfigurationValues* cfg);

  protected:
    ConfigurationValues& config();

  private:
    ConfigurationValues* cfg_;
};

#if !HEXBED_USE_NATIVE_PREFS

class HexBedPrefsDialog : public wxDialog {
  public:
    HexBedPrefsDialog(HexBedMainFrame* parent);

    bool DoOK();

  protected:
    void OnClose(wxCloseEvent& event);
    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnApply(wxCommandEvent& event);

  private:
    void EndDialog(int result);
    bool UpdateSettings();
    bool UpdateSettingsEx();
    template <typename T>
    void AddSettingPage();
    void AddSettings();
    void ApplySettings();
    void RevertSettings();
    void OnOK();

    HexBedMainFrame* main_;
    wxNotebook* book_;
    ConfigurationValues oldConfig_;
    ConfigurationValues newConfig_;
    bool applied_{false};
};

#endif

void InitPreferences(HexBedMainFrame* main);
void ShowPreferences(HexBedMainFrame* main);

};  // namespace ui

};  // namespace hexbed

#endif /* HEXBED_UI_SETTINGS_HH */
