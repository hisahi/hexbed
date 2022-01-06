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
// ui/menuedit.cc -- implementation for the Edit menu

#include "ui/hexbed.hh"
#include "ui/menus.hh"

namespace hexbed {
namespace menu {

wxMenu* createEditMenu(wxMenuBar* menuBar, std::vector<wxMenuItem*>& fileOnly) {
    wxMenu* menuEdit = new wxMenu;
    addItem(menuEdit, wxID_UNDO, _("&Undo"), _("Undoes the last action"),
            wxACCEL_CTRL, 'Z');
    addItem(menuEdit, wxID_REDO, _("&Redo"), _("Redoes the last undone action"),
            wxACCEL_CTRL, 'Y');
    menuEdit->AppendSeparator();
    addItem(menuEdit, wxID_CUT, _("Cu&t"),
            _("Copies the selection and removes it"), wxACCEL_CTRL, 'X');
    addItem(menuEdit, wxID_COPY, _("&Copy"), _("Copies the selection"),
            wxACCEL_CTRL, 'C');
    addItem(menuEdit, wxID_PASTE, _("&Paste Insert"),
            _("Pastes data from the clipboard and inserts it at the caret"),
            wxACCEL_CTRL, 'V');
    addItem(menuEdit, MenuEdit_PasteReplace, _("P&aste Overwrite"),
            _("Pastes data from the clipboard, overwriting existing data"),
            wxACCEL_CTRL, 'B');
    addItem(menuEdit, wxID_DELETE, _("&Delete"),
            _("Deletes the currently selected data"), wxACCEL_NORMAL,
            WXK_DELETE);
    menuEdit->AppendSeparator();
    fileOnly.push_back(addItem(menuEdit, wxID_SELECTALL, _("Select &all"),
                               _("Selects the entire file"), wxACCEL_CTRL,
                               'A'));
    fileOnly.push_back(
        addItem(menuEdit, MenuEdit_SelectRange, _("Select &block..."),
                _("Selects a block of bytes"), wxACCEL_CTRL, 'E'));
    menuEdit->AppendSeparator();
    addCheckItem(
        menuEdit, MenuEdit_InsertMode, _("&Insert mode"),
        _("Inserts bytes typed in instead of overwriting existing ones"),
        wxACCEL_NORMAL, WXK_INSERT);
    menuEdit->AppendSeparator();
    addItem(menuEdit, wxID_PREFERENCES, _("&Options"),
            _("Opens the dialog for editing options or preferences"),
            wxACCEL_CTRL, 'O');
    menuBar->Append(menuEdit, _("&Edit"));
    return menuEdit;
}

};  // namespace menu
};  // namespace hexbed