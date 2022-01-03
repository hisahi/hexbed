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
// ui/settings.cc -- impl for the HexBed preferences dialog

#include "ui/settings.hh"

#include <wx/button.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>

#include "app/config.hh"
#include "common/logger.hh"
#include "ui/settings/appearance.hh"
#include "ui/settings/general.hh"
#include "ui/settings/layout.hh"

namespace hexbed {

namespace ui {

HexBedPrefsPage::HexBedPrefsPage(wxWindow* parent, ConfigurationValues* cfg)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize), cfg_(cfg) {}

ConfigurationValues& HexBedPrefsPage::config() { return *cfg_; }

#define ADDPAGES()                     \
    ADDPAGE(HexBedPrefsGeneralPage)    \
    ADDPAGE(HexBedPrefsAppearancePage) \
    ADDPAGE(HexBedPrefsLayoutPage)

#if HEXBED_USE_NATIVE_PREFS

static wxPreferencesEditor* prefsEditor;

void InitPreferences(HexBedMainFrame* main) {
    prefsEditor = new wxPreferencesEditor(_("Options"));
    ConfigurationValues* cfg = &currentConfig.values();
#define ADDPAGE(T) prefsEditor->AddPage(new T(cfg));
    ADDPAGES()
}

void ShowPreferences(HexBedMainFrame* main) {
    if (!prefsEditor) InitPreferences(main);
    prefsEditor->Show(main);
    if (!wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        main_->ApplyConfig();
    }
}

#else

void InitPreferences(HexBedMainFrame* main) {}

void ShowPreferences(HexBedMainFrame* main) {
    HexBedPrefsDialog(main).ShowModal();
}

HexBedPrefsDialog::HexBedPrefsDialog(HexBedMainFrame* parent)
    : wxDialog(parent, wxID_ANY, _("Options"), wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      main_(parent),
      book_(new wxNotebook(this, wxID_ANY)),
      oldConfig_(currentConfig.values()),
      newConfig_(currentConfig.values()) {
    SetReturnCode(wxID_CANCEL);
    SetSizeHints(GetSize().GetWidth(), GetSize().GetHeight());

    wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);
    SetSizer(top);

    wxStdDialogButtonSizer* buttons = new wxStdDialogButtonSizer();

    wxButton* okButton = new wxButton(this, wxID_OK);
    okButton->Bind(wxEVT_BUTTON, &HexBedPrefsDialog::OnOK, this);

    wxButton* cancelButton = new wxButton(this, wxID_CANCEL);
    cancelButton->Bind(wxEVT_BUTTON, &HexBedPrefsDialog::OnCancel, this);

    wxButton* applyButton = new wxButton(this, wxID_APPLY);
    applyButton->Bind(wxEVT_BUTTON, &HexBedPrefsDialog::OnApply, this);

    buttons->SetAffirmativeButton(okButton);
    buttons->SetNegativeButton(cancelButton);
    buttons->AddButton(applyButton);
    buttons->Realize();

    top->Add(book_, wxSizerFlags().Expand().Proportion(1));
    top->Add(buttons);
    Layout();
    AddSettings();
}

template <typename T>
void HexBedPrefsDialog::AddSettingPage() {
    T page(&newConfig_);
    wxWindow* wnd = page.CreateWindow(book_);
    book_->AddPage(wnd, page.GetName(), false);
    wnd->TransferDataToWindow();
}

void HexBedPrefsDialog::AddSettings() {
#define ADDPAGE(T) AddSettingPage<T>();
    ADDPAGES()
}

void HexBedPrefsDialog::EndDialog(int r) {
    SetReturnCode(r);
    if (IsModal()) EndModal(r);
}

bool HexBedPrefsDialog::UpdateSettings() {
    for (size_t i = 0, e = book_->GetPageCount(); i < e; ++i) {
        wxWindow* win = book_->GetPage(i);
        if (!win->Validate()) return false;
        if (!win->TransferDataFromWindow()) return false;
    }
    return true;
}

bool HexBedPrefsDialog::UpdateSettingsEx() {
    bool ok = UpdateSettings();
    if (!ok) {
        wxMessageBox(
            _("One or more of the settings you have specified is invalid. "
              "Please check your settings and try again."),
            "HexBed", wxOK | wxICON_ERROR);
    }
    return ok;
}

bool HexBedPrefsDialog::DoOK() {
    bool ok = UpdateSettingsEx();
    if (ok) OnOK();
    return ok;
}

void HexBedPrefsDialog::OnOK() {
    ApplySettings();
    currentConfig.save();
    EndDialog(wxID_OK);
}

void HexBedPrefsDialog::OnOK(wxCommandEvent& event) {
    if (!UpdateSettingsEx()) return;
    OnOK();
}

void HexBedPrefsDialog::OnCancel(wxCommandEvent& event) {
    EndDialog(wxID_CANCEL);
}

void HexBedPrefsDialog::OnApply(wxCommandEvent& event) {
    if (!UpdateSettingsEx()) return;
    ApplySettings();
}

void HexBedPrefsDialog::ApplySettings() {
    applied_ = true;
    currentConfig.values() = newConfig_;
    main_->ApplyConfig();
}

void HexBedPrefsDialog::RevertSettings() {
    if (applied_) {
        currentConfig.values() = oldConfig_;
        main_->ApplyConfig();
        applied_ = false;
    }
}

void HexBedPrefsDialog::OnClose(wxCloseEvent& event) {
    if (GetReturnCode() != wxID_OK) RevertSettings();
    event.Skip();
}

#endif

};  // namespace ui

};  // namespace hexbed
