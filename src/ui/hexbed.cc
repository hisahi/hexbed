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

#include "app/bitop.hh"
#include "app/config.hh"
#include "common/buffer.hh"
#include "common/logger.hh"
#include "common/random.hh"
#include "common/version.hh"
#include "file/document.hh"
#include "file/task.hh"
#include "ui/applock.hh"
#include "ui/clipboard.hh"
#include "ui/dialogs/bitopbinary.hh"
#include "ui/dialogs/bitopshift.hh"
#include "ui/dialogs/bitopunary.hh"
#include "ui/dialogs/find.hh"
#include "ui/dialogs/goto.hh"
#include "ui/dialogs/insert.hh"
#include "ui/dialogs/random.hh"
#include "ui/dialogs/replace.hh"
#include "ui/dialogs/selectblock.hh"
#include "ui/editor.hh"
#include "ui/hexbed.hh"
#include "ui/hexedit.hh"
#include "ui/logger.hh"
#include "ui/menus.hh"
#include "ui/plugins/export.hh"
#include "ui/plugins/import.hh"
#include "ui/plugins/plugin.hh"
#include "ui/settings.hh"
#include "ui/string.hh"

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
    void Knock(const wxString& s);
    std::vector<wxString> openFiles;
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
    EVT_MENU(hexbed::menu::MenuEdit_InsertOrReplace,
             HexBedMainFrame::OnEditInsertOrReplace)
    EVT_MENU(hexbed::menu::MenuEdit_InsertRandom,
              HexBedMainFrame::OnEditInsertRandom)
    EVT_MENU(hexbed::menu::MenuEdit_BitwiseBinaryOp,
             HexBedMainFrame::OnEditBitwiseBinaryOp)
    EVT_MENU(hexbed::menu::MenuEdit_BitwiseUnaryOp,
             HexBedMainFrame::OnEditBitwiseUnaryOp)
    EVT_MENU(hexbed::menu::MenuEdit_BitwiseShiftOp,
             HexBedMainFrame::OnEditBitwiseShiftOp)
    EVT_MENU(hexbed::menu::MenuEdit_ByteSwap2,
             HexBedMainFrame::OnEditByteSwap2)
    EVT_MENU(hexbed::menu::MenuEdit_ByteSwap4,
             HexBedMainFrame::OnEditByteSwap4)
    EVT_MENU(hexbed::menu::MenuEdit_ByteSwap8,
             HexBedMainFrame::OnEditByteSwap8)
    EVT_MENU(hexbed::menu::MenuEdit_ByteSwap16,
             HexBedMainFrame::OnEditByteSwap16)
    EVT_MENU(hexbed::menu::MenuEdit_Reverse, HexBedMainFrame::OnEditReverse)
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
    EVT_MENU(hexbed::menu::MenuView_DataInspector,
             HexBedMainFrame::OnViewDataInspector)

    EVT_MENU(wxID_EXIT, HexBedMainFrame::OnExit)
    EVT_MENU(wxID_ABOUT, HexBedMainFrame::OnAbout)

    EVT_AUINOTEBOOK_PAGE_CHANGED(tabContainerID, HexBedMainFrame::OnTabSwitch)
    EVT_AUINOTEBOOK_PAGE_CLOSE(tabContainerID, HexBedMainFrame::OnTabClose)
    EVT_CLOSE(HexBedMainFrame::OnClose)
wxEND_EVENT_TABLE()

template <typename T>
const strchar* getCharPtr(const T&);

// clang-format on

static wxString pathToWxString(const std::filesystem::path& p) {
    if constexpr (std::is_same_v<strchar, wchar_t>)
        return wxString(p.native());
    else
#if defined(__unix__) || defined(__unix)
        return wxString::FromUTF8(
            reinterpret_cast<const char*>(p.u8string().data()));
#else
        return wxString::FromUTF8(p.native());
#endif
}

template <>
const strchar* getCharPtr<strchar*>(strchar* const& ptr) {
    return ptr;
}

template <>
const strchar* getCharPtr<wxCharTypeBuffer<strchar>>(
    const wxCharTypeBuffer<strchar>& ptr) {
    return ptr.data();
}

template <>
const strchar* getCharPtr<wxScopedCharTypeBuffer<strchar>>(
    const wxScopedCharTypeBuffer<strchar>& ptr) {
    return ptr.data();
}

static std::filesystem::path pathFromWxString(const wxString& s) {
    return std::filesystem::path(getCharPtr(s.fn_str()));
}

void HexBedWxApp::Knock(const wxString& s) {
    LOG_DEBUG("got knock <%" FMT_STR ">", stringFromWx(s));
    if (window_) {
        if (!s.empty()) window_->FileKnock(s, false);
    } else {
        openFiles.push_back(s);
    }
}

bool HexBedWxApp::OnInit() {
    auto fn = argc > 1 ? stringFromWx(argv[1]) : string();
    setlocale(LC_ALL, "C");
    setlocale(LC_COLLATE, "");
    setlocale(LC_TIME, "");
#ifdef LC_MESSAGES
    setlocale(LC_MESSAGES, "");
#endif
    logger_ok = true;
    hexbed::plugins::executableDirectory =
        std::filesystem::weakly_canonical(pathFromWxString(argv[0]))
            .parent_path();
#if NDEBUG
    LOG_ADD_HANDLER(StdLogHandler, LogLevel::WARN);
#else
    LOG_ADD_HANDLER(StdLogHandler, LogLevel::TRACE);
#endif
    lock = new AppLock([this](const wxString& pass) { this->Knock(pass); });
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
    hexbed::plugins::loadBuiltinPlugins();
    hexbed::plugins::loadExternalPlugins();
    window_ = new HexBedMainFrame();
    window_->Show(true);
    for (const wxString& s : openFiles) window_->FileKnock(s, false);
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

static int addImportPlugins(HexBedMainFrame* main, wxMenu* menu, int n) {
    if (!n) return wxID_NONE;
    int id = wxWindow::NewControlId(n);
    if (id == wxID_NONE) return wxID_NONE;
    for (int i = 0; i < n; ++i) {
        hexbed::plugins::ImportPlugin& plugin =
            hexbed::plugins::importPluginByIndex(i);
        menu->Append(id + i, plugin.isLocalizable()
                                 ? wxGetTranslation(plugin.getTitle())
                                 : plugin.getTitle());
    }
    return id;
}

static int addExportPlugins(HexBedMainFrame* main, wxMenu* menu, int n) {
    if (!n) return wxID_NONE;
    int id = wxWindow::NewControlId(n);
    if (id == wxID_NONE) return wxID_NONE;
    for (int i = 0; i < n; ++i) {
        hexbed::plugins::ExportPlugin& plugin =
            hexbed::plugins::exportPluginByIndex(i);
        menu->Append(id + i, plugin.isLocalizable()
                                 ? wxGetTranslation(plugin.getTitle())
                                 : plugin.getTitle());
        menu->Enable(id + i, false);
    }
    return id;
}

HexBedMainFrame::HexBedMainFrame()
    : wxFrame(NULL, wxID_ANY, "HexBed", wxDefaultPosition, wxSize(-1, -1)) {
    tabs_ = new wxAuiNotebook(this, tabContainerID);
    context_ = std::make_shared<HexBedContextMain>(this);
    wxMenuBar* menuBar = new wxMenuBar;
    hexbed::menu::createFileMenu(menuBar, fileOnlyMenuItems_, fileMenus_);
    hexbed::menu::createEditMenu(menuBar, fileOnlyMenuItems_)
        ->Bind(wxEVT_MENU_OPEN, &HexBedMainFrame::OnEditMenuOpened, this);
    hexbed::menu::createSearchMenu(menuBar, fileOnlyMenuItems_);
    hexbed::menu::createViewMenu(menuBar, fileOnlyMenuItems_);
    hexbed::menu::createHelpMenu(menuBar, fileOnlyMenuItems_);
    wxToolBar* toolBar = CreateToolBar(wxTB_HORIZONTAL | wxTB_FLAT);
    hexbed::menu::populateToolBar(toolBar, fileOnlyToolItems_);
    toolBar->Show(true);
    SetMenuBar(menuBar);
    if (fileMenus_.importMenu) {
        int n = static_cast<int>(
            std::min<size_t>(std::numeric_limits<int>::max(),
                             hexbed::plugins::importPluginCount()));
        if (n) {
            int id = addImportPlugins(this, fileMenus_.importMenu, n);
            menuBar->Bind(wxEVT_MENU, &HexBedMainFrame::OnFileMenuImport, this,
                          id, id + n - 1);
            fileMenus_.importPluginCount = n;
            fileMenus_.firstImportId = id;
        }
    }
    if (fileMenus_.exportMenu) {
        int n = static_cast<int>(
            std::min<size_t>(std::numeric_limits<int>::max(),
                             hexbed::plugins::exportPluginCount()));
        if (n) {
            int id = addExportPlugins(this, fileMenus_.exportMenu, n);
            menuBar->Bind(wxEVT_MENU, &HexBedMainFrame::OnFileMenuExport, this,
                          id, id + n - 1);
            fileMenus_.exportPluginCount = n;
            fileMenus_.firstExportId = id;
        }
    }
    sbar_ = CreateStatusBar(4);
    if (sbar_) hexbed::menu::populateStatusBar(sbar_);
    tabs_->Layout();
    UpdateFileOnly();
    ApplyConfig();
    InitMenuEnabled();
    InitPreferences(this);
    searchDocument_ = std::make_shared<HexBedDocument>(context_);
    replaceDocument_ = std::make_shared<HexBedDocument>(context_);
    insertDocument_ = std::make_shared<HexBedDocument>(context_);
    binaryOpDocument_ = std::make_shared<HexBedDocument>(context_);
    menuBar->Check(hexbed::menu::MenuEdit_InsertMode, context_->state.insert);
}

hexbed::ui::HexBedEditor* HexBedMainFrame::GetEditor() {
    int sel = tabs_->GetSelection();
    if (sel == wxNOT_FOUND) return nullptr;
    return static_cast<hexbed::ui::HexBedEditor*>(tabs_->GetPage(sel));
}

hexbed::ui::HexBedEditor* HexBedMainFrame::GetEditor(std::size_t i) {
    return static_cast<hexbed::ui::HexBedEditor*>(tabs_->GetPage(i));
}

void HexBedMainFrame::UpdateTabSymbol(std::size_t i) {
    hexbed::ui::HexBedEditor& editor = *GetEditor(i);
    HexBedDocument& document = editor.document();
    bool unsaved = document.unsaved();
    if (!editor.DidUnsavedChange(unsaved)) return;
    if (unsaved) {
        tabs_->SetPageToolTip(
            i, wxString::Format(_("[unsaved] %s"),
                                pathToWxString(document.path())));
        const wxString& label = tabs_->GetPageText(i);
        if (label[0] != '*') tabs_->SetPageText(i, "*" + label);
    } else {
        tabs_->SetPageToolTip(i, pathToWxString(document.path()));
        const wxString& label = tabs_->GetPageText(i);
        if (label[0] == '*') tabs_->SetPageText(i, label.Mid(1));
    }
    /// main window title bar
    SetTitle(wxString::Format(_("%s - %s"), tabs_->GetPageText(i), "HexBed"));
}

bool HexBedMainFrame::FileSave(std::size_t i, bool saveAs) {
    if (i < tabs_->GetPageCount()) {
        hexbed::ui::HexBedEditor* editor = GetEditor(i);
        std::filesystem::path sfn;
        HexBedDocument& document = editor->document();
        saveAs = saveAs || !document.filed();
        if (!saveAs && !document.unsaved()) return true;

        if (saveAs) {
            std::filesystem::path pathdir = document.path().parent_path();
            std::filesystem::path pathfn = document.path().filename();
            if (document.filed()) {
                try {
                    pathdir = document.path().parent_path();
                    pathfn = document.path().filename();
                } catch (...) {
                }
            }
            wxFileDialog dial(this, _("Save a file"), pathToWxString(pathdir),
                              pathToWxString(pathfn),
                              _("All files (*.*)") + "|*",
                              wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
            if (dial.ShowModal() == wxID_CANCEL) return false;
            sfn = pathFromWxString(dial.GetPath());
        }

        try {
            if (saveAs) {
                document.commitAs(sfn);
                try {
                    sfn = std::filesystem::canonical(sfn);
                } catch (...) {
                }
                tabs_->SetPageToolTip(i, pathToWxString(sfn));
                try {
                    tabs_->SetPageText(i, pathToWxString(sfn.filename()));
                } catch (...) {
                }
            } else
                document.commit();
            editor->ReloadFile();
        } catch (...) {
            try {
                wxMessageBox(wxString::Format(
                                 _("Failed to save file %s: %s"),
                                 pathToWxString(saveAs ? sfn : document.path()),
                                 currentExceptionAsString()),
                             "HexBed", wxOK | wxICON_ERROR);
            } catch (...) {
            }
            return false;
        }
        UpdateTabSymbol(i);
    }
    return true;
}

void HexBedMainFrame::OnEditMenuOpened(wxMenuEvent& event) {
    if (tabs_->GetPageCount()) {
        hexbed::ui::HexBedEditor& ed = *GetEditor();
        OnEditorCopy(ed);
        OnUndoRedo(ed);
    }
}

void HexBedMainFrame::FileReload(std::size_t i) {
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
                break;
            } catch (...) {
                try {
                    wxString txt = wxString::Format(
                        _("Failed to load from file %s:\n%s"),
                        pathToWxString(editor->document().path()),
                        currentExceptionAsString());
                    wxMessageDialog dial(
                        this, txt, "HexBed",
                        wxYES_NO | wxNO_DEFAULT | wxICON_ERROR);
                    if (!dial.SetYesNoLabels(_("&Retry"), _("&Cancel")))
                        dial.SetMessage(dial.GetMessage() + "\n\n" +
                                        _("Retry?"));
                    if (dial.ShowModal() == wxID_NO) break;
                } catch (...) {
                    break;
                }
            }
        }
        UpdateTabSymbol(i);
    }
}

void HexBedMainFrame::ApplyConfig() {
    currentConfig.apply();
    HexEditor::InitConfig();
    context_->updateWindows();
    if (findDialog_) findDialog_->UpdateConfig();
}

bool HexBedMainFrame::FileClose(std::size_t i) {
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
    wxMenuBar& mbar = *GetMenuBar();
    bool haveFiles = tabs_->GetPageCount() > 0;
    for (wxMenuItem* p : fileOnlyMenuItems_) p->Enable(haveFiles);
    for (wxToolBarToolBase* p : fileOnlyToolItems_) p->Enable(haveFiles);
    for (int i = 0; i < fileMenus_.exportPluginCount; ++i) {
        mbar.Enable(fileMenus_.firstExportId + i, haveFiles);
    }
}

void HexBedMainFrame::OnTabSwitch(wxAuiNotebookEvent& event) {
    int s = tabs_->GetSelection();
    if (s == wxNOT_FOUND) {
        SetTitle("HexBed");
        InitMenuEnabled();
        context_->announceCursorUpdate(HexBedPeekRegion{});
        return;
    }
    /// main window title bar
    SetTitle(wxString::Format(_("%s - %s"), tabs_->GetPageText(s), "HexBed"));
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
    bool seltext, selhas, selwr;
    ed.GetSelection(sel, seln, seltext);
    wxMenuBar& mbar = *GetMenuBar();
    selhas = seln > 0;
    selwr = selhas && !ed.document().readOnly();
    mbar.Enable(wxID_CUT, selwr);
    mbar.Enable(wxID_COPY, selhas);
    mbar.Enable(wxID_DELETE, selwr);
    mbar.Enable(hexbed::menu::MenuEdit_BitwiseBinaryOp, selwr);
    mbar.Enable(hexbed::menu::MenuEdit_BitwiseUnaryOp, selwr);
    mbar.Enable(hexbed::menu::MenuEdit_BitwiseShiftOp, selwr);
    mbar.Enable(hexbed::menu::MenuEdit_ByteSwap2, selwr && !(seln & 1));
    mbar.Enable(hexbed::menu::MenuEdit_ByteSwap4, selwr && !(seln & 3));
    mbar.Enable(hexbed::menu::MenuEdit_ByteSwap8, selwr && !(seln & 7));
    mbar.Enable(hexbed::menu::MenuEdit_ByteSwap16, selwr && !(seln & 15));
    mbar.Enable(hexbed::menu::MenuEdit_Reverse, selwr);
}

void HexBedMainFrame::OnUndoRedo(hexbed::ui::HexEditorParent& ed) {
    wxMenuBar& mbar = *GetMenuBar();
    mbar.Enable(wxID_UNDO, ed.document().canUndo());
    mbar.Enable(wxID_REDO, ed.document().canRedo());
}

void HexBedMainFrame::OnEditorCopy(hexbed::ui::HexEditorParent& ed) {
    wxMenuBar& mbar = *GetMenuBar();
    bool hasClip = !ed.document().readOnly() && hexbed::clip::HasClipboard();
    mbar.Enable(wxID_PASTE, hasClip);
    mbar.Enable(hexbed::menu::MenuEdit_PasteReplace, hasClip);
}

void HexBedMainFrame::UpdateMenuEnabled(hexbed::ui::HexEditorParent& ed) {
    wxMenuBar& mbar = *GetMenuBar();
    bool writable = !ed.document().readOnly();
    mbar.Enable(hexbed::menu::MenuEdit_InsertOrReplace, writable);
    mbar.Enable(hexbed::menu::MenuEdit_InsertRandom, writable);
    if (findDialog_) findDialog_->AllowReplace(writable);
    UpdateMenuEnabledSelect(ed);
    OnEditorCopy(ed);
    OnUndoRedo(ed);
}

void HexBedMainFrame::OnDocumentEdit(wxCommandEvent& event) {
    int sel = tabs_->GetSelection();
    if (sel != wxNOT_FOUND) UpdateTabSymbol(static_cast<std::size_t>(sel));
}

void HexBedMainFrame::OnLastTabClose() {
    SetTitle("HexBed");
    UpdateFileOnly();
    InitMenuEnabled();
    hexbed::menu::updateStatusBarNoFile(GetStatusBar(), context_->state);
    context_->announceCursorUpdate(HexBedPeekRegion{});
}

void HexBedMainFrame::OnTabClose(wxAuiNotebookEvent& event) {
    auto s = event.GetSelection();
    if (s == wxNOT_FOUND) return;
    if (!FileClose(s))
        event.Veto();
    else {
        if (tabs_->GetPageCount() <= 1) OnLastTabClose();
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
    editor->Bind(HEX_EDIT_EVENT, &HexBedMainFrame::OnDocumentEdit, this);
    context_->addWindow(editor.get());
    if (editor->document().readOnly()) {
        tabs_->SetPageToolTip(i, wxString::Format(_("[read-only] %s"), path));
        tabs_->SetPageText(i, wxString::Format("[%s]", tabs_->GetPageText(i)));
    }
    UpdateFileOnly();
    UpdateMenuEnabled(*editor);
    editor.release();
}

bool HexBedMainFrame::FileNew() {
    try {
        AddTab(MakeEditor(), wxString::Format(_("New-%llu"), ++newFileIndex_),
               wxEmptyString);
        return true;
    } catch (...) {
        try {
            wxMessageBox(wxString::Format(_("Failed to create a new file: %s"),
                                          currentExceptionAsString()),
                         "HexBed", wxOK | wxICON_ERROR);
        } catch (...) {
        }
        return false;
    }
}

void HexBedMainFrame::OnFileNew(wxCommandEvent& event) { FileNew(); }

void HexBedMainFrame::FileKnock(const wxString& fp, bool readOnly) {
    try {
        auto editor = MakeEditor(pathFromWxString(fp), readOnly);
        std::filesystem::path path =
            std::filesystem::canonical(editor->document().path());
        AddTab(std::move(editor), pathToWxString(path.filename()),
               pathToWxString(path));
    } catch (...) {
        try {
            wxMessageBox(wxString::Format(_("Failed to open file %s: %s"), fp,
                                          currentExceptionAsString()),
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
    findDialog_->Destroy();
    findDialog_ = nullptr;
}

void HexBedMainFrame::OnBitEditorClose(wxCloseEvent& event) {
    bitEditorTool_->Destroy();
    bitEditorTool_ = nullptr;
}

void HexBedMainFrame::OnDataInspectorClose(wxCloseEvent& event) {
    dataInspector_->Destroy();
    dataInspector_ = nullptr;
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
            std::make_unique<FindDialog>(this, context_, searchDocument_);
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
            this, context_, searchDocument_, replaceDocument_);
        findDialog_->Bind(wxEVT_CLOSE_WINDOW, &HexBedMainFrame::OnFindClose,
                          this);
    }
    findDialog_->Show(true);
    findDialog_->Raise();
    findDialog_->SetFocus();
}

void HexBedMainFrame::OnEditInsertOrReplace(wxCommandEvent& event) {
    hexbed::ui::HexBedEditor* ed = GetEditor();
    bufsize sel, seln;
    bool seltext;
    ed->GetSelection(sel, seln, seltext);
    InsertBlockDialog insertDialog(this, context_, insertDocument_, seln);
    if (ed && insertDialog.ShowModal() == wxID_OK) {
        bufsize n = insertDialog.GetByteCount();
        try {
            bufsize sn = insertDocument_->size();
            auto sb_ = std::make_unique<byte[]>(sn);
            byte* sb = sb_.get();
            bufsize sr = insertDocument_->read(0, bytespan{sb, sb + sn});
            if (ed->document().replace(sel, seln, n, sr, sb))
                ed->SelectBytes(
                    sel, n, SelectFlags().caretAtBeginning().highlightCaret());
        } catch (...) {
            try {
                wxMessageBox(wxString::Format(_("Insert failed: %s"),
                                              currentExceptionAsString()),
                             "HexBed", wxOK | wxICON_ERROR);
            } catch (...) {
            }
        }
    }
}

void HexBedMainFrame::OnEditInsertRandom(wxCommandEvent& event) {
    hexbed::ui::HexBedEditor* ed = GetEditor();
    bufsize sel, seln;
    bool seltext;
    ed->GetSelection(sel, seln, seltext);
    InsertRandomBlockDialog insertRandomDialog(this, seln);
    if (ed && insertRandomDialog.ShowModal() == wxID_OK) {
        bufsize n = insertRandomDialog.GetByteCount();
        RandomType type = insertRandomDialog.GetRandomType();
        try {
            if (ed->document().pry(
                    sel, seln,
                    [n, type](HexBedTask& task,
                              std::function<void(const_bytespan)> outp) {
                        byte blk[BUFFER_SIZE];
                        for (bufsize i = 0; !task.isCancelled() && i < n;) {
                            task.progress(i);
                            bufsize r = std::min<bufsize>(sizeof(blk), n - i);
                            randomizeBuffer(type, blk, r);
                            outp(const_bytespan{blk, r});
                            i += r;
                        }
                    },
                    n))
                ed->SelectBytes(
                    sel, n, SelectFlags().caretAtBeginning().highlightCaret());
        } catch (...) {
            try {
                wxMessageBox(wxString::Format(_("Insert failed: %s"),
                                              currentExceptionAsString()),
                             "HexBed", wxOK | wxICON_ERROR);
            } catch (...) {
            }
        }
    }
}

void HexBedMainFrame::OnEditBitwiseBinaryOp(wxCommandEvent& event) {
    hexbed::ui::HexBedEditor* ed = GetEditor();
    bufsize sel, seln;
    bool seltext;
    ed->GetSelection(sel, seln, seltext);
    BitwiseBinaryOpDialog dial(this, context_, binaryOpDocument_);
    if (ed && dial.ShowModal() == wxID_OK) {
        try {
            bufsize sn = binaryOpDocument_->size();
            auto sb_ = std::make_unique<byte[]>(sn);
            byte* sb = sb_.get();
            bufsize sr = binaryOpDocument_->read(0, bytespan{sb, sb + sn});
            if (!sr) return;
            doBitwiseBinaryOp(ed->document(), sel, seln,
                              const_bytespan{sb, sb + sr}, dial.GetOperation());
        } catch (...) {
            try {
                wxMessageBox(wxString::Format(_("Bitwise binary op failed: %s"),
                                              currentExceptionAsString()),
                             "HexBed", wxOK | wxICON_ERROR);
            } catch (...) {
            }
        }
    }
}

void HexBedMainFrame::OnEditBitwiseUnaryOp(wxCommandEvent& event) {
    hexbed::ui::HexBedEditor* ed = GetEditor();
    bufsize sel, seln;
    bool seltext;
    ed->GetSelection(sel, seln, seltext);
    BitwiseUnaryOpDialog dial(this);
    if (ed && dial.ShowModal() == wxID_OK) {
        try {
            doBitwiseUnaryOp(ed->document(), sel, seln, dial.GetOperation());
        } catch (...) {
            try {
                wxMessageBox(wxString::Format(_("Bitwise unary op failed: %s"),
                                              currentExceptionAsString()),
                             "HexBed", wxOK | wxICON_ERROR);
            } catch (...) {
            }
        }
    }
}

void HexBedMainFrame::OnEditBitwiseShiftOp(wxCommandEvent& event) {
    hexbed::ui::HexBedEditor* ed = GetEditor();
    bufsize sel, seln;
    bool seltext;
    ed->GetSelection(sel, seln, seltext);
    BitwiseShiftOpDialog dial(this);
    if (ed && dial.ShowModal() == wxID_OK) {
        try {
            doBitwiseShiftOp(ed->document(), sel, seln, dial.GetShiftCount(),
                             dial.GetOperation());
        } catch (...) {
            try {
                wxMessageBox(wxString::Format(_("Bitwise shift op failed: %s"),
                                              currentExceptionAsString()),
                             "HexBed", wxOK | wxICON_ERROR);
            } catch (...) {
            }
        }
    }
}

void HexBedMainFrame::OnEditByteSwap2(wxCommandEvent& event) {
    hexbed::ui::HexBedEditor* ed = GetEditor();
    bufsize sel, seln;
    bool seltext;
    ed->GetSelection(sel, seln, seltext);
    if (seln) doByteSwapOp<2>(ed->document(), sel, seln);
}

void HexBedMainFrame::OnEditByteSwap4(wxCommandEvent& event) {
    hexbed::ui::HexBedEditor* ed = GetEditor();
    bufsize sel, seln;
    bool seltext;
    ed->GetSelection(sel, seln, seltext);
    if (seln) doByteSwapOp<4>(ed->document(), sel, seln);
}

void HexBedMainFrame::OnEditByteSwap8(wxCommandEvent& event) {
    hexbed::ui::HexBedEditor* ed = GetEditor();
    bufsize sel, seln;
    bool seltext;
    ed->GetSelection(sel, seln, seltext);
    if (seln) doByteSwapOp<8>(ed->document(), sel, seln);
}

void HexBedMainFrame::OnEditByteSwap16(wxCommandEvent& event) {
    hexbed::ui::HexBedEditor* ed = GetEditor();
    bufsize sel, seln;
    bool seltext;
    ed->GetSelection(sel, seln, seltext);
    if (seln) doByteSwapOp<16>(ed->document(), sel, seln);
}

void HexBedMainFrame::OnEditReverse(wxCommandEvent& event) {
    hexbed::ui::HexBedEditor* ed = GetEditor();
    bufsize sel, seln;
    bool seltext;
    ed->GetSelection(sel, seln, seltext);
    if (seln) ed->document().reverse(sel, seln);
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

void HexBedMainFrame::OnViewDataInspector(wxCommandEvent& event) {
    if (!dataInspector_) {
        dataInspector_ = std::make_unique<DataInspector>(this, context_);
        dataInspector_->Bind(wxEVT_CLOSE_WINDOW,
                             &HexBedMainFrame::OnDataInspectorClose, this);
    }
    dataInspector_->Show(true);
    dataInspector_->Raise();
    dataInspector_->SetFocus();
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
    for (const wxString& file : files) FileKnock(file, readOnly);
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

void HexBedMainFrame::OnFileMenuImport(wxCommandEvent& event) {
    int i = event.GetId() - fileMenus_.firstImportId;
    if (i >= 0 && i < fileMenus_.importPluginCount) {
        hexbed::plugins::ImportPlugin& plugin =
            hexbed::plugins::importPluginByIndex(i);
        wxFileDialog dial(this, _("Import file"), "", "",
                          plugin.getFileFilter(),
                          wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (dial.ShowModal() == wxID_CANCEL) return;
        std::filesystem::path fn(pathFromWxString(dial.GetPath()));
        if (plugin.configureImport(this, fn)) {
            if (!FileNew()) return;
            hexbed::ui::HexBedEditor* ed = GetEditor();
            HexBedDocument& doc = ed->document();
            try {
                doc.romp(
                    [&plugin, &fn](
                        HexBedTask& task,
                        std::function<void(bufsize, const_bytespan)> outp) {
                        plugin.doImport(task, fn, outp);
                    });
            } catch (const hexbed::plugins::ImportFileInvalidError& e) {
                try {
                    wxString msg = wxGetTranslation(e.what());
                    if (e.lineKnown())
                        msg = wxString::Format(_("Error on line %llu:\n%s"),
                                               e.lineNumber(), msg);
                    wxMessageBox(wxString::Format(_("Failed to import:\nImport "
                                                    "file was invalid.\n%s"),
                                                  msg),
                                 "HexBed", wxOK | wxICON_ERROR);
                } catch (...) {
                }
            } catch (...) {
                try {
                    wxMessageBox(wxString::Format(_("Failed to import:\n%s"),
                                                  currentExceptionAsString()),
                                 "HexBed", wxOK | wxICON_ERROR);
                } catch (...) {
                }
            }
        }
    }
}

void HexBedMainFrame::OnFileMenuExport(wxCommandEvent& event) {
    int i = event.GetId() - fileMenus_.firstExportId;
    hexbed::ui::HexBedEditor* ed = GetEditor();
    if (ed && i >= 0 && i < fileMenus_.exportPluginCount) {
        HexBedDocument& doc = ed->document();
        hexbed::plugins::ExportPlugin& plugin =
            hexbed::plugins::exportPluginByIndex(i);
        std::filesystem::path pathdir;
        if (doc.filed()) {
            try {
                pathdir = doc.path().parent_path();
            } catch (...) {
            }
        }
        wxFileDialog dial(this, _("Export file"), pathToWxString(pathdir), "",
                          plugin.getFileFilter(),
                          wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (dial.ShowModal() == wxID_CANCEL) return;
        bufsize sel, seln;
        bool seltext;
        ed->GetSelection(sel, seln, seltext);
        if (!seln) {
            sel = 0;
            seln = doc.size();
        }
        std::filesystem::path fn(pathFromWxString(dial.GetPath()));
        hexbed::plugins::ExportDetails details{.columns = ed->GetColumnCount()};
        try {
            details.filename =
                stringFromWx(pathToWxString(doc.path().filename()));
        } catch (...) {
        }
        if (plugin.configureExport(this, fn, sel, seln, details)) {
            bufsize beg = sel, end = seln;
            try {
                HexBedTask(&ed->context(), 0, true)
                    .run([&plugin, &doc, &fn, beg, end,
                          &details](HexBedTask& task) {
                        plugin.doExport(
                            task, fn,
                            [&doc, beg, end](bufsize off,
                                             bytespan arr) -> bufsize {
                                bufsize read =
                                    std::min<bufsize>(arr.size(), end - off);
                                return doc.read(off + beg,
                                                bytespan{arr.data(), read});
                            },
                            beg, end, details);
                    });
            } catch (...) {
                try {
                    wxMessageBox(wxString::Format(_("Failed to export:\n%s"),
                                                  currentExceptionAsString()),
                                 "HexBed", wxOK | wxICON_ERROR);
                } catch (...) {
                }
            }
        }
    }
}

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
        } catch (const hexbed::clip::ClipboardError& e) {
            try {
                wxMessageBox(
                    wxString::Format(_("Failed to copy:\n%s"),
                                     _("Failed to open the clipboard")),
                    "HexBed", wxOK | wxICON_ERROR);
            } catch (...) {
            }
            return;
        } catch (...) {
            try {
                wxMessageBox(wxString::Format(_("Failed to copy:\n%s"),
                                              currentExceptionAsString()),
                             "HexBed", wxOK | wxICON_ERROR);
            } catch (...) {
            }
            return;
        }
        if constexpr (cut) {
            if (ed->document().remove(sel, seln)) {
                ed->SelectBytes(sel, 0, SelectFlags().highlightCaret());
                ed->HintBytesChanged(sel);
            }
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
        } catch (const hexbed::clip::ClipboardError& e) {
            try {
                wxMessageBox(
                    wxString::Format(_("Failed to paste:\n%s"),
                                     _("Failed to open the clipboard")),
                    "HexBed", wxOK | wxICON_ERROR);
            } catch (...) {
            }
            return;
        } catch (...) {
            try {
                wxMessageBox(wxString::Format(_("Failed to paste:\n%s"),
                                              currentExceptionAsString()),
                             "HexBed", wxOK | wxICON_ERROR);
            } catch (...) {
            }
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
        if (ed->document().remove(sel, seln))
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
