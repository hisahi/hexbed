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
// ui/saeditor.cc -- implementation for the HexBed standalone editor class

#include "ui/saeditor.hh"

#include <bit>
#include <iomanip>
#include <sstream>

#include "app/config.hh"
#include "common/logger.hh"
#include "ui/config.hh"
#include "ui/hexbed.hh"
#include "ui/hexedit.hh"

namespace hexbed {
namespace ui {

HexBedStandaloneEditor::HexBedStandaloneEditor(
    wxWindow* parent, HexBedContextMain* ctx,
    std::shared_ptr<HexBedDocument>&& document)
    : wxPanel(parent),
      document_(std::move(document)),
      ctx_(ctx),
      timer_(this, wxID_ANY) {
    wxBoxSizer* box_ = new wxBoxSizer(wxHORIZONTAL);
    SetSizer(box_);
    int gap = 4;

    hexEdit_ = new HexEditor(this, this);
    scroll_ = new LongScrollBar(this, wxID_ANY, wxDefaultPosition,
                                wxDefaultSize, wxSB_VERTICAL);

    box_->Add(hexEdit_,
              wxSizerFlags().Expand().Proportion(1).Border(wxALL, gap));
    box_->Add(scroll_, wxSizerFlags().Expand().Border(wxALL, gap));

    hexEdit_->FitInside();

    Bind(wxEVT_SIZE, &HexBedStandaloneEditor::OnResize, this);
    Bind(wxEVT_TIMER, &HexBedStandaloneEditor::OnResizeTimer, this);

    scroll_->Bind(wxEVT_SCROLL_TOP, &HexBedStandaloneEditor::OnScroll, this);
    scroll_->Bind(wxEVT_SCROLL_BOTTOM, &HexBedStandaloneEditor::OnScroll, this);
    scroll_->Bind(wxEVT_SCROLL_LINEUP, &HexBedStandaloneEditor::OnScroll, this);
    scroll_->Bind(wxEVT_SCROLL_LINEDOWN, &HexBedStandaloneEditor::OnScroll,
                  this);
    scroll_->Bind(wxEVT_SCROLL_PAGEUP, &HexBedStandaloneEditor::OnScroll, this);
    scroll_->Bind(wxEVT_SCROLL_PAGEDOWN, &HexBedStandaloneEditor::OnScroll,
                  this);
    scroll_->Bind(wxEVT_SCROLL_THUMBTRACK, &HexBedStandaloneEditor::OnScroll,
                  this);
    scroll_->Bind(wxEVT_SCROLL_THUMBRELEASE, &HexBedStandaloneEditor::OnScroll,
                  this);
    scroll_->Bind(wxEVT_SCROLL_CHANGED, &HexBedStandaloneEditor::OnScroll,
                  this);

    hexEdit_->Bind(wxEVT_MOUSEWHEEL, &HexBedStandaloneEditor::OnMouseWheel,
                   this);
    Layout();
    FullUpdate();
}

void HexBedStandaloneEditor::OnResize(wxSizeEvent& event) {
    // resizeUpdate();
    if (!timer_.IsRunning()) timer_.Start(RESIZE_TIMEOUT, wxTIMER_ONE_SHOT);
    event.Skip();
}

void HexBedStandaloneEditor::ReloadFile() { hexEdit_->Rebuffer(); }

void HexBedStandaloneEditor::OnResizeTimer(wxTimerEvent& event) {
    ResizeUpdate();
}

void HexBedStandaloneEditor::OnScroll(wxScrollEvent& event) { DisplayUpdate(); }

void HexBedStandaloneEditor::FullUpdate() {
    hexEdit_->SetFont(configFont());
    LayoutUpdate();
    hexEdit_->FullRefresh();
    Layout();
    Refresh();
}

void HexBedStandaloneEditor::OnEditorCopy() {}

void HexBedStandaloneEditor::OnUndoRedo() {}

void HexBedStandaloneEditor::LayoutUpdate() {
    group_ = config().groupSize;
    rowHeight_ = hexEdit_->GetLineHeight();
    ResizeUpdate();
}

void HexBedStandaloneEditor::ResizeUpdate() {
    unsigned g = std::max<unsigned>(group_, configUtfGroupSize());
    cols_ = hexEdit_->FitColumns(hexEdit_->GetClientSize().GetWidth());
    cols_ -= cols_ % g;
    if (!cols_) cols_ = g;
    hexEdit_->SetColumns(cols_);
    rows_ =
        std::max((hexEdit_->GetClientSize().GetHeight() + 1) / rowHeight_, 1U);
    hexEdit_->SetRows(rows_);
    hexEdit_->ResizeDone();
    FileSizeUpdate();
    DisplayUpdate();
}

void HexBedStandaloneEditor::Selected() {
    LayoutUpdate();
    hexEdit_->Selected();
}

void HexBedStandaloneEditor::FocusEditor() { hexEdit_->SetFocus(); }

void HexBedStandaloneEditor::FileSizeUpdate() {
    fsize_ = document().size();
    frows_ = fsize_ / config().hexColumns + rows_;
    scroll_->SetScrollbarLong(
        std::min<unsigned>(scroll_->GetThumbPositionLong(), frows_), rows_,
        frows_, rows_);
}

void HexBedStandaloneEditor::OnMouseWheel(wxMouseEvent& event) {
    if (event.GetWheelAxis() != wxMOUSE_WHEEL_VERTICAL) {
        event.Skip();
        return;
    }
    mouseDelta_ += event.GetWheelRotation();
    int delta, threshold = event.GetWheelDelta();
    delta =
        mouseDelta_ < 0 ? -(-mouseDelta_ / threshold) : mouseDelta_ / threshold;
    mouseDelta_ -= delta * threshold;
    if (delta != 0) {
        scroll_->SetThumbPositionLong(
            scroll_->GetThumbPositionLong() -
            delta * (event.IsPageScroll() ? rows_ : event.GetLinesPerAction()));
        DisplayUpdate();
    }
}

void HexBedStandaloneEditor::DisplayUpdate() {
    hexEdit_->SetOffset(scroll_->GetThumbPositionLong() * config().hexColumns);
}

void HexBedStandaloneEditor::BringOffsetToScreen(bufsize offset) {
    bufsize columns = config().hexColumns;
    bufsize range = rows_ * columns - 1;
    bufsize minRow = (offset - range) / columns + 1;
    bufsize maxRow = offset / columns;
    if (scroll_->GetThumbPositionLong() < minRow) {
        scroll_->SetThumbPositionLong(minRow);
        DisplayUpdate();
    } else if (scroll_->GetThumbPositionLong() > maxRow) {
        scroll_->SetThumbPositionLong(maxRow);
        DisplayUpdate();
    }
}

bool HexBedStandaloneEditor::ScrollLine(int dir) {
    auto old = scroll_->GetThumbPositionLong();
    scroll_->SetThumbPositionLong(scroll_->GetThumbPositionLong() + dir);
    auto now = scroll_->GetThumbPositionLong();
    if (old != now) {
        DisplayUpdate();
    }
    return old != now;
}

bool HexBedStandaloneEditor::ScrollPage(int dir) {
    auto old = scroll_->GetThumbPositionLong();
    scroll_->SetThumbPositionLong(scroll_->GetThumbPositionLong() +
                                  dir * (rows_ - 1));
    auto now = scroll_->GetThumbPositionLong();
    if (old != now) {
        DisplayUpdate();
    }
    return old != now;
}

bufsize HexBedStandaloneEditor::GetColumnCount() const {
    return hexEdit_->GetColumns();
}

void HexBedStandaloneEditor::SelectBytes(bufsize start, bufsize length,
                                         SelectFlags flags) {
    hexEdit_->SelectBytes(start, length, flags);
}

void HexBedStandaloneEditor::SelectAll(SelectFlags flags) {
    SelectBytes(0, document_->size(), flags);
}

void HexBedStandaloneEditor::SelectNone() { hexEdit_->SelectNone(); }

void HexBedStandaloneEditor::GetSelection(bufsize& start, bufsize& length,
                                          bool& text) {
    hexEdit_->GetSelection(start, length, text);
}

HexBedPeekRegion HexBedStandaloneEditor::PeekBufferAtCursor() {
    return hexEdit_->PeekBufferAtCursor();
}

void HexBedStandaloneEditor::OnCaretMoved() {
    AddPendingEvent(wxCommandEvent(HEX_CARET_EVENT));
}

void HexBedStandaloneEditor::OnSelectChanged() {
    AddPendingEvent(wxCommandEvent(HEX_SELECT_EVENT));
}

void HexBedStandaloneEditor::HintByteChanged(bufsize offset) {
    hexEdit_->HintByteChanged(offset);
    AddPendingEvent(wxCommandEvent(HEX_EDIT_EVENT));
}

void HexBedStandaloneEditor::HintBytesChanged(bufsize begin) {
    FileSizeUpdate();
    hexEdit_->HintBytesChanged(begin);
    AddPendingEvent(wxCommandEvent(HEX_EDIT_EVENT));
}

void HexBedStandaloneEditor::HintBytesChanged(bufsize begin, bufsize end) {
    hexEdit_->HintBytesChanged(begin, end);
    AddPendingEvent(wxCommandEvent(HEX_EDIT_EVENT));
}

void HexBedStandaloneEditor::DoCtrlCut() { hexEdit_->DoCtrlCut(); }

void HexBedStandaloneEditor::DoCtrlCopy() { hexEdit_->DoCtrlCopy(); }

void HexBedStandaloneEditor::DoCtrlPasteInsert() {
    hexEdit_->DoCtrlPasteInsert();
}

void HexBedStandaloneEditor::DoCtrlPasteOverwrite() {
    hexEdit_->DoCtrlPasteOverwrite();
}

void HexBedStandaloneEditor::DoCtrlUndo() { hexEdit_->DoCtrlUndo(); }

void HexBedStandaloneEditor::DoCtrlRedo() { hexEdit_->DoCtrlRedo(); }

};  // namespace ui
};  // namespace hexbed
