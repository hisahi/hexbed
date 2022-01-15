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
// ui/menus.hh -- header for the main program UI menus

#ifndef HEXBED_UI_MENUS_HH
#define HEXBED_UI_MENUS_HH

#include <wx/accel.h>
#include <wx/menu.h>
#include <wx/statusbr.h>
#include <wx/string.h>
#include <wx/toolbar.h>

#include <vector>

#include "ui/context.hh"
#include "ui/hexbed-fwd.hh"

namespace hexbed {
namespace menu {

enum {
    MenuFile_SaveAll = 0x100,
    MenuFile_CloseAll,
    MenuFile_Reload,

    MenuEdit_PasteReplace = 0x200,
    MenuEdit_InsertMode,
    MenuEdit_SelectRange,
    MenuEdit_InsertOrReplace,
    MenuEdit_InsertRandom,
    MenuEdit_BitwiseBinaryOp,
    MenuEdit_BitwiseUnaryOp,
    MenuEdit_BitwiseShiftOp,
    MenuEdit_ByteSwap2,
    MenuEdit_ByteSwap4,
    MenuEdit_ByteSwap8,
    MenuEdit_ByteSwap16,
    MenuEdit_Reverse,

    MenuSearch_FindNext = 0x300,
    MenuSearch_FindPrevious,
    MenuSearch_GoTo,

    MenuView_ShowColumnsBoth = 0x400,
    MenuView_ShowColumnsHex,
    MenuView_ShowColumnsText,
    MenuView_BitEditor,
    MenuView_DataInspector,
};

// menus.cc
wxMenuItem* addItem(wxMenu* menu, int id, const wxString& text,
                    const wxString& tip);
wxMenuItem* addItem(wxMenu* menu, int id, const wxString& text,
                    const wxString& tip, int flags, int keyCode);
wxMenuItem* addCheckItem(wxMenu* menu, int id, const wxString& text,
                         const wxString& tip, int flags, int keyCode);

struct FileMenus {
    wxMenu* importMenu{nullptr};
    wxMenu* exportMenu{nullptr};

    int firstImportId;
    int importPluginCount{0};
    int firstExportId;
    int exportPluginCount{0};
};

// menufile.cc
wxMenu* createFileMenu(wxMenuBar* menuBar, std::vector<wxMenuItem*>& fileOnly,
                       FileMenus& menus);
// menuedit.cc
wxMenu* createEditMenu(wxMenuBar* menuBar, std::vector<wxMenuItem*>& fileOnly);
// menusearch.cc
wxMenu* createSearchMenu(wxMenuBar* menuBar,
                         std::vector<wxMenuItem*>& fileOnly);
// menuview.cc
wxMenu* createViewMenu(wxMenuBar* menuBar, std::vector<wxMenuItem*>& fileOnly);
// menuhelp.cc
wxMenu* createHelpMenu(wxMenuBar* menuBar, std::vector<wxMenuItem*>& fileOnly);
// status.cc
void populateStatusBar(wxStatusBar* statusBar);
void updateStatusBarNoFile(wxStatusBar* statusBar, const EditorState& state);
// toolbar.cc
void populateToolBar(wxToolBar* toolBar,
                     std::vector<wxToolBarToolBase*> fileOnly);

};  // namespace menu
};  // namespace hexbed

#endif /* HEXBED_UI_MENUS_HH */
