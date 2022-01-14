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
// ui/dialog/goto.cc -- impl for the Go to dialog

#include "ui/dialogs/goto.hh"

#include <wx/sizer.h>

#include "app/config.hh"
#include "common/hexconv.hh"
#include "ui/string.hh"

namespace hexbed {

namespace ui {

GoToDialog::GoToDialog(wxWindow* parent, bufsize cur, bufsize end)
    : wxDialog(parent, wxID_ANY, _("Go to"), wxDefaultPosition,
               wxSize(300, 200)),
      cur_(cur),
      end_(end) {
    SetReturnCode(wxID_CANCEL);

    wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);

    radix_ = new RadixPicker(this);
    radix_->Bind(hEVT_RADIXCHANGE, &GoToDialog::OnRadixChange, this);

    wxStdDialogButtonSizer* buttons = new wxStdDialogButtonSizer();

    okButton_ = new wxButton(this, wxID_OK);
    okButton_->Bind(wxEVT_BUTTON, &GoToDialog::OnOK, this);

    wxButton* cancelButton = new wxButton(this, wxID_CANCEL);
    cancelButton->Bind(wxEVT_BUTTON, &GoToDialog::OnCancel, this);

    buttons->SetAffirmativeButton(okButton_);
    buttons->SetNegativeButton(cancelButton);
    buttons->Realize();

    wxBoxSizer* baseSizer = new wxBoxSizer(wxHORIZONTAL);
    baseSizer->Add(new wxStaticText(this, wxID_ANY, _("From")),
                   wxSizerFlags().Proportion(1));
    baseSizer->Add(base0_ = new wxRadioButton(this, wxID_ANY, _("Start"),
                                              wxDefaultPosition, wxDefaultSize,
                                              wxRB_GROUP),
                   wxSizerFlags().Proportion(1));
    baseSizer->Add(
        base1_ = new wxRadioButton(this, wxID_ANY, _("Cursor"),
                                   wxDefaultPosition, wxDefaultSize, 0),
        wxSizerFlags().Proportion(1));
    baseSizer->Add(
        base2_ = new wxRadioButton(this, wxID_ANY, _("End"), wxDefaultPosition,
                                   wxDefaultSize, 0),
        wxSizerFlags().Proportion(1));

    base0_->SetValue(true);
    base0_->Bind(wxEVT_RADIOBUTTON, &GoToDialog::OnBaseChange, this);
    base1_->Bind(wxEVT_RADIOBUTTON, &GoToDialog::OnBaseChange, this);
    base2_->Bind(wxEVT_RADIOBUTTON, &GoToDialog::OnBaseChange, this);

    text_ = new wxTextCtrl(
        this, wxID_ANY,
        convertBaseTo(cur_, radix_->GetRadix(), config().uppercase),
        wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_RIGHT);
    text_->Bind(wxEVT_TEXT, &GoToDialog::OnTextInput, this);
    text_->Bind(wxEVT_TEXT_ENTER, &GoToDialog::OnOK, this);

    top->Add(new wxStaticText(this, wxID_ANY, _("Offset:")));
    top->Add(text_, wxSizerFlags().Expand());
    top->Add(radix_, wxSizerFlags().Expand());
    top->Add(baseSizer, wxSizerFlags().Expand());
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

bufsize GoToDialog::GetOffset() const noexcept {
    using enum SeekBase;
    bufsize o;
    int negative;
    if (!convertBaseFromNeg(o, negative, stringFromWx(text_->GetValue()),
                            base_))
        return 0;
    switch (seek_) {
    case Begin:
        return o;
    case Current:
        return negative < 0 ? cur_ - o : cur_ + o;
    case End:
        return end_ - o;
    default:
        return 0;
    }
}

void GoToDialog::EndDialog(int r) {
    SetReturnCode(r);
    if (IsModal()) EndModal(r);
}

void GoToDialog::UpdateMetrics(bufsize cur, bufsize end) {
    cur_ = cur;
    end_ = end;
    okButton_->Enable(CheckInput());
}

bool GoToDialog::CheckInput() {
    using enum SeekBase;
    bufsize o;
    int negative;
    if (convertBaseFromNeg(o, negative, stringFromWx(text_->GetValue()),
                           base_)) {
        switch (seek_) {
        case Begin:
        case End:
            return negative >= 0 && o <= end_;
        case Current:
            return negative < 0 ? o <= cur_ : o <= end_ - cur_;
        }
    }
    return false;
}

void GoToDialog::ConvertBase(unsigned base) {
    bufsize o;
    int negative;
    bool ok =
        convertBaseFromNeg(o, negative, stringFromWx(text_->GetValue()), base_);
    if (ok)
        text_->SetValue(
            convertBaseToNeg(o, negative, base, config().uppercase));
    base_ = base;
}

void GoToDialog::OnOK(wxCommandEvent& event) {
    if (!CheckInput()) return;
    EndDialog(wxID_OK);
}

void GoToDialog::OnCancel(wxCommandEvent& event) { EndDialog(wxID_CANCEL); }

void GoToDialog::OnTextInput(wxCommandEvent& event) {
    okButton_->Enable(CheckInput());
}

void GoToDialog::OnBaseChange(wxCommandEvent& event) {
    if (base1_->GetValue())
        seek_ = SeekBase::Current;
    else if (base2_->GetValue())
        seek_ = SeekBase::End;
    else
        seek_ = SeekBase::Begin;
    okButton_->Enable(CheckInput());
}

void GoToDialog::OnRadixChange(wxCommandEvent& event) {
    ConvertBase(radix_->GetRadix());
    okButton_->Enable(CheckInput());
}

};  // namespace ui

};  // namespace hexbed
