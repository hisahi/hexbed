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
// ui/menuhelp.cc -- implementation for the Help menu

#include <wx/artprov.h>

#include "ui/menus.hh"

namespace hexbed {
namespace menu {

void populateToolBar(wxToolBar* toolBar,
                     std::vector<wxToolBarToolBase*> fileOnly) {
    toolBar->AddTool(wxID_NEW, _("New"), wxArtProvider::GetIcon(wxART_NEW));
    toolBar->AddTool(wxID_OPEN, _("Open"),
                     wxArtProvider::GetIcon(wxART_FILE_OPEN));
    fileOnly.push_back(toolBar->AddTool(
        wxID_SAVE, _("Save"), wxArtProvider::GetIcon(wxART_FILE_SAVE)));
    toolBar->AddSeparator();
    toolBar->AddTool(wxID_EXIT, _("Exit"), wxArtProvider::GetIcon(wxART_QUIT));
    toolBar->Realize();
}

};  // namespace menu
};  // namespace hexbed
