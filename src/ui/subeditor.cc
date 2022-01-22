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
// ui/subeditor.cc -- impl for the HexBed subview editor class

#include "ui/subeditor.hh"

#include "ui/hexbed.hh"

namespace hexbed {
namespace ui {

HexBedSubEditor::HexBedSubEditor(HexBedMainFrame* frame, HexBedSubView* parent,
                                 HexBedContextMain* ctx,
                                 std::shared_ptr<HexBedDocument> document)
    : HexBedEditor(frame, parent, ctx, std::move(document), true),
      subviewParent_(parent) {}

void HexBedSubEditor::OnMainFileClose() { subviewParent_->OnMainFileClose(); }

HexBedSubView::HexBedSubView(HexBedMainFrame* frame, HexBedContextMain* ctx,
                             const std::shared_ptr<HexBedDocument>& document,
                             const wxString& title, bufsize pos)
    : wxFrame(frame, wxID_ANY, title, wxDefaultPosition, wxDefaultSize,
              wxRESIZE_BORDER | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN),
      ctx_(ctx) {
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add((editor_ = new HexBedSubEditor(frame, this, ctx, document)),
               wxSizerFlags().Expand().Proportion(1));
    SetSizer(sizer);
    ctx->addWindow(editor_, true);
    editor_->SelectBytes(pos, 0, SelectFlags().highlightBeginning());
}

HexBedSubView::~HexBedSubView() { ctx_->removeWindow(editor_); }

void HexBedSubView::OnMainFileClose() { Destroy(); }

};  // namespace ui
};  // namespace hexbed
