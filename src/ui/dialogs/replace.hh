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
// ui/dialog/replace.hh -- header for the Replace dialog

#ifndef HEXBED_UI_DIALOG_REPLACE_HH
#define HEXBED_UI_DIALOG_REPLACE_HH

#include "ui/dialogs/find.hh"

namespace hexbed {

namespace ui {

class ReplaceDialog : public FindDialog {
  public:
    ReplaceDialog(HexBedMainFrame* parent, HexBedContextMain* context,
                  std::shared_ptr<HexBedDocument> document,
                  std::shared_ptr<HexBedDocument> repdoc);

    inline bool IsReplace() const noexcept { return true; }

    void Unregister();
    void Recommit();

    static void replaceSelection(HexEditorParent* ed);
    static bufsize replaceAll(HexEditorParent* ed);

  private:
    void OnFindNext(wxCommandEvent& event);
    void OnFindPrevious(wxCommandEvent& event);
    void OnReplaceNext(wxCommandEvent& event);
    void OnReplacePrevious(wxCommandEvent& event);
    void OnReplaceAll(wxCommandEvent& event);

    void OnChangedInput(wxCommandEvent& event);
    void OnChangedReplaceInput(wxCommandEvent& event);
    void OnChangedSelection(wxCommandEvent& event);

    std::shared_ptr<HexBedDocument> repdoc_;
    FindDocumentControl* replace_;

    wxButton* replaceNextButton_;
    wxButton* replacePrevButton_;
    wxButton* replaceAllButton_;

    bool dirtyReplace_{false};
};

};  // namespace ui

};  // namespace hexbed

#endif /* HEXBED_UI_DIALOG_REPLACE_HH */
