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
// ui/dialog/random.cc -- impl for the Insert Random block dialog

#include "ui/dialogs/random.hh"

#include <wx/sizer.h>

#include "ui/hexbed.hh"
#include "ui/settings/validate.hh"

namespace hexbed {

namespace ui {

InsertRandomBlockDialog::InsertRandomBlockDialog(HexBedMainFrame* parent,
                                                 bufsize seln)
    : wxDialog(parent, wxID_ANY, _("Insert random"), wxDefaultPosition,
               wxSize(300, 250), wxDEFAULT_DIALOG_STYLE),
      count_(seln) {
    SetReturnCode(wxID_CANCEL);

    wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);
    wxStdDialogButtonSizer* buttons = new wxStdDialogButtonSizer();

    okButton_ = new wxButton(this, wxID_OK);
    okButton_->Bind(wxEVT_BUTTON, &InsertRandomBlockDialog::OnOK, this);

    wxButton* cancelButton = new wxButton(this, wxID_CANCEL);
    cancelButton->Bind(wxEVT_BUTTON, &InsertRandomBlockDialog::OnCancel, this);

    buttons->SetAffirmativeButton(okButton_);
    buttons->SetNegativeButton(cancelButton);
    buttons->Realize();

    spinner_ = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                              wxDefaultSize, wxSP_ARROW_KEYS, 0,
                              std::numeric_limits<int>::max(), seln);
    spinner_->SetValidator(
        hexbed::ui::GenericCustomValidator<bufsize, int>(&count_));

    top->Add(new wxStaticText(this, wxID_ANY, _("Number of bytes:")));
    top->Add(spinner_, wxSizerFlags().Expand());
    top->Add(new wxStaticText(this, wxID_ANY, _("Randomness source")));
    top->Add(new wxRadioButton(this, wxID_ANY, _("Fast, but low quality"),
                               wxDefaultPosition, wxDefaultSize, 0,
                               hexbed::ui::RadioValidator<RandomType>(
                                   &type_, RandomType::Fast)),
             wxSizerFlags().Expand());
    top->Add(new wxRadioButton(this, wxID_ANY, _("Slow, but high quality"),
                               wxDefaultPosition, wxDefaultSize, 0,
                               hexbed::ui::RadioValidator<RandomType>(
                                   &type_, RandomType::Good)),
             wxSizerFlags().Expand());
    top->Add(buttons, wxSizerFlags().Expand());

    SetSizer(top);
    Fit();
    Layout();
    TransferDataToWindow();
}

bufsize InsertRandomBlockDialog::GetByteCount() const noexcept {
    return count_;
}

RandomType InsertRandomBlockDialog::GetRandomType() const noexcept {
    return type_;
}

void InsertRandomBlockDialog::EndDialog(int r) {
    TransferDataFromWindow();
    SetReturnCode(r);
    if (IsModal()) EndModal(r);
}

void InsertRandomBlockDialog::OnOK(wxCommandEvent& event) {
    EndDialog(wxID_OK);
}

void InsertRandomBlockDialog::OnCancel(wxCommandEvent& event) {
    EndDialog(wxID_CANCEL);
}

};  // namespace ui

};  // namespace hexbed
