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
// ui/dialog/insert.cc -- impl for the Insert block dialog

#include "ui/dialogs/insert.hh"

#include <wx/sizer.h>

#include "ui/hexbed.hh"

namespace hexbed {

namespace ui {

InsertBlockDialog::InsertBlockDialog(HexBedMainFrame* parent,
                                     std::shared_ptr<HexBedContextMain> context,
                                     std::shared_ptr<HexBedDocument> document,
                                     bufsize seln)
    : wxDialog(parent, wxID_ANY, _("Insert or replace"), wxDefaultPosition,
               wxSize(300, 250), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      context_(context),
      document_(document) {
    SetReturnCode(wxID_CANCEL);

    wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);
    wxStdDialogButtonSizer* buttons = new wxStdDialogButtonSizer();

    okButton_ = new wxButton(this, wxID_OK);
    okButton_->Bind(wxEVT_BUTTON, &InsertBlockDialog::OnOK, this);

    wxButton* cancelButton = new wxButton(this, wxID_CANCEL);
    cancelButton->Bind(wxEVT_BUTTON, &InsertBlockDialog::OnCancel, this);

    buttons->SetAffirmativeButton(okButton_);
    buttons->SetNegativeButton(cancelButton);
    buttons->Realize();

    decltype(document) copy = document;
    editor_ = new HexBedStandaloneEditor(this, context.get(), std::move(copy));
    editor_->Bind(HEX_EDIT_EVENT, &InsertBlockDialog::OnChangedInput, this);
    registration_ = HexBedEditorRegistration(context, editor_);

    spinner_ = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                              wxDefaultSize, wxSP_ARROW_KEYS, 0,
                              std::numeric_limits<int>::max(), seln);

    top->Add(new wxStaticText(this, wxID_ANY, _("Number of bytes:")));
    top->Add(spinner_, wxSizerFlags().Expand());
    top->Add(new wxStaticText(
        this, wxID_ANY, _("The bytes below will form a repeating pattern.")));
    top->Add(editor_, wxSizerFlags().Expand().Proportion(1));
    top->Add(buttons, wxSizerFlags().Expand());
    editor_->SetFocus();
    editor_->SelectAll(SelectFlags().caretAtBeginning().highlightCaret());
    okButton_->Enable(CheckInput());

    SetSizer(top);
    Layout();
}

bufsize InsertBlockDialog::GetByteCount() const noexcept {
    return spinner_->GetValue();
}

void InsertBlockDialog::EndDialog(int r) {
    SetReturnCode(r);
    if (IsModal()) EndModal(r);
}

bool InsertBlockDialog::CheckInput() { return document_->size() > 0; }

void InsertBlockDialog::OnOK(wxCommandEvent& event) {
    if (!CheckInput()) return;
    EndDialog(wxID_OK);
}

void InsertBlockDialog::OnCancel(wxCommandEvent& event) {
    EndDialog(wxID_CANCEL);
}

void InsertBlockDialog::OnChangedInput(wxCommandEvent& event) {
    okButton_->Enable(CheckInput());
}

};  // namespace ui

};  // namespace hexbed
