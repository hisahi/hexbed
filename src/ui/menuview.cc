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
// ui/menuview.cc -- implementation for the View menu

#include "app/config.hh"
#include "ui/menus.hh"

namespace hexbed {
namespace menu {

wxMenu* createViewMenu(wxMenuBar* menuBar, std::vector<wxMenuItem*>& fileOnly) {
    wxMenu* menuView = new wxMenu;
    wxMenu* viewColumns = new wxMenu;
    viewColumns
        ->AppendRadioItem(MenuView_ShowColumnsBoth, _("&Hex and text"),
                          _("Both columns; hex and text data"))
        ->Check(config().showColumnTypes == 3);
    viewColumns
        ->AppendRadioItem(MenuView_ShowColumnsHex, _("He&x only"),
                          _("Show hex column only"))
        ->Check(config().showColumnTypes == 2);
    viewColumns
        ->AppendRadioItem(MenuView_ShowColumnsText, _("&Text only"),
                          _("Show text column only"))
        ->Check(config().showColumnTypes == 1);
    menuView->AppendSubMenu(viewColumns, _("&Columns"),
                            _("Controls which columns to show"));
    menuView->AppendSeparator();
    addItem(menuView, MenuView_BitEditor, _("&Bit editor"),
            _("Shows the bit editor"), wxACCEL_CTRL | wxACCEL_SHIFT, 'B');
    addItem(menuView, MenuView_DataInspector, _("&Data inspector"),
            _("Shows the data inspector"), wxACCEL_CTRL, 'D');
    menuBar->Append(menuView, _("&View"));
    return menuView;
}

};  // namespace menu
};  // namespace hexbed
