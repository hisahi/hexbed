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
// ui/menufile.cc -- implementation for the File menu

#include "ui/menus.hh"

namespace hexbed {
namespace menu {

wxMenu* createFileMenu(wxMenuBar* menuBar, std::vector<wxMenuItem*>& fileOnly) {
    wxMenu* menuFile = new wxMenu;
    addItem(menuFile, wxID_NEW, _("&New"), _("Creates a new file"),
            wxACCEL_CTRL, 'N');
    addItem(menuFile, wxID_OPEN, _("&Open..."), _("Opens an existing file"),
            wxACCEL_CTRL, 'O');
    fileOnly.push_back(addItem(menuFile, wxID_SAVE, _("&Save"),
                               _("Saves the file in the current location"),
                               wxACCEL_CTRL, 'S'));
    fileOnly.push_back(addItem(menuFile, wxID_SAVEAS, _("Save &As..."),
                               _("Saves the file in a new location")));
    fileOnly.push_back(addItem(menuFile, MenuFile_Reload, _("&Reload"),
                               _("Reload the open file and discard changes")));
    fileOnly.push_back(addItem(menuFile, wxID_CLOSE, _("&Close"),
                               _("Closes the current file"), wxACCEL_CTRL,
                               WXK_F4));
    menuFile->AppendSeparator();
    fileOnly.push_back(addItem(menuFile, MenuFile_SaveAll, _("Sa&ve all"),
                               _("Saves all open files")));
    fileOnly.push_back(addItem(menuFile, MenuFile_CloseAll, _("C&lose all"),
                               _("Closes all open files")));
    menuFile->AppendSeparator();
    addItem(menuFile, wxID_EXIT, _("E&xit"), _("Closes the application"),
            wxACCEL_CTRL, 'Q');
    menuBar->Append(menuFile, _("&File"));
    return menuFile;
}

};  // namespace menu
};  // namespace hexbed
