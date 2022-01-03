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
// ui/dialog/radixpicker.cc -- impl for the dialog radix picker

#include "ui/dialogs/radixpicker.hh"

#include <wx/sizer.h>
#include <wx/stattext.h>

#include "app/config.hh"

namespace hexbed {

namespace ui {

wxDEFINE_EVENT(hEVT_RADIXCHANGE, wxCommandEvent);

RadixPicker::RadixPicker(wxWindow* parent) : wxPanel(parent) {
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    SetSizer(sizer);

    sizer->Add(new wxStaticText(this, wxID_ANY, _("Radix")),
               wxSizerFlags().Proportion(1));
    sizer->Add(
        oct_ = new wxRadioButton(this, wxID_ANY, _("Oct"), wxDefaultPosition,
                                 wxDefaultSize, wxRB_GROUP),
        wxSizerFlags().Proportion(1));
    sizer->Add(dec_ = new wxRadioButton(this, wxID_ANY, _("Dec"),
                                        wxDefaultPosition, wxDefaultSize, 0),
               wxSizerFlags().Proportion(1));
    sizer->Add(hex_ = new wxRadioButton(this, wxID_ANY, _("Hex"),
                                        wxDefaultPosition, wxDefaultSize, 0),
               wxSizerFlags().Proportion(1));

    oct_->Bind(wxEVT_RADIOBUTTON, &RadixPicker::OnRadioButton, this);
    dec_->Bind(wxEVT_RADIOBUTTON, &RadixPicker::OnRadioButton, this);
    hex_->Bind(wxEVT_RADIOBUTTON, &RadixPicker::OnRadioButton, this);

    switch (config().offsetRadix) {
    case 8:
        oct_->SetValue(true);
        break;
    case 10:
        dec_->SetValue(true);
        break;
    case 16:
    default:
        hex_->SetValue(true);
        break;
    }

    Layout();
    UpdateValue();
}

unsigned RadixPicker::GetRadix() const noexcept { return value_; }

void RadixPicker::UpdateValue() {
    if (oct_->GetValue())
        value_ = 8;
    else if (dec_->GetValue())
        value_ = 10;
    else
        value_ = 16;
}

void RadixPicker::OnRadioButton(wxCommandEvent& event) {
    UpdateValue();
    AddPendingEvent(wxCommandEvent(hEVT_RADIXCHANGE));
}

};  // namespace ui

};  // namespace hexbed
