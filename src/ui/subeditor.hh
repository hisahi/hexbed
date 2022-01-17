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
// ui/subeditor.hh -- header for the HexBed subview editor class

#ifndef HEXBED_UI_SUBEDITOR_HH
#define HEXBED_UI_SUBEDITOR_HH

#include <wx/frame.h>

#include "ui/editor.hh"

namespace hexbed {
namespace ui {

class HexBedSubView;

class HexBedSubEditor : public HexBedEditor {
  public:
    HexBedSubEditor(HexBedMainFrame* frame, HexBedSubView* parent,
                    HexBedContextMain* ctx,
                    std::shared_ptr<HexBedDocument> document);

    inline bool IsSubView() const override { return true; }
    void OnMainFileClose();

  private:
    HexBedSubView* subviewParent_;
};

class HexBedSubView : public wxFrame {
  public:
    HexBedSubView(HexBedMainFrame* frame, HexBedContextMain* ctx,
                  const std::shared_ptr<HexBedDocument>& document,
                  const wxString& title);
    HexBedSubView(const HexBedSubView& copy) = delete;
    HexBedSubView(HexBedSubView&& move) = delete;
    HexBedSubView& operator=(const HexBedSubView& copy) = delete;
    HexBedSubView& operator=(HexBedSubView&& move) = delete;
    ~HexBedSubView();

    void OnMainFileClose();

  private:
    HexBedContextMain* ctx_;
    HexBedSubEditor* editor_;
};

};  // namespace ui
};  // namespace hexbed

#endif /* HEXBED_UI_SUBEDITOR_HH */
