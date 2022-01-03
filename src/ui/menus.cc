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
// ui/menus.cc -- impl for some main program UI menu related functions

#include "ui/menus.hh"

namespace hexbed {
namespace menu {

wxMenuItem* addItem(wxMenu* menu, int id, const wxString& text,
                    const wxString& tip) {
    return menu->Append(id, text, tip);
}

wxMenuItem* addItem(wxMenu* menu, int id, const wxString& text,
                    const wxString& tip, int flags, int keyCode) {
    wxMenuItem* item = menu->Append(id, text, tip);
    wxAcceleratorEntry* accel =
        new wxAcceleratorEntry(flags, keyCode, id, item);
    item->SetAccel(accel);
    return item;
}

wxMenuItem* addCheckItem(wxMenu* menu, int id, const wxString& text,
                         const wxString& tip, int flags, int keyCode) {
    wxMenuItem* item = menu->AppendCheckItem(id, text, tip);
    wxAcceleratorEntry* accel =
        new wxAcceleratorEntry(flags, keyCode, id, item);
    item->SetAccel(accel);
    return item;
}

};  // namespace menu
};  // namespace hexbed
