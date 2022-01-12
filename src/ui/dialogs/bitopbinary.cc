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
// ui/dialog/bitopbinary.cc -- impl for the block binary bit op dialog

#include "ui/dialogs/bitopbinary.hh"

#include <wx/sizer.h>

#include "ui/hexbed.hh"
#include "ui/settings/validate.hh"

namespace hexbed {

namespace ui {

BitwiseBinaryOpDialog::BitwiseBinaryOpDialog(
    HexBedMainFrame* parent, std::shared_ptr<HexBedContextMain> context,
    std::shared_ptr<HexBedDocument> document)
    : wxDialog(parent, wxID_ANY, _("Binary bitwise operation"),
               wxDefaultPosition, wxSize(300, 250),
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      context_(context),
      document_(document) {
    SetReturnCode(wxID_CANCEL);

    wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);
    wxStdDialogButtonSizer* buttons = new wxStdDialogButtonSizer();

    okButton_ = new wxButton(this, wxID_OK);
    okButton_->Bind(wxEVT_BUTTON, &BitwiseBinaryOpDialog::OnOK, this);

    wxButton* cancelButton = new wxButton(this, wxID_CANCEL);
    cancelButton->Bind(wxEVT_BUTTON, &BitwiseBinaryOpDialog::OnCancel, this);

    buttons->SetAffirmativeButton(okButton_);
    buttons->SetNegativeButton(cancelButton);
    buttons->Realize();

    decltype(document) copy = document;
    editor_ = new HexBedStandaloneEditor(this, context.get(), std::move(copy));
    editor_->Bind(HEX_EDIT_EVENT, &BitwiseBinaryOpDialog::OnChangedInput, this);
    registration_ = HexBedEditorRegistration(context, editor_);

    std::vector<wxString> choiceTexts{_("Sum"), _("AND"), _("OR"), _("XOR")};
    opChoice_ = new wxChoice(
        this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choiceTexts.size(),
        choiceTexts.data(), 0,
        ChoiceValidator<BitwiseBinaryOp>(
            &choice_, std::vector<BitwiseBinaryOp>{
                          BitwiseBinaryOp::Add, BitwiseBinaryOp::And,
                          BitwiseBinaryOp::Or, BitwiseBinaryOp::Xor}));

    top->Add(new wxStaticText(this, wxID_ANY, _("Operation:")));
    top->Add(opChoice_, wxSizerFlags().Expand());
    top->Add(new wxStaticText(
        this, wxID_ANY, _("The bytes below will form a repeating pattern.")));
    top->Add(editor_, wxSizerFlags().Expand().Proportion(1));
    top->Add(buttons, wxSizerFlags().Expand());
    editor_->SetFocus();
    editor_->SelectAll(SelectFlags().caretAtBeginning().highlightCaret());
    okButton_->Enable(CheckInput());

    SetSizer(top);
    Fit();
    SetSizeHints(GetSize().GetWidth(), GetSize().GetHeight(), -1, -1);
    TransferDataToWindow();
}

BitwiseBinaryOp BitwiseBinaryOpDialog::GetOperation() const noexcept {
    return choice_;
}

void BitwiseBinaryOpDialog::EndDialog(int r) {
    SetReturnCode(r);
    if (IsModal()) EndModal(r);
}

bool BitwiseBinaryOpDialog::CheckInput() { return document_->size() > 0; }

void BitwiseBinaryOpDialog::OnOK(wxCommandEvent& event) {
    if (CheckInput() && Validate() && TransferDataFromWindow())
        EndDialog(wxID_OK);
}

void BitwiseBinaryOpDialog::OnCancel(wxCommandEvent& event) {
    EndDialog(wxID_CANCEL);
}

void BitwiseBinaryOpDialog::OnChangedInput(wxCommandEvent& event) {
    okButton_->Enable(CheckInput());
}

};  // namespace ui

};  // namespace hexbed
