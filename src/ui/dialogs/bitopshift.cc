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
// ui/dialog/bitopshift.cc -- impl for the block shift bit op dialog

#include "ui/dialogs/bitopshift.hh"

#include <wx/sizer.h>

#include "ui/hexbed.hh"
#include "ui/settings/validate.hh"

namespace hexbed {

namespace ui {

BitwiseShiftOpDialog::BitwiseShiftOpDialog(HexBedMainFrame* parent)
    : wxDialog(parent, wxID_ANY, _("Bitwise shift operation"),
               wxDefaultPosition, wxSize(300, 250),
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {
    SetReturnCode(wxID_CANCEL);

    wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);
    wxStdDialogButtonSizer* buttons = new wxStdDialogButtonSizer();

    wxButton* okButton = new wxButton(this, wxID_OK);
    okButton->Bind(wxEVT_BUTTON, &BitwiseShiftOpDialog::OnOK, this);

    wxButton* cancelButton = new wxButton(this, wxID_CANCEL);
    cancelButton->Bind(wxEVT_BUTTON, &BitwiseShiftOpDialog::OnCancel, this);

    buttons->SetAffirmativeButton(okButton);
    buttons->SetNegativeButton(cancelButton);
    buttons->Realize();

    std::vector<wxString> choiceTexts{_("Shift left"), _("Shift right"),
                                      _("Shift right (arithmetic)"),
                                      _("Rotate left"), _("Rotate right")};
    opChoice_ = new wxChoice(
        this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choiceTexts.size(),
        choiceTexts.data(), 0,
        ChoiceValidator<BitwiseShiftOp>(
            &choice_,
            std::vector<BitwiseShiftOp>{
                BitwiseShiftOp::ShiftLeft, BitwiseShiftOp::ShiftRight,
                BitwiseShiftOp::ShiftRightArithmetic,
                BitwiseShiftOp::RotateLeft, BitwiseShiftOp::RotateRight}));

    spinner_ =
        new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                       wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 0, 7, 1);

    top->Add(new wxStaticText(this, wxID_ANY, _("Operation:")));
    top->Add(opChoice_, wxSizerFlags().Expand());
    top->Add(new wxStaticText(this, wxID_ANY, _("Shift count:")));
    top->Add(spinner_, wxSizerFlags().Expand());
    top->Add(buttons, wxSizerFlags().Expand());

    SetSizer(top);
    wxSize sz = GetSize();
    Fit();
    SetSize(sz.GetWidth(), GetSize().GetHeight());
    FitInside();
    Layout();
}

BitwiseShiftOp BitwiseShiftOpDialog::GetOperation() const noexcept {
    return choice_;
}

int BitwiseShiftOpDialog::GetShiftCount() const noexcept { return shiftCount_; }

void BitwiseShiftOpDialog::EndDialog(int r) {
    SetReturnCode(r);
    if (IsModal()) EndModal(r);
}

void BitwiseShiftOpDialog::OnOK(wxCommandEvent& event) { EndDialog(wxID_OK); }

void BitwiseShiftOpDialog::OnCancel(wxCommandEvent& event) {
    EndDialog(wxID_CANCEL);
}

};  // namespace ui

};  // namespace hexbed
