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

wxMenu* createViewMenu(wxMenuBar* menuBar, std::vector<wxMenuItem*>& fileOnly,
                       MenuIds& menus) {
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
    wxMenu* viewUtf = new wxMenu;
    viewUtf
        ->AppendRadioItem(MenuView_UTFModeOff, _("&Off"),
                          _("Show single-byte characters"))
        ->Check(config().utfMode == 0);
    viewUtf
        ->AppendRadioItem(MenuView_UTFMode16LE, _("UTF-16LE (little endian)"),
                          _("Show little-endian UTF-16 characters"))
        ->Check(config().utfMode == 1);
    viewUtf
        ->AppendRadioItem(MenuView_UTFMode16BE, _("UTF-16BE (big endian)"),
                          _("Show big-endian UTF-16 characters"))
        ->Check(config().utfMode == 2);
    viewUtf
        ->AppendRadioItem(MenuView_UTFMode32LE, _("UTF-32LE (little endian)"),
                          _("Show little-endian UTF-32 characters"))
        ->Check(config().utfMode == 3);
    viewUtf
        ->AppendRadioItem(MenuView_UTFMode32BE, _("UTF-32BE (big endian)"),
                          _("Show big-endian UTF-32 characters"))
        ->Check(config().utfMode == 4);
    menuView->AppendSubMenu(viewUtf, _("&UTF mode"),
                            _("Controls UTF text mode"));
    menuView->AppendSeparator();
    addItem(menuView, MenuView_BitEditor, _("&Bit editor"),
            _("Shows the bit editor"), wxACCEL_CTRL | wxACCEL_SHIFT, 'B');
    addItem(menuView, MenuView_DataInspector, _("&Data inspector"),
            _("Shows the data inspector"), wxACCEL_CTRL, 'D');
    addItem(menuView, MenuView_TextConverter, _("Te&xt converter"),
            _("Shows the text converter"), wxACCEL_CTRL | wxACCEL_SHIFT, 'T');

    int bookmarkGetId =
        wxWindow::NewControlId(DocumentMetadata::BOOKMARK_COUNT);
    int bookmarkSetId =
        wxWindow::NewControlId(DocumentMetadata::BOOKMARK_COUNT);
    menus.firstBookmarkGetId = bookmarkGetId;
    menus.firstBookmarkSetId = bookmarkSetId;
    if (bookmarkGetId != wxID_NONE && bookmarkSetId != wxID_NONE) {
        menuView->AppendSeparator();
        wxMenu* bookmarkGetMenu = new wxMenu;
        wxMenu* bookmarkSetMenu = new wxMenu;

        for (std::size_t i = 0; i < DocumentMetadata::BOOKMARK_COUNT; ++i) {
            wxString text =
                wxString::Format(_("Bookmark &%u"), static_cast<unsigned>(i));
            addItem(bookmarkGetMenu, bookmarkGetId + i, text, wxEmptyString,
                    wxACCEL_CTRL | wxACCEL_SHIFT, '0' + i);
            fileOnly.push_back(addItem(bookmarkSetMenu, bookmarkSetId + i, text,
                                       wxEmptyString,
                                       wxACCEL_ALT | wxACCEL_SHIFT, '0' + i));
        }

        menuView->AppendSubMenu(bookmarkGetMenu, _("&Jump to bookmark"),
                                _("Jumps to a bookmark (jump point)"));
        menuView->AppendSubMenu(bookmarkSetMenu, _("S&et bookmark"),
                                _("Sets a bookmark (jump point)"));
        fileOnly.push_back(addItem(menuView, MenuView_BookmarkPrev,
                                   _("Pre&vious bookmark"),
                                   _("Jumps to the bookmark closest to the "
                                     "cursor when going backwards if any"),
                                   wxACCEL_CTRL | wxACCEL_SHIFT, ','));
        fileOnly.push_back(addItem(menuView, MenuView_BookmarkNext,
                                   _("Ne&xt bookmark"),
                                   _("Jumps to the bookmark closest to the "
                                     "cursor when going forwards if any"),
                                   wxACCEL_CTRL | wxACCEL_SHIFT, '.'));
    }
    menuView->AppendSeparator();
    fileOnly.push_back(addItem(menuView, MenuView_NewSubView, _("Add subvie&w"),
                               _("Adds a new subview"),
                               wxACCEL_CTRL | wxACCEL_SHIFT, 'W'));
    menuBar->Append(menuView, _("&View"));
    return menuView;
}

};  // namespace menu
};  // namespace hexbed
