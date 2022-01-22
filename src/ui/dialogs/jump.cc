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
// ui/dialog/jump.cc -- impl for the jump (quick go to) dialog

#include "ui/dialogs/jump.hh"

#include <wx/sizer.h>

#include "app/config.hh"
#include "common/hexconv.hh"
#include "ui/string.hh"

namespace hexbed {

namespace ui {

OffsetJumpDialog::OffsetJumpDialog(wxWindow* parent, bufsize cur, bufsize end)
    : wxDialog(parent, wxID_ANY, _("Jump"), wxDefaultPosition,
               wxSize(300, 200)),
      cur_(cur),
      end_(end) {
    SetReturnCode(wxID_CANCEL);

    wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);

    wxStdDialogButtonSizer* buttons = new wxStdDialogButtonSizer();

    okButton_ = new wxButton(this, wxID_OK);
    okButton_->Bind(wxEVT_BUTTON, &OffsetJumpDialog::OnOK, this);

    wxButton* cancelButton = new wxButton(this, wxID_CANCEL);
    cancelButton->Bind(wxEVT_BUTTON, &OffsetJumpDialog::OnCancel, this);

    buttons->SetAffirmativeButton(okButton_);
    buttons->SetNegativeButton(cancelButton);
    buttons->Realize();

    text_ = new wxTextCtrl(
        this, wxID_ANY,
        convertBaseTo(cur_, config().offsetRadix, config().uppercase),
        wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_RIGHT);
    text_->Bind(wxEVT_TEXT, &OffsetJumpDialog::OnTextInput, this);
    text_->Bind(wxEVT_TEXT_ENTER, &OffsetJumpDialog::OnOK, this);

    top->Add(text_, wxSizerFlags().Expand());
    top->Add(buttons, wxSizerFlags().Expand());
    text_->SetFocus();
    text_->SelectAll();
    okButton_->Enable(CheckInput());

    SetSizer(top);
    wxSize sz = GetSize();
    Fit();
    SetSize(sz.GetWidth(), GetSize().GetHeight());
    SetSizeHints(GetSize().GetWidth(), GetSize().GetHeight());
    FitInside();
    Layout();
}

bufsize OffsetJumpDialog::GetOffset() const noexcept {
    bufsize o;
    if (!convertBaseFrom(o, stringFromWx(text_->GetValue()),
                         config().offsetRadix))
        return 0;
    return o;
}

void OffsetJumpDialog::EndDialog(int r) {
    SetReturnCode(r);
    if (IsModal()) EndModal(r);
}

bool OffsetJumpDialog::CheckInput() {
    bufsize o;
    return convertBaseFrom(o, stringFromWx(text_->GetValue()),
                           config().offsetRadix) &&
           o <= end_;
}

void OffsetJumpDialog::OnOK(wxCommandEvent& event) {
    if (!CheckInput()) return;
    EndDialog(wxID_OK);
}

void OffsetJumpDialog::OnCancel(wxCommandEvent& event) {
    EndDialog(wxID_CANCEL);
}

void OffsetJumpDialog::OnTextInput(wxCommandEvent& event) {
    okButton_->Enable(CheckInput());
}

};  // namespace ui

};  // namespace hexbed
