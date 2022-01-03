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
// ui/hexbed.hh -- header for the main program UI

#ifndef HEXBED_UI_HEXBED_HH
#define HEXBED_UI_HEXBED_HH

#include <wx/aui/auibook.h>
#include <wx/event.h>
#include <wx/frame.h>
#include <wx/menu.h>
#include <wx/statusbr.h>
#include <wx/toolbar.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "file/document-fwd.hh"
#include "ui/context.hh"
#include "ui/dialogs/find.hh"
#include "ui/editor-fwd.hh"

namespace hexbed {

namespace ui {

class HexBedMainFrame : public wxFrame {
  public:
    HexBedMainFrame();

    void FileKnock(const std::string& s, bool readOnly);
    void ApplyConfig();

    void UpdateMenuEnabled(hexbed::ui::HexEditorParent& editor);
    void UpdateMenuEnabledUndo(hexbed::ui::HexEditorParent& editor);
    void UpdateMenuEnabledSelect(hexbed::ui::HexEditorParent& editor);
    void UpdateMenuEnabledClip(hexbed::ui::HexEditorParent& editor);

    hexbed::ui::HexEditorParent* GetCurrentEditor();
    bool DoFindNext();
    bool DoFindPrevious();
    void OnReplaceDone(bufsize count);

  private:
    void OnClose(wxCloseEvent& event);
    void OnTabSwitch(wxAuiNotebookEvent& event);
    void OnTabClose(wxAuiNotebookEvent& event);
    void OnFindClose(wxCloseEvent& event);

    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    void OnFileNew(wxCommandEvent& event);
    void OnFileOpen(wxCommandEvent& event);
    void OnFileSave(wxCommandEvent& event);
    void OnFileSaveAs(wxCommandEvent& event);
    void OnFileClose(wxCommandEvent& event);
    void OnFileSaveAll(wxCommandEvent& event);
    void OnFileCloseAll(wxCommandEvent& event);
    void OnFileReload(wxCommandEvent& event);

    void OnEditMenuOpened(wxMenuEvent& event);
    void OnEditUndo(wxCommandEvent& event);
    void OnEditRedo(wxCommandEvent& event);
    void OnEditCut(wxCommandEvent& event);
    void OnEditCopy(wxCommandEvent& event);
    void OnEditPasteInsert(wxCommandEvent& event);
    void OnEditPasteReplace(wxCommandEvent& event);
    void OnEditDelete(wxCommandEvent& event);
    void OnEditSelectAll(wxCommandEvent& event);
    void OnEditSelectBlock(wxCommandEvent& event);
    void OnEditInsertToggle(wxCommandEvent& event);
    void OnEditPrefs(wxCommandEvent& event);

    void OnSearchFind(wxCommandEvent& event);
    void OnSearchFindNext(wxCommandEvent& event);
    void OnSearchFindPrevious(wxCommandEvent& event);
    void OnSearchReplace(wxCommandEvent& event);
    void OnSearchGoTo(wxCommandEvent& event);

    void OnViewColumnsBoth(wxCommandEvent& event);
    void OnViewColumnsHex(wxCommandEvent& event);
    void OnViewColumnsText(wxCommandEvent& event);

    void OnSelectChange(wxCommandEvent& event);

    hexbed::ui::HexBedEditor* GetEditor();
    hexbed::ui::HexBedEditor* GetEditor(size_t i);
    template <typename... Ts>
    std::unique_ptr<hexbed::ui::HexBedEditor> MakeEditor(Ts&&... args);
    void AddTab(std::unique_ptr<hexbed::ui::HexBedEditor>&& editor,
                const wxString& fn, const wxString& path);

    template <bool cut>
    void DoCopy();
    template <bool insert>
    void DoPaste();

    void FileReload(size_t i);
    bool FileSave(size_t i, bool saveAs);
    bool FileClose(size_t i);
    bool FileCloseAll();
    void UpdateFileOnly();
    void InitMenuEnabled();

    static void NoMoreResults();

    wxDECLARE_EVENT_TABLE();

    std::unique_ptr<HexBedContextMain> context_;
    wxAuiNotebook* tabs_;
    wxStatusBar* sbar_;
    std::uintmax_t newFileIndex_{0};
    std::vector<wxMenuItem*> fileOnlyMenuItems_;
    std::vector<wxToolBarToolBase*> fileOnlyToolItems_;
    std::shared_ptr<HexBedDocument> searchDocument_;
    std::shared_ptr<HexBedDocument> replaceDocument_;
    std::unique_ptr<FindDialog> findDialog_;
};

};  // namespace ui

}  // namespace hexbed

#endif /* HEXBED_UI_HEXBED_HH */
