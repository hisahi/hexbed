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
// ui/editor.cc -- implementation for the HexBed editor class

#include "ui/editor.hh"

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

wxDEFINE_EVENT(HEX_EDIT_EVENT, wxCommandEvent);
wxDEFINE_EVENT(HEX_SELECT_EVENT, wxCommandEvent);

LongScrollBar::scroll_t LongScrollBar::GetPageSizeLong() const {
    return (GetPageSize() << shift_) | pageRem_;
}

LongScrollBar::scroll_t LongScrollBar::GetRangeLong() const {
    return (GetRangeLong() << shift_) | rangeRem_;
}

LongScrollBar::scroll_t LongScrollBar::GetThumbPositionLong() const {
    return (GetThumbPosition() << shift_) | posRem_;
}

LongScrollBar::scroll_t LongScrollBar::GetThumbSizeLong() const {
    return (GetThumbSize() << shift_) | thumbRem_;
}

void LongScrollBar::SetScrollbar(int position, int thumbSize, int range,
                                 int pageSize, bool refresh) {
    wxScrollBar::SetScrollbar(position, thumbSize, range, pageSize, refresh);
    shift_ = 0;
    posRem_ = thumbRem_ = rangeRem_ = pageRem_ = 0;
}

template <typename T, typename T2>
static constexpr T remShift(T val, T2 shift) {
    return val & ((1 << shift) - 1);
}

void LongScrollBar::SetScrollbarLong(scroll_t position, scroll_t thumbSize,
                                     scroll_t range, scroll_t pageSize,
                                     bool refresh) {
    if (pageSize > range) pageSize = range;
    if (position > range) position = range;
    if (thumbSize > pageSize) thumbSize = pageSize;

    scroll_t bw = std::bit_width(range);
    constexpr scroll_t mbw =
        std::bit_width<scroll_t>(std::numeric_limits<int>::max());
    shift_ = bw > mbw ? bw - mbw : 0;

    wxScrollBar::SetScrollbar(static_cast<int>(position >> shift_),
                              static_cast<int>(thumbSize >> shift_),
                              static_cast<int>(range >> shift_),
                              static_cast<int>(pageSize >> shift_), refresh);

    posRem_ = remShift(position, shift_);
    thumbRem_ = remShift(thumbSize, shift_);
    rangeRem_ = remShift(range, shift_);
    pageRem_ = remShift(pageSize, shift_);
}

void LongScrollBar::SetThumbPosition(int viewStart) {
    wxScrollBar::SetThumbPosition(viewStart);
    posRem_ = 0;
}

void LongScrollBar::SetThumbPositionLong(scroll_t viewStart) {
    wxScrollBar::SetThumbPosition(static_cast<int>(viewStart >> shift_));
    posRem_ = remShift(viewStart, shift_);
}

HexBedEditor::HexBedEditor(HexBedMainFrame* frame, wxWindow* parent,
                           HexBedContextMain* ctx, HexBedDocument&& document)
    : HexBedEditor(frame, parent, ctx,
                   std::make_shared<HexBedDocument>(std::move(document))) {}

HexBedEditor::HexBedEditor(HexBedMainFrame* frame, wxWindow* parent,
                           HexBedContextMain* ctx,
                           std::shared_ptr<HexBedDocument>&& document)
    : wxPanel(parent),
      frame_(frame),
      document_(std::move(document)),
      ctx_(ctx),
      timer_(this, wxID_ANY) {
    box_ = new wxBoxSizer(wxVERTICAL);
    SetSizer(box_);
    int gap = 4;
    grid_ = new wxFlexGridSizer(4, gap, gap);
    box_->Add(grid_, wxSizerFlags().Expand().Proportion(1).Border(wxALL, 0));

    scroller_ = new wxScrolledWindowFocusRedirect(this);
    scroller_->DisableKeyboardScrolling();

    rowTop_ = new wxStaticText(scroller_, wxID_ANY, "", wxDefaultPosition,
                               wxDefaultSize, wxST_NO_AUTORESIZE);
    hexEdit_ = new HexEditor(scroller_, this);
    colOffset_ = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition,
                                  wxDefaultSize, wxST_NO_AUTORESIZE);
    scroll_ = new LongScrollBar(this, wxID_ANY, wxDefaultPosition,
                                wxDefaultSize, wxSB_VERTICAL);

    scroller_->SetFocusTarget(hexEdit_);

    wxBoxSizer* sz = new wxBoxSizer(wxVERTICAL);
    scroller_->SetSizer(sz);
    sz->Add(rowTop_, wxSizerFlags().Expand().Border(wxTOP, gap));
    sz->Add(hexEdit_, wxSizerFlags().Expand().Proportion(1).Border(wxTOP, gap));
    rowTop_->FitInside();
    hexEdit_->FitInside();
    scroller_->Layout();

    grid_->AddSpacer(4);
    wxBoxSizer* pad = new wxBoxSizer(wxVERTICAL);
    shift_ = pad->AddSpacer(10);
    pad->Add(colOffset_,
             wxSizerFlags().Expand().Proportion(1).Border(wxTOP, gap * 2));
    pad->Layout();
    grid_->Add(pad, wxSizerFlags().Expand().Border(wxALL, gap));
    grid_->Add(scroller_, wxSizerFlags().Expand().Border(wxALL, gap));
    grid_->Add(scroll_, wxSizerFlags().Expand().Border(wxALL, gap));

    grid_->AddGrowableRow(0);
    grid_->AddGrowableCol(2);

    Bind(wxEVT_SIZE, &HexBedEditor::OnResize, this);
    Bind(wxEVT_TIMER, &HexBedEditor::OnResizeTimer, this);

    scroll_->Bind(wxEVT_SCROLL_TOP, &HexBedEditor::OnScroll, this);
    scroll_->Bind(wxEVT_SCROLL_BOTTOM, &HexBedEditor::OnScroll, this);
    scroll_->Bind(wxEVT_SCROLL_LINEUP, &HexBedEditor::OnScroll, this);
    scroll_->Bind(wxEVT_SCROLL_LINEDOWN, &HexBedEditor::OnScroll, this);
    scroll_->Bind(wxEVT_SCROLL_PAGEUP, &HexBedEditor::OnScroll, this);
    scroll_->Bind(wxEVT_SCROLL_PAGEDOWN, &HexBedEditor::OnScroll, this);
    scroll_->Bind(wxEVT_SCROLL_THUMBTRACK, &HexBedEditor::OnScroll, this);
    scroll_->Bind(wxEVT_SCROLL_THUMBRELEASE, &HexBedEditor::OnScroll, this);
    scroll_->Bind(wxEVT_SCROLL_CHANGED, &HexBedEditor::OnScroll, this);

    hexEdit_->Bind(wxEVT_MOUSEWHEEL, &HexBedEditor::OnMouseWheel, this);
    Layout();
    FullUpdate();
}

void HexBedEditor::OnResize(wxSizeEvent& event) {
    // resizeUpdate();
    if (!timer_.IsRunning()) timer_.Start(RESIZE_TIMEOUT, wxTIMER_ONE_SHOT);
    event.Skip();
}

void HexBedEditor::ReloadFile() { hexEdit_->Rebuffer(); }

void HexBedEditor::OnResizeTimer(wxTimerEvent& event) {
    if (!AutoFitUpdate()) ResizeUpdate();
}

void HexBedEditor::OnScroll(wxScrollEvent& event) {
    ScrollUpdate();
    DisplayUpdate();
}

void HexBedEditor::FullUpdate() {
    wxFont fnt = configFont();
    SetBackgroundColour(wxColour(config().backgroundColor));

    rowTop_->SetBackgroundColour(wxColour(config().backgroundColor));
    colOffset_->SetBackgroundColour(wxColour(config().backgroundColor));

    rowTop_->SetForegroundColour(wxColour(config().offsetColor));
    colOffset_->SetForegroundColour(wxColour(config().offsetColor));
    rowTop_->SetFont(fnt);
    colOffset_->SetFont(fnt);

    hexEdit_->SetFont(fnt);
    if (!AutoFitUpdate()) LayoutUpdate();
    hexEdit_->FullRefresh();
    Layout();
    Refresh();
}

void HexBedEditor::UpdateMenuEnabledSelect() {
    frame_->UpdateMenuEnabledSelect(*this);
}

void HexBedEditor::UpdateMenuEnabledClip() {
    frame_->UpdateMenuEnabledClip(*this);
}

void HexBedEditor::LayoutUpdate() {
    group_ = config().groupSize;
    if (!config().autoFit) {
        cols_ = config().hexColumns;
        cols_ -= cols_ % group_;
        if (!cols_) cols_ = group_;
    }
    if (config().showColumnTypes & 2) {  // if showing hex column
        std::ostringstream os;
        switch (config().offsetRadix) {
        case 8:
            os << std::oct;
            break;
        case 10:
            break;
        case 16:
            os << std::hex;
            if (config().uppercase)
                os << std::uppercase;
            else
                os << std::nouppercase;
            break;
        }
        os << std::setfill('0');
        os << "00";
        for (unsigned i = 1; i < cols_; ++i) {
            if (i % group_)
                os << "  ";
            else
                os << ' ' << std::setw(2) << i;
        }
        std::string s = os.str();
        rowTop_->SetLabel(s);
    } else
        rowTop_->SetLabel(wxEmptyString);
    hexEdit_->SetColumns(cols_);
    wxSize zeroSz = rowTop_->GetTextExtent("0");
    charWidth_ = zeroSz.GetWidth();
    rowHeight_ = zeroSz.GetHeight();
    ResizeUpdate();
    if (config().autoFit) {
        wxSize sz{hexEdit_->GetClientSize().GetWidth(),
                  static_cast<int>(rowHeight_)};
        rowTop_->SetMinClientSize(wxSize{1, sz.GetHeight()});
        rowTop_->SetClientSize(sz);
        rowTop_->SetVirtualSize(sz);
    } else {
        wxSize sz{hexEdit_->GetMinClientSize().GetWidth(),
                  static_cast<int>(rowHeight_)};
        rowTop_->SetMinClientSize(sz);
        rowTop_->SetVirtualSize(sz);
    }
    shift_->SetMinSize(1, rowHeight_);
}

bool HexBedEditor::AutoFitUpdate() {
    if (config().autoFit) {
        group_ = config().groupSize;
        unsigned colsOld_ = cols_;
        cols_ = hexEdit_->FitColumns(hexEdit_->GetClientSize().GetWidth());
        cols_ -= cols_ % group_;
        if (!cols_) cols_ = group_;
        if (cols_ != colsOld_) {
            hexEdit_->SetColumns(cols_);
            FileSizeUpdate();
            LayoutUpdate();
            return true;
        }
    }
    return false;
}

void HexBedEditor::ResizeUpdate() {
    // how many rows can we display now?
    rows_ =
        std::max((hexEdit_->GetClientSize().GetHeight() + 1) / rowHeight_, 1U);
    hexEdit_->SetRows(rows_);
    hexEdit_->ResizeDone();
    unsigned hCanvasSize = hexEdit_->GetMinClientSize().GetWidth();
    unsigned x = scroller_->GetViewStart().x;
    scroller_->SetScrollbars(
        charWidth_, 1, (hCanvasSize + charWidth_ - 1) / charWidth_, 0, x, 0);
    FileSizeUpdate();
    ScrollUpdate();
    DisplayUpdate();
}

void HexBedEditor::Selected() {
    LayoutUpdate();
    hexEdit_->Selected();
}

void HexBedEditor::SelectBytes(bufsize start, bufsize length,
                               SelectFlags flags) {
    hexEdit_->SelectBytes(start, length, flags);
}

void HexBedEditor::SelectNone() { hexEdit_->SelectNone(); }

void HexBedEditor::GetSelection(bufsize& start, bufsize& length, bool& text) {
    hexEdit_->GetSelection(start, length, text);
}

void HexBedEditor::OnSelectChanged() {
    AddPendingEvent(wxCommandEvent(HEX_SELECT_EVENT));
}

void HexBedEditor::HintByteChanged(bufsize offset) {
    hexEdit_->HintByteChanged(offset);
    AddPendingEvent(wxCommandEvent(HEX_EDIT_EVENT));
}

void HexBedEditor::HintBytesChanged(bufsize begin) {
    FileSizeUpdate();
    hexEdit_->HintBytesChanged(begin);
    AddPendingEvent(wxCommandEvent(HEX_EDIT_EVENT));
}

void HexBedEditor::HintBytesChanged(bufsize begin, bufsize end) {
    hexEdit_->HintBytesChanged(begin, end);
    AddPendingEvent(wxCommandEvent(HEX_EDIT_EVENT));
}

void HexBedEditor::FocusEditor() { hexEdit_->SetFocus(); }

template <typename T>
static T divRoundUp(T a, T b) {
    return (a + b - 1) / b;
}

static constexpr std::size_t log10Shift_ =
    std::bit_width(std::numeric_limits<std::size_t>::max()) > 30 ? 22 : 10;
template <typename T>
static constexpr T log10RoundDown_ = log10Shift_ == 22 ? T(1262611) : T(308);
template <typename T>
static constexpr T log10RoundUp_ = log10RoundDown_<T> + 1;

template <typename T>
static consteval std::size_t powersOf10Size() {
    return (std::bit_width(std::numeric_limits<T>::max()) * log10RoundUp_<T>) >>
           log10Shift_;
}

template <typename T>
static consteval T powersOf10HelperPower(T value) {
    T result = 1;
    for (T i = 0; i < value; ++i) result *= 10;
    return value;
}

template <typename T, std::size_t... I>
static consteval std::array<const T, powersOf10Size<T>()> powersOf10Helper_(
    std::index_sequence<I...>) {
    return std::array<const T, powersOf10Size<T>()>{
        powersOf10HelperPower<T>(I)...};
}

template <typename T, std::size_t I>
static consteval std::array<const T, powersOf10Size<T>()> powersOf10Helper() {
    return powersOf10Helper_<T>(std::make_index_sequence<I>{});
}

template <typename T>
static const std::array<const T, powersOf10Size<T>()> powersOf10 =
    powersOf10Helper<T, powersOf10Size<T>()>();

template <typename T>
static T ilog10ceil(T x) {
    T bw = std::bit_width(x);
    T l10 = (bw * log10RoundDown_<T>) >> log10Shift_;
    if (l10) --l10;
    while (powersOf10<bufsize>[l10] >= x) ++l10;
    return l10;
}

void HexBedEditor::FileSizeUpdate() {
    fsize_ = document().size();
    frows_ = fsize_ / cols_ + rows_;
    scroll_->SetScrollbarLong(
        std::min<unsigned>(scroll_->GetThumbPositionLong(), frows_), rows_,
        frows_, rows_);
    switch (config().offsetRadix) {
    case 8:
        offsetWidth_ = divRoundUp<bufsize>(std::bit_width(fsize_), 3);
        break;
    case 10:
        offsetWidth_ = ilog10ceil<bufsize>(fsize_);
        break;
    case 16:
        offsetWidth_ = divRoundUp<bufsize>(std::bit_width(fsize_), 4);
        break;
    }
    offsetWidth_ = std::max<unsigned>(offsetWidth_, 8);
    auto offSize =
        colOffset_->GetTextExtent(std::string(offsetWidth_ + 1, '0'));
    colOffset_->SetMinClientSize(wxSize(offSize.GetWidth(), 1));
    colOffset_->SetClientSize(
        wxSize(offSize.GetWidth(), offSize.GetHeight() * rows_));
    grid_->Layout();
    offsetBuf_.resize((offsetWidth_ + 1) * rows_ + 3, '\n');
    ScrollUpdate();
}

void HexBedEditor::OnMouseWheel(wxMouseEvent& event) {
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
        ScrollUpdate();
        DisplayUpdate();
    }
}

template <std::size_t M>
static void fastIntConv(char* out, const char* hex, unsigned n, bufsize off) {
    char* dp = out + n;
    while (n--) {
        *--dp = hex[off % M];
        off /= M;
    }
}

void HexBedEditor::ScrollUpdate() {
    unsigned i;
    char* dst_off = offsetBuf_.data();
    bufsize off = scroll_->GetThumbPositionLong() * cols_, xoff = off;
    static const char* hex = config().uppercase ? HEX_UPPERCASE : HEX_LOWERCASE;

    for (i = 0; i < rows_ && xoff <= fsize_; ++i) {
        switch (config().offsetRadix) {
        case 8:
            fastIntConv<8>(dst_off, hex, offsetWidth_, xoff);
            break;
        case 10:
            fastIntConv<10>(dst_off, hex, offsetWidth_, xoff);
            break;
        case 16:
            fastIntConv<16>(dst_off, hex, offsetWidth_, xoff);
            break;
        }
        dst_off += offsetWidth_;
        *dst_off++ = '\n';
        xoff += cols_;
    }
    HEXBED_ASSERT(i);
    if (!i) return;
    dst_off[-1] = '\0';
    try {
        colOffset_->SetLabel(offsetBuf_.data());
    } catch (...) {
    }
}

void HexBedEditor::DisplayUpdate() {
    hexEdit_->SetOffset(scroll_->GetThumbPositionLong() * cols_);
}

void HexBedEditor::BringOffsetToScreen(bufsize offset) {
    bufsize columns = cols_;
    bufsize range = rows_ * columns - 1;
    bufsize minRow = (offset - range) / columns + 1;
    bufsize maxRow = offset / columns;
    if (scroll_->GetThumbPositionLong() < minRow) {
        scroll_->SetThumbPositionLong(minRow);
        ScrollUpdate();
        DisplayUpdate();
    } else if (scroll_->GetThumbPositionLong() > maxRow) {
        scroll_->SetThumbPositionLong(maxRow);
        ScrollUpdate();
        DisplayUpdate();
    }
}

void HexBedEditor::UpdateStatusBar() { hexEdit_->UpdateStatusBar(); }

void HexBedEditor::SetStatusBar(wxStatusBar* sbar) {
    hexEdit_->SetStatusBar(sbar);
}

bool HexBedEditor::ScrollLine(int dir) {
    auto old = scroll_->GetThumbPositionLong();
    scroll_->SetThumbPositionLong(scroll_->GetThumbPositionLong() + dir);
    auto now = scroll_->GetThumbPositionLong();
    if (old != now) {
        ScrollUpdate();
        DisplayUpdate();
    }
    return old != now;
}

bool HexBedEditor::ScrollPage(int dir) {
    auto old = scroll_->GetThumbPositionLong();
    scroll_->SetThumbPositionLong(scroll_->GetThumbPositionLong() +
                                  dir * (rows_ - 1));
    auto now = scroll_->GetThumbPositionLong();
    if (old != now) {
        ScrollUpdate();
        DisplayUpdate();
    }
    return old != now;
}

};  // namespace ui
};  // namespace hexbed
