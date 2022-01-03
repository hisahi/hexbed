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
// ui/dialog/replace.cc -- impl for the Replace dialog

#include "ui/dialogs/replace.hh"

#include <wx/event.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/valgen.h>

#include <type_traits>

#include "ui/hexbed.hh"
#include "ui/settings/validate.hh"

namespace hexbed {

namespace ui {

ReplaceDialog::ReplaceDialog(HexBedMainFrame* parent,
                             HexBedContextMain* context,
                             std::shared_ptr<HexBedDocument> document,
                             std::shared_ptr<HexBedDocument> repdoc)
    : FindDialog(parent, context, document, FindDialogNoPrepare{}),
      repdoc_(repdoc) {
    SetTitle(_("Replace"));
    SetSize(wxSize(450, 300));
    SetMinSize(GetSize());
    SetSize(wxSize(600, 400));

    wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* buttons = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* rbuttons = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* rbuttons2 = new wxBoxSizer(wxHORIZONTAL);

    findNextButton_ = new wxButton(this, wxID_ANY, _("Find &next"));
    findNextButton_->Bind(wxEVT_BUTTON, &ReplaceDialog::OnFindNext, this);

    findPrevButton_ = new wxButton(this, wxID_ANY, _("Find &previous"));
    findPrevButton_->Bind(wxEVT_BUTTON, &ReplaceDialog::OnFindPrevious, this);

    wxButton* cancelButton = new wxButton(this, wxID_CANCEL);
    cancelButton->Bind(wxEVT_BUTTON, &ReplaceDialog::OnCancel, this);

    buttons->Add(1, 1, wxSizerFlags().Expand().Proportion(1));
    buttons->Add(findPrevButton_);
    buttons->Add(findNextButton_);
    buttons->Add(cancelButton);

    replaceNextButton_ = new wxButton(this, wxID_ANY, _("N&ext"));
    replaceNextButton_->Bind(wxEVT_BUTTON, &ReplaceDialog::OnReplaceNext, this);

    replacePrevButton_ = new wxButton(this, wxID_ANY, _("Pre&vious"));
    replacePrevButton_->Bind(wxEVT_BUTTON, &ReplaceDialog::OnReplacePrevious,
                             this);

    replaceAllButton_ = new wxButton(this, wxID_ANY, _("&All"));
    replaceAllButton_->Bind(wxEVT_BUTTON, &ReplaceDialog::OnReplaceAll, this);

    cancelButton = new wxButton(this, wxID_CANCEL);
    cancelButton->Bind(wxEVT_BUTTON, &ReplaceDialog::OnCancel, this);

    rbuttons->Add(1, 1, wxSizerFlags().Expand().Proportion(1));
    rbuttons->Add(new wxStaticText(this, wxID_ANY, _("Replace and find")),
                  wxSizerFlags().Border(wxLEFT | wxRIGHT, 5).Center());
    rbuttons->Add(replacePrevButton_);
    rbuttons->Add(replaceNextButton_);

    rbuttons2->Add(1, 1, wxSizerFlags().Expand().Proportion(1));
    rbuttons2->Add(new wxStaticText(this, wxID_ANY, _("Replace")),
                   wxSizerFlags().Border(wxLEFT | wxRIGHT, 5).Center());
    rbuttons2->Add(replaceAllButton_);
    rbuttons2->Add(cancelButton);

    replace_ = new FindDocumentControl(this, context, repdoc, false);
    replace_->Bind(HEX_EDIT_EVENT, &ReplaceDialog::OnChangedReplaceInput, this);

    control_ = new FindDocumentControl(this, context, document, true);
    control_->Bind(HEX_EDIT_EVENT, &ReplaceDialog::OnChangedInput, this);

    bool flag = CheckInput();
    findNextButton_->Enable(flag);
    findPrevButton_->Enable(flag);
    replaceNextButton_->Enable(flag);
    replacePrevButton_->Enable(flag);
    replaceAllButton_->Enable(flag);

    top->Add(new wxStaticText(this, wxID_ANY, _("Find data")),
             wxSizerFlags().Expand());
    top->Add(control_, wxSizerFlags().Expand().Proportion(1));
    top->Add(
        new wxCheckBox(this, wxID_ANY, _("&Wrap around (find only)"),
                       wxDefaultPosition, wxDefaultSize, 0,
                       wxGenericValidator(&context->state.searchWrapAround)),
        wxSizerFlags().Expand());
    top->Add(buttons, wxSizerFlags().Expand());
    top->Add(new wxStaticText(this, wxID_ANY, _("Find and replace")),
             wxSizerFlags().Expand());
    top->Add(replace_, wxSizerFlags().Expand().Proportion(1));
    top->Add(rbuttons, wxSizerFlags().Expand());
    top->Add(rbuttons2, wxSizerFlags().Expand());

    SetSizer(top);
    Layout();
}

void ReplaceDialog::Recommit() {
    FindDialog::Recommit();
    if (dirtyReplace_) {
        dirtyReplace_ = false;
        bufsize n = repdoc_->size();
        byte* b = context_->getReplaceBuffer(n);
        repdoc_->read(0, bytespan{b, n});
    }
}

void ReplaceDialog::Unregister() {
    replace_->Unregister();
    control_->Unregister();
}

void ReplaceDialog::OnFindNext(wxCommandEvent& event) {
    Recommit();
    parent_->DoFindNext();
}

void ReplaceDialog::OnFindPrevious(wxCommandEvent& event) {
    Recommit();
    parent_->DoFindPrevious();
}

void ReplaceDialog::OnReplaceNext(wxCommandEvent& event) {
    Recommit();
    HexEditorParent* ed = parent_->GetCurrentEditor();
    if (ed) {
        replaceSelection(ed);
        parent_->DoFindNext();
    }
}

void ReplaceDialog::OnReplacePrevious(wxCommandEvent& event) {
    Recommit();
    HexEditorParent* ed = parent_->GetCurrentEditor();
    if (ed) {
        replaceSelection(ed);
        parent_->DoFindPrevious();
    }
}

void ReplaceDialog::OnReplaceAll(wxCommandEvent& event) {
    Recommit();
    HexEditorParent* ed = parent_->GetCurrentEditor();
    if (ed) parent_->OnReplaceDone(replaceAll(ed));
}

void ReplaceDialog::OnChangedInput(wxCommandEvent& event) {
    bool flag = CheckInput();
    dirty_ = true;
    findNextButton_->Enable(flag);
    findPrevButton_->Enable(flag);
    replaceNextButton_->Enable(flag);
    replacePrevButton_->Enable(flag);
    replaceAllButton_->Enable(flag);
}

void ReplaceDialog::OnChangedReplaceInput(wxCommandEvent& event) {
    dirtyReplace_ = true;
}

void ReplaceDialog::replaceSelection(HexEditorParent* ed) {
    bufsize sel, seln;
    bool seltext;
    ed->GetSelection(sel, seln, seltext);
    if (ed->document().compareEqual(sel, seln,
                                    ed->context().getSearchString())) {
        const_bytespan replace = ed->context().getReplaceString();
        ed->document().replace(sel, seln, replace);
        ed->SelectBytes(sel + replace.size(), 0, SelectFlags());
    }
}

bufsize ReplaceDialog::replaceAll(HexEditorParent* ed) {
    bufsize sel, seln, repls = 0, cur, rat;
    bool seltext;
    ed->GetSelection(sel, seln, seltext);
    cur = sel;
    rat = 0;
    SearchResult res;
    const_bytespan search = ed->context().getSearchString();
    const_bytespan replace = ed->context().getReplaceString();
    bufsize sn = search.size(), rn = replace.size();
    UndoGroupToken ugt = ed->document().undoGroup();
    while ((res = ed->document().searchForwardFull(rat, false, search))) {
        bufsize so = res.offset;
        ++repls;
        if (so < cur && sn != rn) {
            cur = cur >= sn ? cur - sn : 0;
            cur += rn;
        }
        ed->document().replace(so, sn, replace);
        ugt.tick();
        rat = so + rn;
    }
    ed->SelectBytes(cur, 0, SelectFlags());
    ugt.commit();
    return repls;
}

};  // namespace ui

};  // namespace hexbed
