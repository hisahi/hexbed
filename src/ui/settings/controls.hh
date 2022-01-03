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
// ui/settings/controls.hh -- header for common preferences page controls

#ifndef HEXBED_UI_SETTINGS_CONTROLS_HH
#define HEXBED_UI_SETTINGS_CONTROLS_HH

#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/clrpicker.h>
#include <wx/font.h>
#include <wx/fontpicker.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/statbox.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/valgen.h>

#include <vector>

#include "common/flags.hh"
#include "common/logger.hh"
#include "ui/settings/validate.hh"

namespace hexbed {

namespace ui {

#define PREFS_MAKECOLUMN(col)                                         \
    wxScrolledWindow* col##_panel = new wxScrolledWindow(             \
        this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL); \
    wxBoxSizer* col##_container = new wxBoxSizer(wxVERTICAL);         \
    wxBoxSizer* col##_margin = new wxBoxSizer(wxVERTICAL);            \
    wxBoxSizer* col = new wxBoxSizer(wxVERTICAL);                     \
    col##_panel->SetScrollRate(1, 1);                                 \
    col##_container->Add(col##_panel, wxSizerFlags().Expand().Proportion(1));

#define PREFS_HEADING(col, text)                                             \
    do {                                                                     \
        wxStaticText* label =                                                \
            new wxStaticText(col##_panel, wxID_ANY, text, wxDefaultPosition, \
                             wxDefaultSize, wxST_ELLIPSIZE_END);             \
        col->Add(label);                                                     \
        label->SetFont(wxFont(label->GetFont()).MakeBold().MakeLarger());    \
    } while (0)

#define PREFS_SEPARATOR(col)                                                \
    do {                                                                    \
        col->Add(new wxStaticLine(col##_panel, wxID_ANY, wxDefaultPosition, \
                                  wxSize(500, 1), wxLI_HORIZONTAL),         \
                 wxSizerFlags().Border(wxTOP, 10).Border(wxBOTTOM, 10));    \
    } while (0)

// TODO: replace with something better
#define PREFS_LABEL(col, text)                                             \
    do {                                                                   \
        wxTextCtrl* label =                                                \
            new wxTextCtrl(col##_panel, wxID_ANY, text, wxDefaultPosition, \
                           wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY); \
        col->Add(label, wxSizerFlags().Expand());                          \
    } while (0)

#define PREFS_SETTING_FRAME(col, text, ctrl)                                 \
    do {                                                                     \
        wxStaticText* label =                                                \
            new wxStaticText(col##_panel, wxID_ANY, text, wxDefaultPosition, \
                             wxDefaultSize, wxST_ELLIPSIZE_END);             \
        wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);                      \
        row->Add(label, wxSizerFlags().Center().Proportion(1));              \
        row->AddStretchSpacer();                                             \
        row->Add(ctrl);                                                      \
        col->Add(row, wxSizerFlags().Expand());                              \
    } while (0)

#define PREFS_SETTING_BOOL(col, text, name)                               \
    do {                                                                  \
        col->Add(new wxCheckBox(                                          \
                     col##_panel, wxID_ANY, text, wxDefaultPosition,      \
                     wxDefaultSize, wxCHK_2STATE,                         \
                     wxGenericValidator(static_cast<bool*>(&cfg->name))), \
                 wxSizerFlags().Expand());                                \
    } while (0)

#define PREFS_SETTING_INT(col, text, name, vmin, vmax)               \
    do {                                                             \
        wxSpinCtrl* picker = new wxSpinCtrl(                         \
            col##_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, \
            wxDefaultSize, wxSP_ARROW_KEYS, vmin, vmax, vmin);       \
        picker->SetValidator(                                        \
            GenericLongValidator(static_cast<long*>(&cfg->name)));   \
        PREFS_SETTING_FRAME(col, text, picker);                      \
    } while (0)

#define PREFS_SETTING_CHOICE(col, text, name, type, ...)                   \
    do {                                                                   \
        std::pair<std::vector<wxString>, std::vector<type>> choices = {    \
            __VA_ARGS__};                                                  \
        HEXBED_ASSERT(choices.first.size() == choices.second.size());      \
        wxChoice* choice = new wxChoice(                                   \
            col##_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize,       \
            choices.first.size(), choices.first.data(), 0,                 \
            ChoiceValidator<type>(&cfg->name, std::move(choices.second))); \
        PREFS_SETTING_FRAME(col, text, choice);                            \
    } while (0)

#define PREFS_SETTING_COLOR(col, text, name)                                   \
    do {                                                                       \
        wxColourPickerCtrl* picker = new wxColourPickerCtrl(                   \
            col##_panel, wxID_ANY, *wxBLACK, wxDefaultPosition, wxDefaultSize, \
            wxCLRP_SHOW_LABEL, ColourValidator(&cfg->name));                   \
        PREFS_SETTING_FRAME(col, text, picker);                                \
    } while (0)

#define PREFS_FINISHCOLUMN(col)                                            \
    do {                                                                   \
        col##_margin->Add(                                                 \
            col, wxSizerFlags().Expand().Proportion(1).Border(wxALL, 10)); \
        col##_panel->SetSizer(col##_margin);                               \
        col->Layout();                                                     \
        SetSizer(col##_container);                                         \
        Layout();                                                          \
    } while (0)

};  // namespace ui

};  // namespace hexbed

#endif /* HEXBED_UI_SETTINGS_CONTROLS_HH */
