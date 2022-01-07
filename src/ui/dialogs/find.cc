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
// ui/dialog/find.cc -- impl for the Find dialog

#include "ui/dialogs/find.hh"

#include <wx/event.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/valgen.h>

#include "ui/hexbed.hh"

namespace hexbed {

namespace ui {

FindDocumentControl::FindDocumentControl(
    wxWindow* parent, HexBedContextMain* context,
    std::shared_ptr<HexBedDocument> document, bool isFind)
    : wxPanel(parent), context_(context), document_(document) {
    wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);
    notebook_ = new wxNotebook(this, wxID_ANY);
    decltype(document) copy = document;
    editor_ = new HexBedStandaloneEditor(notebook_, context, std::move(copy));
    notebook_->AddPage(editor_, _("Hex data"), true);
    editor_->Bind(HEX_EDIT_EVENT, &FindDocumentControl::ForwardEvent, this);
    top->Add(notebook_, wxSizerFlags().Expand().Proportion(1));
    SetSizer(top);
    context->addWindow(editor_);
}

void FindDocumentControl::Unregister() { context_->removeWindow(editor_); }

void FindDocumentControl::ForwardEvent(wxCommandEvent& event) {
    AddPendingEvent(event);
}

FindDialog::FindDialog(HexBedMainFrame* parent, HexBedContextMain* context,
                       std::shared_ptr<HexBedDocument> document,
                       FindDialogNoPrepare)
    : wxDialog(parent, wxID_ANY, _("Find"), wxDefaultPosition, wxSize(300, 200),
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      parent_(parent),
      context_(context),
      document_(document) {}

FindDialog::FindDialog(HexBedMainFrame* parent, HexBedContextMain* context,
                       std::shared_ptr<HexBedDocument> document)
    : FindDialog(parent, context, document, FindDialogNoPrepare{}) {
    wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* buttons = new wxBoxSizer(wxHORIZONTAL);
    SetSize(wxSize(400, 300));

    findNextButton_ = new wxButton(this, wxID_ANY, _("Find &next"));
    findNextButton_->Bind(wxEVT_BUTTON, &FindDialog::OnFindNext, this);

    findPrevButton_ = new wxButton(this, wxID_ANY, _("Find &previous"));
    findPrevButton_->Bind(wxEVT_BUTTON, &FindDialog::OnFindPrevious, this);

    wxButton* cancelButton = new wxButton(this, wxID_CANCEL);
    cancelButton->Bind(wxEVT_BUTTON, &FindDialog::OnCancel, this);

    buttons->Add(1, 1, wxSizerFlags().Expand().Proportion(1));
    buttons->Add(findPrevButton_);
    buttons->Add(findNextButton_);
    buttons->Add(cancelButton);

    control_ = new FindDocumentControl(this, context, document, true);
    control_->Bind(HEX_EDIT_EVENT, &FindDialog::OnChangedInput, this);

    bool flag = CheckInput();
    findNextButton_->Enable(flag);
    findPrevButton_->Enable(flag);

    top->Add(new wxStaticText(this, wxID_ANY, _("Find data")),
             wxSizerFlags().Expand());
    top->Add(control_, wxSizerFlags().Expand().Proportion(1));
    top->Add(
        new wxCheckBox(this, wxID_ANY, _("&Wrap around"), wxDefaultPosition,
                       wxDefaultSize, 0,
                       wxGenericValidator(&context->state.searchWrapAround)),
        wxSizerFlags().Expand());
    top->Add(buttons, wxSizerFlags().Expand());

    SetSizer(top);
    Layout();
    SetMinSize(GetSize());
}

void FindDialog::Recommit() {
    if (dirty_) {
        dirty_ = false;
        bufsize n = document_->size();
        byte* b = context_->getSearchBuffer(n);
        document_->read(0, bytespan{b, n});
    }
}

void FindDialog::Unregister() { control_->Unregister(); }

bool FindDialog::CheckInput() { return document_->size() > 0; }

void FindDialog::OnFindNext(wxCommandEvent& event) {
    Recommit();
    parent_->DoFindNext();
}

void FindDialog::OnFindPrevious(wxCommandEvent& event) {
    Recommit();
    parent_->DoFindPrevious();
}

void FindDialog::OnCancel(wxCommandEvent& event) { Close(); }

void FindDialog::OnChangedInput(wxCommandEvent& event) {
    bool flag = CheckInput();
    dirty_ = true;
    findNextButton_->Enable(flag);
    findPrevButton_->Enable(flag);
}

SearchResult FindDialog::findNext(HexEditorParent* ed) {
    const HexBedContextMain& context = ed->context();
    bufsize sel, seln;
    bool seltext;
    ed->GetSelection(sel, seln, seltext);
    const_bytespan search = context.getSearchString();
    bufsize dn = search.size();
    SearchResult res{};
    if (dn) {
        HexBedDocument& doc = ed->document();
        if (sel < dn)
            res = doc.searchForwardFull(sel, context.state.searchWrapAround,
                                        search);
        else if (context.state.searchWrapAround)
            res = doc.searchForwardFull(0, false, search);
        if (res)
            ed->SelectBytes(res.offset, dn,
                            SelectFlags().caretAtEnd().highlightBeginning());
    }
    return res;
}

SearchResult FindDialog::findPrevious(HexEditorParent* ed) {
    const HexBedContextMain& context = ed->context();
    bufsize sel, seln;
    bool seltext;
    ed->GetSelection(sel, seln, seltext);
    const_bytespan search = context.getSearchString();
    bufsize dn = search.size();
    SearchResult res{};
    if (dn) {
        HexBedDocument& doc = ed->document();
        if (sel > 0)
            res = doc.searchBackwardFull(sel, context.state.searchWrapAround,
                                         search);
        else if (context.state.searchWrapAround)
            res = doc.searchBackwardFull(dn - 1, false, search);
        if (res)
            ed->SelectBytes(res.offset, dn,
                            SelectFlags().caretAtEnd().highlightBeginning());
    }
    return res;
}

};  // namespace ui

};  // namespace hexbed
