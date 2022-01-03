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
// ui/settings/appearance.cc -- impl for the appearance preferences page

#include "ui/settings/appearance.hh"

#include <wx/msgdlg.h>

#include "ui/config.hh"
#include "ui/settings/controls.hh"

namespace hexbed {

namespace ui {

HexBedPrefsAppearancePage::HexBedPrefsAppearancePage(ConfigurationValues* cfg)
    : cfg_(cfg) {}

wxString HexBedPrefsAppearancePage::GetName() const { return _("Appearance"); }

wxBitmap HexBedPrefsAppearancePage::GetLargeIcon() const {
    return wxBitmap(32, 32);
}

wxWindow* HexBedPrefsAppearancePage::CreateWindow(wxWindow* parent) {
    return new HexBedPrefsAppearance(parent, cfg_);
}

#define PREFS_SETTING_FONT(col, text, name)                                  \
    do {                                                                     \
        wxStaticText* label =                                                \
            new wxStaticText(col##_panel, wxID_ANY, text, wxDefaultPosition, \
                             wxDefaultSize, wxST_ELLIPSIZE_END);             \
        wxFontPickerCtrl* picker = new wxFontPickerCtrl(                     \
            col##_panel, wxID_ANY, wxNullFont, wxDefaultPosition,            \
            wxDefaultSize,                                                   \
            wxFNTP_FONTDESC_AS_LABEL | wxFNTP_USEFONT_FOR_LABEL,             \
            FontValidator(&cfg->name));                                      \
        wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);                      \
        picker->Bind(wxEVT_FONTPICKER_CHANGED,                               \
                     &HexBedPrefsAppearance::OnFontPicked, this);            \
        picker->SetMaxPointSize(MAXIMUM_HEX_FONTSIZE);                       \
        row->Add(label, wxSizerFlags().Center().Proportion(1));              \
        row->AddStretchSpacer();                                             \
        row->Add(picker);                                                    \
        col->Add(row, wxSizerFlags().Expand());                              \
    } while (0)

HexBedPrefsAppearance::HexBedPrefsAppearance(wxWindow* parent,
                                             ConfigurationValues* cfg)
    : HexBedPrefsPage(parent, cfg) {
    PREFS_MAKECOLUMN(col);
    PREFS_HEADING(col, _("Font"));
    PREFS_SETTING_FONT(col, _("Hex view font"), font);
    PREFS_HEADING(col, _("Colors"));
    PREFS_SETTING_COLOR(col, _("Background"), backgroundColor);
    PREFS_SETTING_COLOR(col, _("Offset labels"), offsetColor);
    PREFS_SEPARATOR(col);
    PREFS_SETTING_COLOR(col, _("Hex data, odd columns"), hexColorOdd);
    PREFS_SETTING_COLOR(col, _("Hex data, even columns"), hexColorEven);
    PREFS_SETTING_COLOR(col, _("Text, printable"), textColor);
    PREFS_SETTING_COLOR(col, _("Text, non-printable"), textNonprintableColor);
    PREFS_SEPARATOR(col);
    PREFS_SETTING_COLOR(col, _("Selection background"), selectBackgroundColor);
    PREFS_SETTING_COLOR(col, _("Selection foreground"), selectForegroundColor);
    PREFS_SETTING_COLOR(col, _("Secondary selection background"),
                        selectAltBackgroundColor);
    PREFS_SETTING_COLOR(col, _("Secondary selection foreground"),
                        selectAltForegroundColor);
    PREFS_SETTING_COLOR(col, _("Alignment hint"), alignColor);
    PREFS_FINISHCOLUMN(col);
}

void HexBedPrefsAppearance::OnFontPicked(wxFontPickerEvent& event) {
    if (!event.GetFont().IsFixedWidth()) {
        wxMessageBox(_("Using a non-monospace font is not recommended."),
                     "HexBed", wxOK | wxICON_EXCLAMATION);
    }
    event.Skip();
}

};  // namespace ui

};  // namespace hexbed
