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
// ui/valueinput.cc -- impl for the value input class

#include "ui/valueinput.hh"

#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/valtext.h>

#include <bit>

#include "app/config.hh"
#include "common/logger.hh"
#include "file/document.hh"
#include "ui/plugins/inspector.hh"
#include "ui/settings/validate.hh"

namespace hexbed {
namespace ui {

HexBedValueInput::HexBedValueInput(wxWindow* parent, wxString* string,
                                   std::size_t* type)
    : wxPanel(parent, wxID_ANY), pType_(type), pText_(string) {
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(sizer);
    textCtrl_ = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                               wxDefaultSize, 0,
                               wxTextValidator(wxFILTER_NONE, string));

    sizer->Add(new wxStaticText(this, wxID_ANY, _("Value"), wxDefaultPosition,
                                wxDefaultSize, wxST_ELLIPSIZE_END),
               wxSizerFlags().Expand());
    sizer->Add(textCtrl_, wxSizerFlags().Expand());

    choice_ = new wxChoice(
        this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, 0,
        ChoiceValidator<std::size_t>(type, std::vector<std::size_t>{}));

    wxStaticText* label =
        new wxStaticText(this, wxID_ANY, _("Encoding"), wxDefaultPosition,
                         wxDefaultSize, wxST_ELLIPSIZE_END);
    wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
    row->Add(label, wxSizerFlags().Center().Proportion(1));
    row->AddStretchSpacer();
    row->Add(choice_);
    sizer->Add(row, wxSizerFlags().Expand());

    littleEndian_ =
        new wxCheckBox(this, wxID_ANY, _("Little endian"), wxDefaultPosition,
                       wxDefaultSize, wxCHK_2STATE);
    littleEndian_->SetValue(std::endian::native == std::endian::little);
    sizer->Add(littleEndian_, wxSizerFlags().Expand());

    textCtrl_->Bind(wxEVT_TEXT, &HexBedValueInput::ForwardEvent, this);
    Layout();
    UpdateConfig();
}

void HexBedValueInput::UpdateConfig() {
    ChoiceValidator<std::size_t>* validate =
        wxDynamicCast(choice_->GetValidator(), ChoiceValidator<std::size_t>);
    validate->TruncateItems(0);
    for (std::size_t i = 0, e = hexbed::plugins::dataInspectorPluginCount();
         i < e; ++i) {
        hexbed::plugins::DataInspectorPlugin& plugin =
            hexbed::plugins::dataInspectorPluginByIndex(i);
        if (!plugin.isReadOnly()) validate->AddItem(i, plugin.getTitle());
    }
    TransferDataToWindow();
}

void HexBedValueInput::ForwardEvent(wxCommandEvent& event) {
    AddPendingEvent(event);
}

bool HexBedValueInput::Commit(HexBedDocument* document) {
    if (!Validate() || !TransferDataFromWindow()) return false;
    hexbed::plugins::DataInspectorPlugin& plugin =
        hexbed::plugins::dataInspectorPluginByIndex(*pType_);
    hexbed::plugins::DataInspectorSettings settings;
    settings.littleEndian = littleEndian_->GetValue();
    std::size_t bs = plugin.getRequestedDataBufferSize();
    auto buf = std::make_unique<byte[]>(bs);
    bool ok = plugin.convertToBytes(bs, buf.get(), pText_->c_str(), settings);
    if (ok) {
        if (document)
            document->replace(0, document->size(),
                              const_bytespan{buf.get(), bs});
    } else {
        wxMessageBox(_("The entered value cannot be represented with the "
                       "specified data type."),
                     "HexBed", wxOK | wxICON_ERROR);
    }
    return ok;
}

bool HexBedValueInput::NonEmpty() const noexcept {
    return !textCtrl_->IsEmpty();
}

};  // namespace ui
};  // namespace hexbed
