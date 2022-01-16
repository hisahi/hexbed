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
// ui/settings/layout.cc -- impl for the layout preferences page

#include "ui/settings/layout.hh"

#include "app/encoding.hh"
#include "ui/config.hh"
#include "ui/encoding.hh"
#include "ui/settings/controls.hh"

namespace hexbed {

namespace ui {

HexBedPrefsLayoutPage::HexBedPrefsLayoutPage(ConfigurationValues* cfg)
    : cfg_(cfg) {}

wxString HexBedPrefsLayoutPage::GetName() const { return _("Layout"); }

wxBitmap HexBedPrefsLayoutPage::GetLargeIcon() const {
    return wxBitmap(32, 32);
}

wxWindow* HexBedPrefsLayoutPage::CreateWindow(wxWindow* parent) {
    return new HexBedPrefsLayout(parent, cfg_);
}

HexBedPrefsLayout::HexBedPrefsLayout(wxWindow* parent, ConfigurationValues* cfg)
    : HexBedPrefsPage(parent, cfg) {
    PREFS_MAKECOLUMN(col);
    PREFS_HEADING(col, _("Display"));
    PREFS_SETTING_BOOL(col, _("Use uppercase hex characters (A-F)"), uppercase);
    // PREFS_SETTING_INT
    wxSpinCtrl* picker =
        new wxSpinCtrl(col_panel, wxID_ANY, wxEmptyString, wxDefaultPosition,
                       wxDefaultSize, wxSP_ARROW_KEYS, 1, MAX_COLUMNS, 1);
    picker->SetValidator(
        GenericLongValidator(static_cast<long*>(&cfg->hexColumns)));
    picker->Enable(!cfg->autoFit);
    // PREFS_SETTING_BOOL
    wxCheckBox* check = new wxCheckBox(
        col_panel, wxID_ANY, _("Fit as many columns as possible"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE,
        wxGenericValidator(static_cast<bool*>(&cfg->autoFit)));
    col->Add(check, wxSizerFlags().Expand());
    check->Bind(wxEVT_CHECKBOX,
                [picker, check, cfg](wxCommandEvent& event) -> void {
                    picker->Enable(!check->GetValue());
                });
    PREFS_SETTING_FRAME(col, _("Number of columns"), picker);

    /// group sizes
    PREFS_SETTING_CHOICE(col, _("Group size"), groupSize, long,
                         {_("1 (byte)"), _("2 (word)"), _("4 (dword)"),
                          _("8 (qword)"), _("16 (oword)")},
                         {1L, 2L, 4L, 8L, 16L});
    /// offset radix
    PREFS_SETTING_CHOICE(col, _("Offset radix"), offsetRadix, long,
                         {_("8 (octal)"), _("10 (decimal)"), _("16 (hex)")},
                         {8L, 10L, 16L});
    std::pair<std::vector<wxString>, std::vector<string>> encodingChoices = {
        {SBCS_ENCODING_NAMES()}, {SBCS_ENCODING_KEYS()}};
    HEXBED_ASSERT(encodingChoices.first.size() ==
                  encodingChoices.second.size());
    for (std::size_t i = 0, e = hexbed::plugins::charsetPluginCount(); i < e;
         ++i) {
        const auto& pair = hexbed::plugins::charsetPluginByIndex(i);
        encodingChoices.second.push_back(pair.first);
        encodingChoices.first.push_back(pair.second);
    }
    wxChoice* encodingChoice = new wxChoice(
        col_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        encodingChoices.first.size(), encodingChoices.first.data(), 0,
        ChoiceValidator<string>(&cfg->charset,
                                std::move(encodingChoices.second)));
    PREFS_SETTING_FRAME(col, _("Text encoding"), encodingChoice);

    PREFS_FINISHCOLUMN(col);
}

};  // namespace ui

};  // namespace hexbed
