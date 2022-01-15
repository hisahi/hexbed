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
    fileOnly.push_back(
        addItem(menuEdit, MenuEdit_InsertOrReplace, _("Ins&ert or replace..."),
                _("Inserts or replaces the selection with a block of bytes"),
                wxACCEL_CTRL, 'I'));
    fileOnly.push_back(addItem(
        menuEdit, MenuEdit_InsertRandom, _("Insert rando&m..."),
        _("Inserts or replaces the selection with a random block of bytes")));
    wxMenu* editOps = new wxMenu;
    wxMenu* editSwapOps = new wxMenu;
    fileOnly.push_back(addItem(
        editOps, MenuEdit_BitwiseBinaryOp, _("Bitwise &binary operation..."),
        _("Performs a binary bitwise operation on the selected block")));
    fileOnly.push_back(addItem(
        editOps, MenuEdit_BitwiseUnaryOp, _("Bitwise &unary operation..."),
        _("Performs a unary bitwise operation on the selected block")));
    fileOnly.push_back(addItem(
        editOps, MenuEdit_BitwiseShiftOp, _("Bitwise &shift operation..."),
        _("Performs a bitwise shift operation on the selected block")));
    editOps->AppendSeparator();
    fileOnly.push_back(addItem(
        editSwapOps, MenuEdit_ByteSwap2, _("Byte swap (&2-byte words)"),
        _("Swaps the byte order in the selected block of 2-byte words")));
    fileOnly.push_back(addItem(
        editSwapOps, MenuEdit_ByteSwap4, _("Byte swap (&4-byte words)"),
        _("Swaps the byte order in the selected block of 4-byte words")));
    fileOnly.push_back(addItem(
        editSwapOps, MenuEdit_ByteSwap8, _("Byte swap (&8-byte words)"),
        _("Swaps the byte order in the selected block of 8-byte words")));
    fileOnly.push_back(addItem(
        editSwapOps, MenuEdit_ByteSwap16, _("Byte swap (1&6-byte words)"),
        _("Swaps the byte order in the selected block of 16-byte words")));
    editOps->AppendSubMenu(editSwapOps, _("Byte s&wap"),
                           _("Performs byte order swaps on the selection"));
    editOps->AppendSeparator();
    fileOnly.push_back(
        addItem(editOps, MenuEdit_Reverse, _("&Reverse"),
                _("Reverses the order of bytes within the selection")));
    menuEdit->AppendSubMenu(editOps, _("&Operations"),
                            _("Performs operations on the selection"));
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
