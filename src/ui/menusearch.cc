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
// ui/menusearch.cc -- implementation for the Search menu

#include "ui/menus.hh"

namespace hexbed {
namespace menu {

wxMenu* createSearchMenu(wxMenuBar* menuBar, std::vector<wxMenuItem*>& fileOnly,
                         MenuIds& menus) {
    wxMenu* menuSearch = new wxMenu;
    fileOnly.push_back(addItem(menuSearch, wxID_FIND, _("&Find..."),
                               _("Finds data and brings it into view"),
                               wxACCEL_CTRL, 'F'));
    fileOnly.push_back(addItem(menuSearch, MenuSearch_FindNext, _("Fi&nd next"),
                               _("Finds the next match with the same search"),
                               wxACCEL_NORMAL, WXK_F3));
    fileOnly.push_back(
        addItem(menuSearch, MenuSearch_FindPrevious, _("Find &previous"),
                _("Finds the previous match with the same search"),
                wxACCEL_SHIFT, WXK_F3));
    fileOnly.push_back(addItem(menuSearch, wxID_REPLACE, _("&Replace..."),
                               _("Finds and replaces data in the file"),
                               wxACCEL_CTRL, 'H'));
    menuSearch->AppendSeparator();
    fileOnly.push_back(addItem(menuSearch, MenuSearch_GoTo, _("&Go to..."),
                               _("Goes to a specific offset in the file"),
                               wxACCEL_CTRL, 'G'));
    menuBar->Append(menuSearch, _("&Search"));
    return menuSearch;
}

};  // namespace menu
};  // namespace hexbed
