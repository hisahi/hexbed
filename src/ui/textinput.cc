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
// ui/textinput.cc -- impl for the text input class

#include "ui/textinput.hh"

#include <wx/checkbox.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/valtext.h>

#include "app/config.hh"
#include "common/logger.hh"
#include "file/document.hh"
#include "ui/encoding.hh"
#include "ui/settings/validate.hh"

namespace hexbed {
namespace ui {

static std::initializer_list<std::string> mbcsKeys_{MBCS_ENCODING_KEYS()};
static std::initializer_list<wxString> mbcsNames_{MBCS_ENCODING_NAMES()};
static std::initializer_list<std::string> sbcsKeys_{SBCS_ENCODING_KEYS()};
static std::initializer_list<wxString> sbcsNames_{SBCS_ENCODING_NAMES()};

HexBedTextInput::HexBedTextInput(wxWindow* parent, wxString* string,
                                 std::string* encoding, bool* caseInsensitive)
    : wxPanel(parent, wxID_ANY), pEncoding_(encoding), pText_(string) {
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(sizer);
    textCtrl_ = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                               wxDefaultSize, 0,
                               wxTextValidator(wxFILTER_NONE, string));

    sizer->Add(new wxStaticText(this, wxID_ANY, _("Text"), wxDefaultPosition,
                                wxDefaultSize, wxST_ELLIPSIZE_END),
               wxSizerFlags().Expand());
    sizer->Add(textCtrl_, wxSizerFlags().Expand());

    wxString choices = "Configured character encoding ()";

    choice_ = new wxChoice(
        this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 1, &choices, 0,
        ChoiceValidator<std::string>(encoding, std::vector<std::string>{""}));
    ChoiceValidator<std::string>* validate =
        wxDynamicCast(choice_->GetValidator(), ChoiceValidator<std::string>);

    HEXBED_ASSERT(mbcsKeys_.size() == mbcsNames_.size());
    HEXBED_ASSERT(sbcsKeys_.size() == sbcsNames_.size());

    for (std::size_t i = 0; i < mbcsKeys_.size(); ++i)
        validate->AddItem(mbcsKeys_.begin()[i], mbcsNames_.begin()[i]);
    for (std::size_t i = 0; i < sbcsKeys_.size(); ++i)
        validate->AddItem(sbcsKeys_.begin()[i], sbcsNames_.begin()[i]);

    wxStaticText* label =
        new wxStaticText(this, wxID_ANY, _("Encoding"), wxDefaultPosition,
                         wxDefaultSize, wxST_ELLIPSIZE_END);
    wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
    row->Add(label, wxSizerFlags().Center().Proportion(1));
    row->AddStretchSpacer();
    row->Add(choice_);
    sizer->Add(row, wxSizerFlags().Expand());

    if (caseInsensitive) {
        auto disable = [](wxCheckBox* control) -> wxCheckBox* {
            control->Enable(false);
            return control;
        };
        sizer->Add(disable(new wxCheckBox(this, wxID_ANY, _("Case insensitive"),
                                          wxDefaultPosition, wxDefaultSize,
                                          wxCHK_2STATE,
                                          wxGenericValidator(caseInsensitive))),
                   wxSizerFlags().Expand());
    }

    textCtrl_->Bind(wxEVT_TEXT, &HexBedTextInput::ForwardEvent, this);
    Layout();
    UpdateConfig();
    TransferDataToWindow();
}

void HexBedTextInput::UpdateConfig() {
    std::size_t n = sbcsKeys_.size();
    std::size_t i = 0;
    const std::string& encoding = config().charset;
    for (i = 0; i < n; ++i) {
        if (encoding == sbcsKeys_.begin()[i]) break;
    }
    if (i < n)
        choice_->SetString(
            0, wxString::Format(_("Configured character encoding (%s)"),
                                sbcsNames_.begin()[i]));
}

void HexBedTextInput::ForwardEvent(wxCommandEvent& event) {
    AddPendingEvent(event);
}

bool HexBedTextInput::Commit(HexBedDocument* document) {
    if (!Validate() || !TransferDataFromWindow()) return false;
    bufsize bs;
    bool ok = textEncode(*pEncoding_, *pText_, bs, document);
    if (!ok) {
        wxMessageBox(_("The entered text cannot be represented in the selected "
                       "encoding."),
                     "HexBed", wxOK | wxICON_ERROR);
    }
    return ok;
}

bool HexBedTextInput::NonEmpty() const noexcept {
    return !textCtrl_->IsEmpty();
}

};  // namespace ui
};  // namespace hexbed
