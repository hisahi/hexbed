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
// ui/hexbed.cc -- implementation for the main program UI

#include "ui/hexbed.hh"

#include <wx/aboutdlg.h>
#include <wx/arrstr.h>
#include <wx/wx.h>

#include <chrono>
#include <filesystem>
#include <thread>

#include "app/config.hh"
#include "common/logger.hh"
#include "common/version.hh"
#include "file/document.hh"
#include "file/task.hh"
#include "ui/applock.hh"
#include "ui/clipboard.hh"
#include "ui/dialogs/find.hh"
#include "ui/dialogs/goto.hh"
#include "ui/dialogs/replace.hh"
#include "ui/dialogs/selectblock.hh"
#include "ui/editor.hh"
#include "ui/hexbed.hh"
#include "ui/hexedit.hh"
#include "ui/logger.hh"
#include "ui/menus.hh"
#include "ui/settings.hh"

#define PLURAL wxPLURAL

namespace hexbed {

static AppLock* lock;

namespace ui {

static constexpr int tabContainerID = 0x80;

class HexBedWxApp : public wxApp {
  public:
    virtual bool OnInit();
    virtual int OnExit();

  private:
    HexBedMainFrame* window_;
    void Knock(const std::string& s);
    std::vector<std::string> openFiles;
};

// clang-format off
wxBEGIN_EVENT_TABLE(HexBedMainFrame, wxFrame)
    EVT_MENU(wxID_NEW, HexBedMainFrame::OnFileNew)
    EVT_MENU(wxID_OPEN, HexBedMainFrame::OnFileOpen)
    EVT_MENU(wxID_SAVE, HexBedMainFrame::OnFileSave)
    EVT_MENU(wxID_SAVEAS, HexBedMainFrame::OnFileSaveAs)
    EVT_MENU(wxID_CLOSE, HexBedMainFrame::OnFileClose)
    EVT_MENU(hexbed::menu::MenuFile_SaveAll, HexBedMainFrame::OnFileSaveAll)
    EVT_MENU(hexbed::menu::MenuFile_CloseAll, HexBedMainFrame::OnFileCloseAll)
    EVT_MENU(hexbed::menu::MenuFile_Reload, HexBedMainFrame::OnFileReload)

    EVT_MENU(wxID_UNDO, HexBedMainFrame::OnEditUndo)
    EVT_MENU(wxID_REDO, HexBedMainFrame::OnEditRedo)
    EVT_MENU(wxID_CUT, HexBedMainFrame::OnEditCut)
    EVT_MENU(wxID_COPY, HexBedMainFrame::OnEditCopy)
    EVT_MENU(wxID_PASTE, HexBedMainFrame::OnEditPasteInsert)
    EVT_MENU(hexbed::menu::MenuEdit_PasteReplace,
             HexBedMainFrame::OnEditPasteReplace)
    EVT_MENU(hexbed::menu::MenuEdit_InsertMode,
             HexBedMainFrame::OnEditInsertToggle)
    EVT_MENU(wxID_DELETE, HexBedMainFrame::OnEditDelete)
    EVT_MENU(wxID_SELECTALL, HexBedMainFrame::OnEditSelectAll)
    EVT_MENU(hexbed::menu::MenuEdit_SelectRange,
             HexBedMainFrame::OnEditSelectBlock)
    EVT_MENU(wxID_PREFERENCES, HexBedMainFrame::OnEditPrefs)

    EVT_MENU(wxID_FIND, HexBedMainFrame::OnSearchFind)
    EVT_MENU(hexbed::menu::MenuSearch_FindNext,
             HexBedMainFrame::OnSearchFindNext)
    EVT_MENU(hexbed::menu::MenuSearch_FindPrevious,
             HexBedMainFrame::OnSearchFindPrevious)
    EVT_MENU(wxID_REPLACE, HexBedMainFrame::OnSearchReplace)
    EVT_MENU(hexbed::menu::MenuSearch_GoTo, HexBedMainFrame::OnSearchGoTo)

    EVT_MENU(hexbed::menu::MenuView_ShowColumnsBoth,
             HexBedMainFrame::OnViewColumnsBoth)
    EVT_MENU(hexbed::menu::MenuView_ShowColumnsHex,
             HexBedMainFrame::OnViewColumnsHex)
    EVT_MENU(hexbed::menu::MenuView_ShowColumnsText,
             HexBedMainFrame::OnViewColumnsText)
    EVT_MENU(hexbed::menu::MenuView_BitEditor,
             HexBedMainFrame::OnViewBitEditor)

    EVT_MENU(wxID_EXIT, HexBedMainFrame::OnExit)
    EVT_MENU(wxID_ABOUT, HexBedMainFrame::OnAbout)

    EVT_AUINOTEBOOK_PAGE_CHANGED(tabContainerID, HexBedMainFrame::OnTabSwitch)
    EVT_AUINOTEBOOK_PAGE_CLOSE(tabContainerID, HexBedMainFrame::OnTabClose)
    EVT_CLOSE(HexBedMainFrame::OnClose)
wxEND_EVENT_TABLE()

void HexBedWxApp::Knock(const std::string& s) {
    LOG_DEBUG("got knock <%s>", s);
    if (window_) {
        if (!s.empty()) window_->FileKnock(s, false);
    } else {
        openFiles.push_back(s);
    }
}

bool HexBedWxApp::OnInit() {
    // clang-format on
    auto fn = argc > 1 ? argv[1].ToStdString() : "";
    setlocale(LC_ALL, "C");
    setlocale(LC_COLLATE, "");
    setlocale(LC_TIME, "");
#ifdef LC_MESSAGES
    setlocale(LC_MESSAGES, "");
#endif
    logger_ok = true;
#if NDEBUG
    LOG_ADD_HANDLER(StdLogHandler, LogLevel::WARN);
#else
    LOG_ADD_HANDLER(StdLogHandler, LogLevel::TRACE);
#endif
    lock = new AppLock([this](const std::string& pass) { this->Knock(pass); });
    if (!lock->acquire(fn)) {
        LOG_DEBUG("lock taken; knocking existing impl");
        return false;
    }
    currentConfig.load();
    wxTranslations* trans = new wxTranslations();
    trans->SetLanguage(config().language);
#ifdef LC_MESSAGES
    if (!config().language.empty())
        setlocale(LC_MESSAGES, config().language.c_str());
#endif
    if (!fn.empty()) openFiles.push_back(fn);
    if (!trans->AddStdCatalog())
        LOG_WARN("translation: could not load the wxstd catalog");
    if (!trans->AddCatalog("hexbed"))
        LOG_WARN("translation: could not load the hexbed catalog");
    wxTranslations::Set(trans);
    window_ = new HexBedMainFrame();
    window_->Show(true);
    for (const std::string& s : openFiles) window_->FileKnock(s, false);
    openFiles.clear();
    return true;
}

int HexBedWxApp::OnExit() {
    if (lock) {
        lock->release();
        delete lock;
    }
    return 0;
}

HexBedMainFrame::HexBedMainFrame()
    : wxFrame(NULL, wxID_ANY, "HexBed", wxDefaultPosition, wxSize(-1, -1)) {
    tabs_ = new wxAuiNotebook(this, tabContainerID);
    context_ = std::make_shared<HexBedContextMain>(this);
    wxMenuBar* menuBar = new wxMenuBar;
    hexbed::menu::createFileMenu(menuBar, fileOnlyMenuItems_);
    hexbed::menu::createEditMenu(menuBar, fileOnlyMenuItems_)
        ->Bind(wxEVT_MENU_OPEN, &HexBedMainFrame::OnEditMenuOpened, this);
    hexbed::menu::createSearchMenu(menuBar, fileOnlyMenuItems_);
    hexbed::menu::createViewMenu(menuBar, fileOnlyMenuItems_);
    hexbed::menu::createHelpMenu(menuBar, fileOnlyMenuItems_);
    wxToolBar* toolBar = CreateToolBar(wxTB_HORIZONTAL | wxTB_FLAT);
    hexbed::menu::populateToolBar(toolBar, fileOnlyToolItems_);
    toolBar->Show(true);
    SetMenuBar(menuBar);
    sbar_ = CreateStatusBar(4);
    if (sbar_) hexbed::menu::populateStatusBar(sbar_);
    tabs_->Layout();
    UpdateFileOnly();
    ApplyConfig();
    InitMenuEnabled();
    InitPreferences(this);
    searchDocument_ = std::make_shared<HexBedDocument>(context_);
    replaceDocument_ = std::make_shared<HexBedDocument>(context_);
    menuBar->Check(hexbed::menu::MenuEdit_InsertMode, context_->state.insert);
}

hexbed::ui::HexBedEditor* HexBedMainFrame::GetEditor() {
    int sel = tabs_->GetSelection();
    if (sel == wxNOT_FOUND) return nullptr;
    return static_cast<hexbed::ui::HexBedEditor*>(tabs_->GetPage(sel));
}

hexbed::ui::HexBedEditor* HexBedMainFrame::GetEditor(size_t i) {
    return static_cast<hexbed::ui::HexBedEditor*>(tabs_->GetPage(i));
}

bool HexBedMainFrame::FileSave(size_t i, bool saveAs) {
    if (i < tabs_->GetPageCount()) {
        hexbed::ui::HexBedEditor* editor = GetEditor(i);
        std::string sfn;
        HexBedDocument& document = editor->document();
        saveAs = saveAs || !document.filed();
        if (!saveAs && !document.unsaved()) return true;
        std::string pathdir = "";
        std::string pathfn = "";

        if (document.filed()) {
            try {
                std::filesystem::path p(document.path());
                pathdir = p.parent_path();
                pathfn = p.filename();
            } catch (...) {
            }
        }
        if (saveAs) {
            wxFileDialog dial(this, _("Save a file"), pathdir, pathfn,
                              _("All files (*.*)") + "|*",
                              wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
            if (dial.ShowModal() == wxID_CANCEL) return false;
            sfn = dial.GetPath();
        }

        try {
            if (saveAs) {
                document.commitAs(sfn);
                try {
                    sfn = std::filesystem::canonical(sfn).string();
                } catch (...) {
                }
                tabs_->SetPageToolTip(i, sfn);
                try {
                    tabs_->SetPageText(
                        i, static_cast<std::string>(
                               std::filesystem::path(sfn).filename()));
                } catch (...) {
                }
            } else
                document.commit();
            editor->ReloadFile();
        } catch (...) {
            try {
                wxMessageBox(wxString::Format(
                                 _("Failed to save file %s: %s"),
                                 saveAs ? sfn.c_str() : document.path().c_str(),
                                 currentExceptionAsString().c_str()),
                             "HexBed", wxOK | wxICON_ERROR);
            } catch (...) {
            }
            return false;
        }
    }
    return true;
}

void HexBedMainFrame::OnEditMenuOpened(wxMenuEvent& event) {
    if (tabs_->GetPageCount()) UpdateMenuEnabled(*GetEditor());
}

void HexBedMainFrame::FileReload(size_t i) {
    if (i < tabs_->GetPageCount()) {
        hexbed::ui::HexBedEditor* editor = GetEditor(i);
        if (!editor->document().filed()) return;
        if (editor->document().unsaved()) {
            tabs_->ChangeSelection(i);
            wxString txt = wxString::Format(
                _("'%s' has unsaved changes. Discard them and reload?"),
                tabs_->GetPageText(i));
            wxMessageDialog dial(this, txt, "HexBed",
                                 wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
            switch (dial.ShowModal()) {
            case wxID_YES:
                break;
            case wxID_NO:
                return;
            };
        }

        for (;;) {
            try {
                editor->document().discard();
                editor->ReloadFile();
                return;
            } catch (...) {
                try {
                    wxString txt =
                        wxString::Format(_("Failed to load from file %s:\n%s"),
                                         editor->document().path().c_str(),
                                         currentExceptionAsString().c_str());
                    wxMessageDialog dial(
                        this, txt, "HexBed",
                        wxYES_NO | wxNO_DEFAULT | wxICON_ERROR);
                    if (!dial.SetYesNoLabels(_("&Retry"), _("&Cancel")))
                        dial.SetMessage(dial.GetMessage() + "\n\n" +
                                        _("Retry?"));
                    if (dial.ShowModal() == wxID_NO) return;
                } catch (...) {
                    return;
                }
            }
        }
    }
}

void HexBedMainFrame::ApplyConfig() {
    currentConfig.apply();
    HexEditor::InitConfig();
    context_->updateWindows();
}

bool HexBedMainFrame::FileClose(size_t i) {
    if (i < tabs_->GetPageCount()) {
        hexbed::ui::HexBedEditor* editor = GetEditor(i);
        if (editor->document().unsaved()) {
            tabs_->ChangeSelection(i);
            wxString txt = wxString::Format(
                _("'%s' has unsaved changes. Do you want to save it?"),
                tabs_->GetPageText(i));
            wxMessageDialog dial(this, txt, "HexBed",
                                 wxYES_NO | wxCANCEL | wxICON_QUESTION);
            switch (dial.ShowModal()) {
            case wxID_YES:
                if (!FileSave(i, false)) return false;
            case wxID_NO:
                break;
            default:  // cancel
                return false;
            };
        }
    }
    return true;
}

bool HexBedMainFrame::FileCloseAll() {
    while (tabs_->GetPageCount())
        if (!FileClose(0))
            return false;
        else
            tabs_->DeletePage(0);
    UpdateFileOnly();
    return true;
}

void HexBedMainFrame::UpdateFileOnly() {
    bool haveFiles = tabs_->GetPageCount() > 0;
    for (wxMenuItem* p : fileOnlyMenuItems_) p->Enable(haveFiles);
    for (wxToolBarToolBase* p : fileOnlyToolItems_) p->Enable(haveFiles);
}

void HexBedMainFrame::OnTabSwitch(wxAuiNotebookEvent& event) {
    int s = tabs_->GetSelection();
    if (s == wxNOT_FOUND) {
        InitMenuEnabled();
        context_->announceCursorUpdate(HexBedPeekRegion{});
        return;
    }
    hexbed::ui::HexBedEditor* editor = GetEditor(s);
    editor->Selected();
    UpdateMenuEnabled(*editor);
    context_->announceCursorUpdate(editor->PeekBufferAtCursor());
    AddPendingEvent(wxCommandEvent(HEX_SELECT_EVENT));
}

void HexBedMainFrame::InitMenuEnabled() {
    wxMenuBar& mbar = *GetMenuBar();
    mbar.Enable(wxID_CUT, false);
    mbar.Enable(wxID_COPY, false);
    mbar.Enable(wxID_PASTE, false);
    mbar.Enable(hexbed::menu::MenuEdit_PasteReplace, false);
    mbar.Enable(wxID_DELETE, false);
    mbar.Enable(wxID_UNDO, false);
    mbar.Enable(wxID_REDO, false);
}

void HexBedMainFrame::UpdateMenuEnabledSelect(hexbed::ui::HexEditorParent& ed) {
    bufsize sel, seln;
    bool seltext;
    ed.GetSelection(sel, seln, seltext);
    wxMenuBar& mbar = *GetMenuBar();
    mbar.Enable(wxID_CUT, seln > 0);
    mbar.Enable(wxID_COPY, seln > 0);
    mbar.Enable(wxID_DELETE, seln > 0);
}

void HexBedMainFrame::OnUndoRedo(hexbed::ui::HexEditorParent& ed) {
    wxMenuBar& mbar = *GetMenuBar();
    mbar.Enable(wxID_UNDO, ed.document().canUndo());
    mbar.Enable(wxID_REDO, ed.document().canRedo());
}

void HexBedMainFrame::OnEditorCopy(hexbed::ui::HexEditorParent& ed) {
    wxMenuBar& mbar = *GetMenuBar();
    mbar.Enable(wxID_PASTE, hexbed::clip::HasClipboard());
    mbar.Enable(hexbed::menu::MenuEdit_PasteReplace,
                hexbed::clip::HasClipboard());
}

void HexBedMainFrame::UpdateMenuEnabled(hexbed::ui::HexEditorParent& ed) {
    UpdateMenuEnabledSelect(ed);
    OnEditorCopy(ed);
    OnUndoRedo(ed);
}

void HexBedMainFrame::OnTabClose(wxAuiNotebookEvent& event) {
    auto s = event.GetSelection();
    if (s == wxNOT_FOUND) return;
    if (!FileClose(s))
        event.Veto();
    else {
        if (tabs_->GetPageCount() <= 1) {
            InitMenuEnabled();
            hexbed::menu::updateStatusBarNoFile(GetStatusBar(),
                                                context_->state);
            context_->announceCursorUpdate(HexBedPeekRegion{});
        }
        context_->removeWindow(GetEditor(s));
        event.Allow();
        AddPendingEvent(wxCommandEvent(HEX_SELECT_EVENT));
    }
}

template <typename... Ts>
std::unique_ptr<hexbed::ui::HexBedEditor> HexBedMainFrame::MakeEditor(
    Ts&&... args) {
    return std::make_unique<hexbed::ui::HexBedEditor>(
        this, this, context_.get(),
        HexBedDocument(context_, std::forward<Ts>(args)...));
}

void HexBedMainFrame::AddTab(std::unique_ptr<hexbed::ui::HexBedEditor>&& editor,
                             const wxString& fn, const wxString& path) {
    auto i = tabs_->GetPageCount() ? tabs_->GetSelection() + 1 : 0;
    if (!tabs_->InsertPage(i, editor.get(), fn, true))
        throw std::runtime_error("could not add wxAuiNotebook page");
    if (sbar_) editor->SetStatusBar(sbar_);
    if (!path.IsEmpty()) tabs_->SetPageToolTip(i, path);
    editor->SetFocus();
    editor->FocusEditor();
    context_->addWindow(editor.get());
    editor.release();
    UpdateFileOnly();
}

void HexBedMainFrame::OnFileNew(wxCommandEvent& event) {
    try {
        AddTab(MakeEditor(), wxString::Format(_("New-%llu"), ++newFileIndex_),
               "");
    } catch (...) {
        try {
            wxMessageBox(wxString::Format(_("Failed to create a new file: %s"),
                                          currentExceptionAsString().c_str()),
                         "HexBed", wxOK | wxICON_ERROR);
        } catch (...) {
        }
    }
}

void HexBedMainFrame::FileKnock(const std::string& fp, bool readOnly) {
    try {
        auto editor = MakeEditor(fp, readOnly);
        std::string path = editor->document().path(), fn = path;
        try {
            path = std::filesystem::canonical(path).string();
        } catch (...) {
        }
        try {
            fn = std::filesystem::path(fn).filename();
        } catch (...) {
        }
        AddTab(std::move(editor), fn, path);
    } catch (...) {
        try {
            wxMessageBox(
                wxString::Format(_("Failed to open file %s: %s"), fp.c_str(),
                                 currentExceptionAsString().c_str()),
                "HexBed", wxOK | wxICON_ERROR);
        } catch (...) {
        }
    }
}

static wxWindow* MakeCheckBoxWindow(wxWindow* parent) {
    return new wxCheckBox(parent, wxID_ANY, _("Read only"));
}

void HexBedMainFrame::OnEditSelectAll(wxCommandEvent& event) {
    hexbed::ui::HexBedEditor* ed = GetEditor();
    ed->SelectBytes(0, ed->document().size(), SelectFlags().caretAtEnd());
}

void HexBedMainFrame::OnEditSelectBlock(wxCommandEvent& event) {
    hexbed::ui::HexBedEditor* ed = GetEditor();
    bufsize sel, seln;
    bool seltext;
    ed->GetSelection(sel, seln, seltext);
    SelectBlockDialog g(this, sel, seln, ed->document().size());
    if (g.ShowModal() == wxID_OK) {
        bufsize o = g.GetOffset();
        bufsize l = g.GetLength();
        ed->SelectBytes(o, l,
                        SelectFlags().caretAtBeginning().highlightCaret());
    }
}

void HexBedMainFrame::OnFindClose(wxCloseEvent& event) {
    findDialog_->Unregister();
    findDialog_->Destroy();
    findDialog_ = nullptr;
}

void HexBedMainFrame::OnBitEditorClose(wxCloseEvent& event) {
    bitEditorTool_->Destroy();
    bitEditorTool_ = nullptr;
}

void HexBedMainFrame::NoMoreResults() {
    wxMessageBox(_("No more results found for this search."), "HexBed",
                 wxOK | wxICON_INFORMATION);
}

void HexBedMainFrame::OnReplaceDone(bufsize count) {
    wxMessageBox(
        wxString::Format(PLURAL("Found and replaced %llu match.",
                                "Found and replaced %llu matches.", count),
                         count),
        "HexBed", wxOK | wxICON_INFORMATION);
}

hexbed::ui::HexEditorParent* HexBedMainFrame::GetCurrentEditor() {
    return GetEditor();
}

bool HexBedMainFrame::DoFindNext() {
    hexbed::ui::HexBedEditor* ed = GetEditor();
    if (findDialog_) findDialog_->Recommit();
    if (!hexbed::ui::FindDialog::findNext(ed)) {
        NoMoreResults();
        return false;
    } else {
        return true;
    }
}

bool HexBedMainFrame::DoFindPrevious() {
    hexbed::ui::HexBedEditor* ed = GetEditor();
    if (findDialog_) findDialog_->Recommit();
    if (!hexbed::ui::FindDialog::findPrevious(ed)) {
        NoMoreResults();
        return false;
    } else {
        return true;
    }
}

void HexBedMainFrame::OnCaretMoved(hexbed::ui::HexEditorParent& editor) {
    hexbed::ui::HexBedEditor* ed = GetEditor();
    if (!ed)
        context_->announceCursorUpdate(HexBedPeekRegion{});
    else if (ed == &editor)
        context_->announceCursorUpdate(editor.PeekBufferAtCursor());
}

void HexBedMainFrame::OnSelectChanged(hexbed::ui::HexEditorParent& editor) {
    hexbed::ui::HexBedEditor* ed = GetEditor();
    if (ed == &editor) UpdateMenuEnabledSelect(editor);
}

void HexBedMainFrame::OnSearchFind(wxCommandEvent& event) {
    if (!findDialog_ || findDialog_->IsReplace()) {
        if (findDialog_) findDialog_->Close(true);  // calls OnFindClose
        findDialog_ =
            std::make_unique<FindDialog>(this, context_.get(), searchDocument_);
        findDialog_->Bind(wxEVT_CLOSE_WINDOW, &HexBedMainFrame::OnFindClose,
                          this);
    }
    findDialog_->Show(true);
    findDialog_->SetFocus();
}

void HexBedMainFrame::OnSearchFindNext(wxCommandEvent& event) {
    if (!searchDocument_->size()) return OnSearchFind(event);
    DoFindNext();
}

void HexBedMainFrame::OnSearchFindPrevious(wxCommandEvent& event) {
    if (!searchDocument_->size()) return OnSearchFind(event);
    DoFindPrevious();
}

void HexBedMainFrame::OnSearchReplace(wxCommandEvent& event) {
    if (!findDialog_ || !findDialog_->IsReplace()) {
        if (findDialog_) {
            findDialog_->Close(true);
            // OnFindClose ...
        }
        findDialog_ = std::make_unique<ReplaceDialog>(
            this, context_.get(), searchDocument_, replaceDocument_);
        findDialog_->Bind(wxEVT_CLOSE_WINDOW, &HexBedMainFrame::OnFindClose,
                          this);
    }
    findDialog_->Show(true);
    findDialog_->Raise();
    findDialog_->SetFocus();
}

void HexBedMainFrame::OnSearchGoTo(wxCommandEvent& event) {
    hexbed::ui::HexBedEditor* ed = GetEditor();
    bufsize sel, seln;
    bool seltext;
    ed->GetSelection(sel, seln, seltext);
    GoToDialog g(this, sel, ed->document().size());
    if (g.ShowModal() == wxID_OK) {
        bufsize o = g.GetOffset();
        ed->SelectBytes(o, 0, SelectFlags());
    }
}

void HexBedMainFrame::OnViewColumnsBoth(wxCommandEvent& event) {
    currentConfig.values().showColumnTypes = 3;
    GetMenuBar()->Check(hexbed::menu::MenuView_ShowColumnsBoth, true);
    ApplyConfig();
}

void HexBedMainFrame::OnViewColumnsHex(wxCommandEvent& event) {
    currentConfig.values().showColumnTypes = 2;
    GetMenuBar()->Check(hexbed::menu::MenuView_ShowColumnsHex, true);
    ApplyConfig();
}

void HexBedMainFrame::OnViewColumnsText(wxCommandEvent& event) {
    currentConfig.values().showColumnTypes = 1;
    GetMenuBar()->Check(hexbed::menu::MenuView_ShowColumnsText, true);
    ApplyConfig();
}

void HexBedMainFrame::OnViewBitEditor(wxCommandEvent& event) {
    if (!bitEditorTool_) {
        bitEditorTool_ = std::make_unique<BitEditorTool>(this, context_);
        bitEditorTool_->Bind(wxEVT_CLOSE_WINDOW,
                             &HexBedMainFrame::OnBitEditorClose, this);
    }
    bitEditorTool_->Show(true);
    bitEditorTool_->Raise();
    bitEditorTool_->SetFocus();
}

void HexBedMainFrame::OnEditInsertToggle(wxCommandEvent& event) {
    EditorState& state = context_->state;
    state.insert = !state.insert;
    GetMenuBar()->Check(hexbed::menu::MenuEdit_InsertMode, state.insert);
    hexbed::ui::HexBedEditor* ed = GetEditor();
    if (ed)
        ed->UpdateStatusBar();
    else
        hexbed::menu::updateStatusBarNoFile(GetStatusBar(), state);
}

void HexBedMainFrame::OnFileOpen(wxCommandEvent& event) {
    wxFileDialog dial(this, _("Open a file"), "", "",
                      _("All files (*.*)") + "|*",
                      wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    dial.SetExtraControlCreator(&MakeCheckBoxWindow);
    if (dial.ShowModal() == wxID_CANCEL) return;
    bool readOnly = false;
    wxArrayString files;
    dial.GetPaths(files);
    wxWindow* check = dial.GetExtraControl();
    if (check) readOnly = dynamic_cast<wxCheckBox*>(check)->GetValue();
    for (const wxString& file : files) FileKnock(file.ToStdString(), readOnly);
}

void HexBedMainFrame::OnFileSave(wxCommandEvent& event) {
    if (tabs_->GetCurrentPage()) FileSave(tabs_->GetSelection(), false);
}

void HexBedMainFrame::OnFileSaveAs(wxCommandEvent& event) {
    if (tabs_->GetCurrentPage()) FileSave(tabs_->GetSelection(), true);
}

void HexBedMainFrame::OnFileClose(wxCommandEvent& event) {
    if (tabs_->GetCurrentPage()) {
        auto i = tabs_->GetSelection();
        if (FileClose(i)) {
            tabs_->DeletePage(i);
            UpdateFileOnly();
        }
    }
}

void HexBedMainFrame::OnFileSaveAll(wxCommandEvent& event) {
    for (std::size_t i = 0; i < tabs_->GetPageCount(); ++i)
        if (!FileSave(i, false)) break;
}

void HexBedMainFrame::OnFileReload(wxCommandEvent& event) {
    if (tabs_->GetCurrentPage()) FileReload(tabs_->GetSelection());
}

void HexBedMainFrame::OnFileCloseAll(wxCommandEvent& event) { FileCloseAll(); }

void HexBedMainFrame::OnEditUndo(wxCommandEvent& event) {
    hexbed::ui::HexBedEditor* ed = GetEditor();
    if (ed->document().canUndo()) {
        HexBedRange sel = ed->document().undo();
        ed->HintBytesChanged(0);
        OnUndoRedo(*ed);
        ed->SelectBytes(sel.offset, sel.length,
                        SelectFlags().caretAtEnd().highlightCaret());
    }
}

void HexBedMainFrame::OnEditRedo(wxCommandEvent& event) {
    hexbed::ui::HexBedEditor* ed = GetEditor();
    if (ed->document().canRedo()) {
        HexBedRange sel = ed->document().redo();
        ed->HintBytesChanged(0);
        OnUndoRedo(*ed);
        ed->SelectBytes(sel.offset, sel.length,
                        SelectFlags().caretAtEnd().highlightCaret());
    }
}

template <bool cut>
void HexBedMainFrame::DoCopy() {
    hexbed::ui::HexBedEditor* ed = GetEditor();
    bufsize sel, seln;
    bool seltext;
    ed->GetSelection(sel, seln, seltext);
    if (seln > 0) {
        try {
            hexbed::clip::CopyBytes(ed->document(), sel, seln, seltext);
        } catch (...) {
            wxMessageBox(wxString::Format(_("Failed to copy: %s"),
                                          currentExceptionAsString().c_str()),
                         "HexBed", wxOK | wxICON_ERROR);
            return;
        }
        if constexpr (cut) {
            ed->document().remove(sel, seln);
            ed->SelectBytes(sel, 0, SelectFlags().highlightCaret());
            ed->HintBytesChanged(sel);
        }
    }
}

void HexBedMainFrame::OnEditCut(wxCommandEvent& event) { DoCopy<true>(); }

void HexBedMainFrame::OnEditCopy(wxCommandEvent& event) { DoCopy<false>(); }

template <bool insert>
void HexBedMainFrame::DoPaste() {
    if (hexbed::clip::HasClipboard()) {
        hexbed::ui::HexBedEditor* ed = GetEditor();
        bufsize sel, seln;
        bool seltext;
        ed->GetSelection(sel, seln, seltext);
        try {
            bufsize len;
            if (!hexbed::clip::PasteBytes(ed->document(), insert, sel, seln,
                                          seltext, len)) {
                wxMessageBox(
                    seltext
                        ? _("Cannot paste the current clipboard contents, "
                            "because it contains characters that the currently "
                            "selected character encoding cannot represent.")
                        : _("Cannot paste the current clipboard contents, "
                            "because it does not contain valid hexadecimal "
                            "data. Text data should be a string of hexadecimal "
                            "bytes."),
                    "HexBed", wxOK | wxICON_ERROR);
            } else {
                ed->HintBytesChanged(sel);
                ed->SelectBytes(sel + len, 0,
                                SelectFlags().caretAtEnd().highlightCaret());
            }
        } catch (...) {
            wxMessageBox(wxString::Format(_("Failed to paste: %s"),
                                          currentExceptionAsString().c_str()),
                         "HexBed", wxOK | wxICON_ERROR);
        }
    }
}

void HexBedMainFrame::OnEditPasteInsert(wxCommandEvent& event) {
    DoPaste<true>();
}

void HexBedMainFrame::OnEditPasteReplace(wxCommandEvent& event) {
    DoPaste<false>();
}

void HexBedMainFrame::OnEditDelete(wxCommandEvent& event) {
    hexbed::ui::HexBedEditor* ed = GetEditor();
    bufsize sel, seln;
    bool seltext;
    ed->GetSelection(sel, seln, seltext);
    if (seln > 0) {
        ed->document().remove(sel, seln);
        ed->SelectBytes(sel, 0, SelectFlags().highlightCaret());
    }
}

void HexBedMainFrame::OnEditPrefs(wxCommandEvent& event) {
    ShowPreferences(this);
}

void HexBedMainFrame::OnExit(wxCommandEvent& event) {
    if (FileCloseAll()) Close(true);
}

void HexBedMainFrame::OnClose(wxCloseEvent& event) {
    if (event.CanVeto() && !FileCloseAll())
        event.Veto();
    else {
        currentConfig.save();
        event.Skip();
    }
}

void HexBedMainFrame::OnAbout(wxCommandEvent& event) {
    wxAboutDialogInfo info;
    info.SetName("HexBed");
    info.SetVersion(HEXBED_VER_STRING);
    info.SetDescription(_("A hex editor."));
    info.SetCopyright(wxT("(C) 2021-2022 Sampo 'hisahi' Hippeläinen"));
    wxAboutBox(info);
}

};  // namespace ui

};  // namespace hexbed

using hexbed::ui::HexBedWxApp;
// clang-format off
wxIMPLEMENT_APP(HexBedWxApp);
// clang-format on
