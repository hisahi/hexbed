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
// ui/tools/bitedit.cc -- impl for the bit editor tool

#include "ui/tools/bitedit.hh"

#include "app/config.hh"
#include "common/hexconv.hh"
#include "ui/hexbed.hh"

namespace hexbed {

namespace ui {

static constexpr byte bitMask(int i) { return static_cast<byte>(0x80U >> i); }

BitEditorTool::BitEditorTool(HexBedMainFrame* parent,
                             std::shared_ptr<HexBedContextMain> context)
    : wxDialog(parent, wxID_ANY, _("Bit editor"), wxDefaultPosition,
               wxDefaultSize, wxDEFAULT_DIALOG_STYLE),
      HexBedViewer(1) {
    wxGridSizer* sizer = new wxGridSizer(2, 10, 3, 3);
    for (int i = 0; i < 8; ++i) {
        sizer->Add(
            (bitChecks_[i] = new wxCheckBox(this, wxID_ANY, wxEmptyString)),
            wxSizerFlags().Center());
        bitChecks_[i]->Bind(wxEVT_CHECKBOX, [this, i](wxCommandEvent& event) {
            OnBitFlip(i, event.IsChecked());
        });
    }
    sizer->AddSpacer(1);
    sizer->AddSpacer(1);
    for (int i = 0; i < 8; ++i)
        sizer->Add((bitLabels_[i] = new wxStaticText(
                        this, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize)),
                   wxSizerFlags().Center());
    sizer->Add(new wxStaticText(this, wxID_ANY, "=", wxDefaultPosition,
                                wxDefaultSize));
    sizer->Add((byteLabel_ = new wxStaticText(
                    this, wxID_ANY, "00", wxDefaultPosition, wxDefaultSize)),
               wxSizerFlags().Center());
    const wxFont& orig = byteLabel_->GetFont();
    byteLabel_->SetFont(wxFont(orig.GetPointSize(), wxFONTFAMILY_TELETYPE,
                               orig.GetStyle(), orig.GetWeight()));
    byteLabel_->SetLabel("FF");
    SetSizer(sizer);
    Layout();
    Fit();
    wasAllowed_ = true;
    reg_ = HexBedViewerRegistration(context, this);
}

void BitEditorTool::onUpdateCursor(HexBedPeekRegion peek) {
    document_ = peek.document;
    offset_ = peek.offset;
    bool allow = document_ && !peek.data.empty();
    if (allow) {
        byte b = peek.data[0];
        buf_ = b;
        char buf[3] = "00";
        for (int i = 0; i < 8; ++i) {
            bool set = b & bitMask(i);
            bitChecks_[i]->Enable(true);
            bitChecks_[i]->SetValue(set);
            bitLabels_[i]->SetLabelText(set ? "1" : "0");
        }
        fastByteConv(buf, config().uppercase ? HEX_UPPERCASE : HEX_LOWERCASE,
                     b);
        byteLabel_->SetLabelText(buf);
    } else if (wasAllowed_) {
        for (int i = 0; i < 8; ++i) {
            bitChecks_[i]->Enable(false);
            bitChecks_[i]->SetValue(false);
            bitLabels_[i]->SetLabelText("-");
        }
        byteLabel_->SetLabelText("--");
    }
    wasAllowed_ = allow;
}

void BitEditorTool::OnBitFlip(int bit, bool newState) {
    if (document_) {
        bitLabels_[bit]->SetLabelText(newState ? "1" : "0");
        if (newState)
            buf_ |= bitMask(bit);
        else
            buf_ &= ~bitMask(bit);
        document_->impose(offset_, 1, buf_);
    }
}

};  // namespace ui

};  // namespace hexbed
