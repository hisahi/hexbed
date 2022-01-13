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

#include "app/config.hh"
#include "file/cisearch.hh"
#include "ui/hexbed.hh"

namespace hexbed {

namespace ui {

wxDEFINE_EVENT(FIND_DOCUMENT_EDIT_EVENT, wxCommandEvent);

FindDocumentControl::FindDocumentControl(
    wxWindow* parent, std::shared_ptr<HexBedContextMain> context,
    std::shared_ptr<HexBedDocument> document, bool isFind)
    : wxPanel(parent), context_(context), document_(document) {
    wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);
    notebook_ = new wxNotebook(this, wxID_ANY);
    decltype(document) copy = document;
    editor_ =
        new HexBedStandaloneEditor(notebook_, context.get(), std::move(copy));
    textInput_ = new HexBedTextInput(
        notebook_,
        isFind ? &context->state.searchFindTextString
               : &context->state.searchReplaceTextString,
        isFind ? &context->state.searchFindTextEncoding
               : &context->state.searchReplaceTextEncoding,
        isFind ? &context->state.searchFindTextCaseInsensitive : nullptr);
    valueInput_ =
        new HexBedValueInput(notebook_,
                             isFind ? &context->state.searchFindDataValue
                                    : &context->state.searchReplaceDataValue,
                             isFind ? &context->state.searchFindDataType
                                    : &context->state.searchReplaceDataType);
    notebook_->AddPage(editor_, _("Hex data"), true);
    notebook_->AddPage(textInput_, _("Text"), false);
    notebook_->AddPage(valueInput_, _("Data value"), false);
    notebook_->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED,
                    &FindDocumentControl::ForwardBookEvent, this);
    editor_->Bind(HEX_EDIT_EVENT, &FindDocumentControl::ForwardEvent, this);
    textInput_->Bind(wxEVT_TEXT, &FindDocumentControl::ForwardEvent, this);
    valueInput_->Bind(wxEVT_TEXT, &FindDocumentControl::ForwardEvent, this);
    top->Add(notebook_, wxSizerFlags().Expand().Proportion(1));
    registration_ = HexBedEditorRegistration(context, editor_);
    SetSizer(top);
}

bool FindDocumentControl::DoValidate() {
    switch (notebook_->GetSelection()) {
    case 0:
        break;
    case 1:
        return textInput_->Commit(document_.get());
    case 2:
        return valueInput_->Commit(document_.get());
    }
    return true;
}

bool FindDocumentControl::NonEmpty() const noexcept {
    switch (notebook_->GetSelection()) {
    case 0:
        return document_->size() > 0;
    case 1:
        return textInput_->NonEmpty();
    case 2:
        return valueInput_->NonEmpty();
    }
    return false;
}

bool FindDocumentControl::IsFindingText() const noexcept {
    return notebook_->GetSelection() == 1;
}

void FindDocumentControl::ForwardEvent(wxCommandEvent& event) {
    AddPendingEvent(wxCommandEvent(FIND_DOCUMENT_EDIT_EVENT));
}

void FindDocumentControl::ForwardBookEvent(wxBookCtrlEvent& event) {
    AddPendingEvent(wxCommandEvent(FIND_DOCUMENT_EDIT_EVENT));
}

void FindDocumentControl::UpdateConfig() {
    textInput_->UpdateConfig();
    valueInput_->UpdateConfig();
}

FindDialog::FindDialog(HexBedMainFrame* parent,
                       std::shared_ptr<HexBedContextMain> context,
                       std::shared_ptr<HexBedDocument> document,
                       FindDialogNoPrepare)
    : wxDialog(parent, wxID_ANY, _("Find"), wxDefaultPosition, wxSize(300, 200),
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      parent_(parent),
      context_(context),
      document_(document) {}

FindDialog::FindDialog(HexBedMainFrame* parent,
                       std::shared_ptr<HexBedContextMain> context,
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
    control_->Bind(FIND_DOCUMENT_EDIT_EVENT, &FindDialog::OnChangedInput, this);

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

bool FindDialog::Recommit() {
    if (!control_->DoValidate()) return false;
    context_->state.searchFindText = control_->IsFindingText();
    if (dirty_) {
        dirty_ = false;
        bufsize n = document_->size();
        byte* b = context_->getSearchBuffer(n);
        document_->read(0, bytespan{b, n});
        if (context_->state.searchFindText &&
            context_->state.searchFindTextCaseInsensitive)
            context_->state.searchCaseInsensitive = CaseInsensitivePattern{
                !context_->state.searchFindTextEncoding.empty()
                    ? context_->state.searchFindTextEncoding
                    : config().charset,
                context_->state.searchFindTextString.ToStdWstring()};
    }
    return true;
}

bool FindDialog::CheckInput() { return control_->NonEmpty(); }

void FindDialog::UpdateConfig() { control_->UpdateConfig(); }

void FindDialog::OnFindNext(wxCommandEvent& event) {
    if (!Recommit()) return;
    parent_->DoFindNext();
}

void FindDialog::OnFindPrevious(wxCommandEvent& event) {
    if (!Recommit()) return;
    parent_->DoFindPrevious();
}

void FindDialog::OnCancel(wxCommandEvent& event) { Close(); }

void FindDialog::OnChangedInput(wxCommandEvent& event) {
    bool flag = CheckInput();
    dirty_ = true;
    findNextButton_->Enable(flag);
    findPrevButton_->Enable(flag);
}

static SearchResult findNextCaseInsensitive(HexBedTask& task,
                                            const HexBedDocument& document,
                                            HexBedContextMain& context,
                                            bufsize dn, bufsize sel,
                                            bool wrapAround) {
    CaseInsensitivePattern& pattern = context.state.searchCaseInsensitive;
    SearchResult res = searchForwardCaseless(task, document, sel, dn, pattern);
    if (!task.isCancelled() && !res && wrapAround)
        res = searchForwardCaseless(task, document, 0, sel, pattern);
    return res;
}

static SearchResult findPrevCaseInsensitive(HexBedTask& task,
                                            const HexBedDocument& document,
                                            HexBedContextMain& context,
                                            bufsize dn, bufsize sel,
                                            bool wrapAround) {
    SearchResult res;
    bufsize osel = sel;
    CaseInsensitivePattern& pattern = context.state.searchCaseInsensitive;
    for (;;) {
        res = searchBackwardCaseless(task, document, 0, sel, pattern);
        if (task.isCancelled() || !res || res.offset + res.length <= osel)
            break;
        sel = res.offset;
    }
    if (!task.isCancelled() && !res && wrapAround)
        res = searchBackwardCaseless(task, document, sel + 1, dn, pattern);
    return res;
}

SearchResult FindDialog::findNext(HexEditorParent* ed) {
    HexBedContextMain& context = ed->context();
    bufsize sel, seln;
    bool seltext;
    ed->GetSelection(sel, seln, seltext);
    bufsize cur = sel + seln;
    const_bytespan search = context.getSearchString();
    bufsize dn = search.size();
    SearchResult res{};
    if (dn) {
        HexBedDocument& doc = ed->document();
        bufsize sn = doc.size();
        HexBedTask task(&context, 0, true);
        task.run([&res, &context, &doc, sn, dn, cur, search](HexBedTask& task) {
            if (context.state.searchFindText &&
                context.state.searchFindTextCaseInsensitive)
                res = findNextCaseInsensitive(task, doc, context, sn, cur,
                                              context.state.searchWrapAround);
            else if (sn - cur >= dn)
                res = doc.searchForwardFull(
                    task, cur, context.state.searchWrapAround, search);
            else if (context.state.searchWrapAround)
                res = doc.searchForwardFull(task, 0, false, search);
        });
        if (task.isCancelled()) return SearchResult{};
        if (res)
            ed->SelectBytes(res.offset, res.length,
                            SelectFlags().caretAtEnd().highlightBeginning());
    }
    return res;
}

SearchResult FindDialog::findPrevious(HexEditorParent* ed) {
    HexBedContextMain& context = ed->context();
    bufsize sel, seln;
    bool seltext;
    ed->GetSelection(sel, seln, seltext);
    bufsize cur = sel;
    const_bytespan search = context.getSearchString();
    bufsize dn = search.size();
    SearchResult res{};
    if (dn) {
        HexBedDocument& doc = ed->document();
        bufsize sn = doc.size();
        HexBedTask task(&context, 0, true);
        task.run([&res, &context, &doc, sn, dn, cur, search](HexBedTask& task) {
            if (context.state.searchFindText &&
                context.state.searchFindTextCaseInsensitive)
                res = findPrevCaseInsensitive(task, doc, context, sn, cur,
                                              context.state.searchWrapAround);
            else if (cur >= dn)
                res = doc.searchBackwardFull(
                    task, cur - dn, context.state.searchWrapAround, search);
            else if (context.state.searchWrapAround)
                res = doc.searchBackwardFull(task, sn - 1, false, search);
        });
        if (task.isCancelled()) return SearchResult{};
        if (res)
            ed->SelectBytes(res.offset, res.length,
                            SelectFlags().caretAtEnd().highlightBeginning());
    }
    return res;
}

};  // namespace ui

};  // namespace hexbed
