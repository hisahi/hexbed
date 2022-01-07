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
// ui/dialog/bitopunary.cc -- impl for the block unary bit op dialog

#include "ui/dialogs/bitopunary.hh"

#include <wx/sizer.h>

#include "ui/hexbed.hh"
#include "ui/settings/validate.hh"

namespace hexbed {

namespace ui {

BitwiseUnaryOpDialog::BitwiseUnaryOpDialog(HexBedMainFrame* parent)
    : wxDialog(parent, wxID_ANY, _("Bitwise unary operation"),
               wxDefaultPosition, wxSize(300, 250),
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {
    SetReturnCode(wxID_CANCEL);

    wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);
    wxStdDialogButtonSizer* buttons = new wxStdDialogButtonSizer();

    wxButton* okButton = new wxButton(this, wxID_OK);
    okButton->Bind(wxEVT_BUTTON, &BitwiseUnaryOpDialog::OnOK, this);

    wxButton* cancelButton = new wxButton(this, wxID_CANCEL);
    cancelButton->Bind(wxEVT_BUTTON, &BitwiseUnaryOpDialog::OnCancel, this);

    buttons->SetAffirmativeButton(okButton);
    buttons->SetNegativeButton(cancelButton);
    buttons->Realize();

    std::vector<wxString> choiceTexts{_("NOT")};
    opChoice_ = new wxChoice(
        this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choiceTexts.size(),
        choiceTexts.data(), 0,
        ChoiceValidator<BitwiseUnaryOp>(
            &choice_, std::vector<BitwiseUnaryOp>{BitwiseUnaryOp::Not}));

    top->Add(new wxStaticText(this, wxID_ANY, _("Operation:")));
    top->Add(opChoice_, wxSizerFlags().Expand());
    top->Add(buttons, wxSizerFlags().Expand());

    SetSizer(top);
    wxSize sz = GetSize();
    Fit();
    SetSize(sz.GetWidth(), GetSize().GetHeight());
    FitInside();
    Layout();
}

BitwiseUnaryOp BitwiseUnaryOpDialog::GetOperation() const noexcept {
    return choice_;
}

void BitwiseUnaryOpDialog::EndDialog(int r) {
    SetReturnCode(r);
    if (IsModal()) EndModal(r);
}

void BitwiseUnaryOpDialog::OnOK(wxCommandEvent& event) { EndDialog(wxID_OK); }

void BitwiseUnaryOpDialog::OnCancel(wxCommandEvent& event) {
    EndDialog(wxID_CANCEL);
}

};  // namespace ui

};  // namespace hexbed
