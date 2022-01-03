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
// ui/dialog/selectblock.cc -- impl for the Select block dialog

#include "ui/dialogs/selectblock.hh"

#include <wx/sizer.h>

#include "app/config.hh"
#include "common/hexconv.hh"

namespace hexbed {

namespace ui {

SelectBlockDialog::SelectBlockDialog(wxWindow* parent, bufsize cur, bufsize len,
                                     bufsize end)
    : wxDialog(parent, wxID_ANY, _("Select block"), wxDefaultPosition,
               wxSize(300, 250)),
      cur_(cur),
      len_(len),
      end_(end) {
    SetReturnCode(wxID_CANCEL);

    wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);

    radix_ = new RadixPicker(this);
    radix_->Bind(hEVT_RADIXCHANGE, &SelectBlockDialog::OnRadixChange, this);

    wxStdDialogButtonSizer* buttons = new wxStdDialogButtonSizer();

    okButton_ = new wxButton(this, wxID_OK);
    okButton_->Bind(wxEVT_BUTTON, &SelectBlockDialog::OnOK, this);

    wxButton* cancelButton = new wxButton(this, wxID_CANCEL);
    cancelButton->Bind(wxEVT_BUTTON, &SelectBlockDialog::OnCancel, this);

    buttons->SetAffirmativeButton(okButton_);
    buttons->SetNegativeButton(cancelButton);
    buttons->Realize();

    wxBoxSizer* offSizer = new wxBoxSizer(wxHORIZONTAL);
    offSizer->Add(
        off0_ = new wxRadioButton(this, wxID_ANY, _("Length:"),
                                  wxDefaultPosition, wxDefaultSize, wxRB_GROUP),
        wxSizerFlags().Proportion(1));
    offSizer->Add(
        off1_ = new wxRadioButton(this, wxID_ANY, _("End:"), wxDefaultPosition,
                                  wxDefaultSize, 0),
        wxSizerFlags().Proportion(1));

    off0_->SetValue(true);
    off0_->Bind(wxEVT_RADIOBUTTON, &SelectBlockDialog::OnOffsetChange, this);
    off1_->Bind(wxEVT_RADIOBUTTON, &SelectBlockDialog::OnOffsetChange, this);

    text_ = new wxTextCtrl(
        this, wxID_ANY,
        convertBaseTo(cur_, radix_->GetRadix(), config().uppercase),
        wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_RIGHT);
    text_->Bind(wxEVT_TEXT, &SelectBlockDialog::OnTextInput, this);
    text_->Bind(wxEVT_TEXT_ENTER, &SelectBlockDialog::OnEnterFirst, this);

    text2_ = new wxTextCtrl(
        this, wxID_ANY,
        convertBaseTo(len_, radix_->GetRadix(), config().uppercase),
        wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_RIGHT);
    text2_->Bind(wxEVT_TEXT, &SelectBlockDialog::OnTextInput, this);
    text2_->Bind(wxEVT_TEXT_ENTER, &SelectBlockDialog::OnOK, this);

    top->Add(new wxStaticText(this, wxID_ANY, _("Start:")));
    top->Add(text_, wxSizerFlags().Expand());
    top->Add(offSizer, wxSizerFlags().Expand());
    top->Add(text2_, wxSizerFlags().Expand());
    top->Add(radix_, wxSizerFlags().Expand());
    top->Add(buttons, wxSizerFlags().Expand());
    text_->SetFocus();
    text_->SelectAll();
    text2_->SelectAll();
    okButton_->Enable(CheckInput());

    SetSizer(top);
    wxSize sz = GetSize();
    Fit();
    SetSize(sz.GetWidth(), GetSize().GetHeight());
    SetSizeHints(GetSize().GetWidth(), GetSize().GetHeight());
    FitInside();
    Layout();
}

bufsize SelectBlockDialog::GetOffset() const noexcept {
    bufsize o;
    if (!convertBaseFrom(o, text_->GetValue().ToStdString(), base_)) return 0;
    return o;
}

bufsize SelectBlockDialog::GetLength() const noexcept {
    bufsize o, o2;
    int negative;
    if (!convertBaseFrom(o, text_->GetValue().ToStdString(), base_)) return 0;
    if (!convertBaseFromNeg(o2, negative, text2_->GetValue().ToStdString(),
                            base_))
        return 0;
    return sub_ ? o2 - o : o2;
}

void SelectBlockDialog::EndDialog(int r) {
    SetReturnCode(r);
    if (IsModal()) EndModal(r);
}

void SelectBlockDialog::UpdateMetrics(bufsize cur, bufsize len, bufsize end) {
    cur_ = cur;
    len_ = len;
    end_ = end;
    okButton_->Enable(CheckInput());
}

bool SelectBlockDialog::CheckInput() {
    bufsize o, oe;
    if (!convertBaseFrom(o, text_->GetValue().ToStdString(), base_))
        return false;
    if (!convertBaseFrom(oe, text2_->GetValue().ToStdString(), base_))
        return false;
    if (sub_)
        return o <= oe && oe <= end_;
    else
        return o <= end_ && o + oe <= end_;
}

void SelectBlockDialog::ConvertBase(unsigned base) {
    bufsize o;
    int neg;
    bool ok = convertBaseFrom(o, text_->GetValue().ToStdString(), base_);
    if (ok) text_->SetValue(convertBaseTo(o, base, config().uppercase));
    bool ok2 =
        convertBaseFromNeg(o, neg, text2_->GetValue().ToStdString(), base_);
    if (ok2)
        text2_->SetValue(convertBaseToNeg(o, neg, base, config().uppercase));
    base_ = base;
}

void SelectBlockDialog::ConvertNewSub(bool sub) {
    bufsize o;
    if (sub_ == sub) return;
    bool ok = convertBaseFrom(o, text_->GetValue().ToStdString(), base_);
    if (ok) {
        bufsize oe;
        int neg;
        ok = convertBaseFromNeg(oe, neg, text2_->GetValue().ToStdString(),
                                base_);
        if (sub) {
            // length -> end
            if (neg < 0) {
                if (oe > o)
                    text2_->SetValue(
                        convertBaseToNeg(oe - o, 1, base_, config().uppercase));
                else
                    text2_->SetValue(
                        convertBaseTo(o - oe, base_, config().uppercase));
            } else {
                text2_->SetValue(
                    convertBaseTo(o + oe, base_, config().uppercase));
            }
        } else {
            // end -> length
            if (neg < 0) {
                text2_->SetValue(
                    convertBaseToNeg(o + oe, 1, base_, config().uppercase));
            } else {
                if (oe < o)
                    text2_->SetValue(
                        convertBaseToNeg(o - oe, 1, base_, config().uppercase));
                else
                    text2_->SetValue(
                        convertBaseTo(oe - o, base_, config().uppercase));
            }
        }
    }
    sub_ = sub;
}

void SelectBlockDialog::OnOK(wxCommandEvent& event) {
    if (!CheckInput()) return;
    EndDialog(wxID_OK);
}

void SelectBlockDialog::OnCancel(wxCommandEvent& event) {
    EndDialog(wxID_CANCEL);
}

void SelectBlockDialog::OnEnterFirst(wxCommandEvent& event) {
    text2_->SetFocus();
}

void SelectBlockDialog::OnTextInput(wxCommandEvent& event) {
    okButton_->Enable(CheckInput());
}

void SelectBlockDialog::OnOffsetChange(wxCommandEvent& event) {
    ConvertNewSub(off1_->GetValue());
    okButton_->Enable(CheckInput());
}

void SelectBlockDialog::OnRadixChange(wxCommandEvent& event) {
    ConvertBase(radix_->GetRadix());
    okButton_->Enable(CheckInput());
}

};  // namespace ui

};  // namespace hexbed
