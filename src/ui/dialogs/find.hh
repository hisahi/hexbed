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
// ui/dialog/find.hh -- header for the Find dialog

#ifndef HEXBED_UI_DIALOG_FIND_HH
#define HEXBED_UI_DIALOG_FIND_HH

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/notebook.h>
#include <wx/radiobut.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "common/types.hh"
#include "file/document.hh"
#include "ui/editor-fwd.hh"
#include "ui/hexbed-fwd.hh"
#include "ui/saeditor.hh"
#include "ui/textinput.hh"

namespace hexbed {

namespace ui {

wxDECLARE_EVENT(FIND_DOCUMENT_EDIT_EVENT, wxCommandEvent);

class FindDocumentControl : public wxPanel {
  public:
    FindDocumentControl(wxWindow* parent, HexBedContextMain* context,
                        std::shared_ptr<HexBedDocument> document, bool isFind);
    bool DoValidate();
    void Unregister();
    void ForwardEvent(wxCommandEvent& event);
    void ForwardBookEvent(wxBookCtrlEvent& event);
    void UpdateConfig();

    bool NonEmpty() const noexcept;

  private:
    HexBedContextMain* context_;
    std::shared_ptr<HexBedDocument> document_;
    wxNotebook* notebook_;
    HexBedStandaloneEditor* editor_;
    HexBedTextInput* textInput_;
};

struct FindDialogNoPrepare {};

class FindDialog : public wxDialog {
  public:
    FindDialog(HexBedMainFrame* parent, HexBedContextMain* context,
               std::shared_ptr<HexBedDocument> document);
    FindDialog(HexBedMainFrame* parent, HexBedContextMain* context,
               std::shared_ptr<HexBedDocument> document, FindDialogNoPrepare);

    inline virtual bool IsReplace() const noexcept { return false; }

    virtual void Unregister();
    virtual bool Recommit();
    virtual void UpdateConfig();
    virtual inline void AllowReplace(bool flag) {}

    static SearchResult findNext(HexEditorParent* ed);
    static SearchResult findPrevious(HexEditorParent* ed);

  protected:
    void OnCancel(wxCommandEvent& event);

    void OnFindNext(wxCommandEvent& event);
    void OnFindPrevious(wxCommandEvent& event);

    wxButton* findNextButton_;
    wxButton* findPrevButton_;

    bool CheckInput();

    HexBedMainFrame* parent_;
    HexBedContextMain* context_;
    std::shared_ptr<HexBedDocument> document_;

    FindDocumentControl* control_;

    bool dirty_{false};

  private:
    void OnChangedInput(wxCommandEvent& event);
};

};  // namespace ui

};  // namespace hexbed

#endif /* HEXBED_UI_DIALOG_FIND_HH */
