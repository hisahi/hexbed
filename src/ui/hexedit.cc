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
// ui/hexedit.cc -- impl for the hex editor class

#include "ui/hexedit.hh"

#include <cstring>
#include <new>
#include <utility>

#include "app/config.hh"
#include "common/charconv.hh"
#include "common/format.hh"
#include "common/hexconv.hh"
#include "common/logger.hh"
#include "common/memory.hh"
#include "common/specs.hh"

namespace hexbed {
namespace ui {

constexpr int SCROLL_SPEED = 100;
constexpr int SCROLL_DIVISOR = 128;

wxColor HexEditor::backColor_;
wxColor HexEditor::textColor_;
wxColor HexEditor::textNPColor_;
wxColor HexEditor::oddColor_;
wxColor HexEditor::evenColor_;
wxColor HexEditor::alignColor_;
wxColor HexEditor::selBgColor_;
wxColor HexEditor::selFgColor_;
wxColor HexEditor::selBg2Color_;
wxColor HexEditor::selFg2Color_;

HexEditor::HexEditor(wxWindow* parent, HexEditorParent* editor)
    : wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
               wxWANTS_CHARS | wxBORDER_NONE),
      parent_(editor),
      caret_(this, 1, 1),
      timer_(this, wxID_ANY) {
    SetMinClientSize(wxSize(1, 1));
    Bind(wxEVT_CHAR, &HexEditor::OnChar, this);
    Bind(wxEVT_PAINT, &HexEditor::OnPaint, this);
    Bind(wxEVT_SIZE, &HexEditor::OnResize, this);
    Bind(wxEVT_SET_FOCUS, &HexEditor::OnFocus, this);
    Bind(wxEVT_KILL_FOCUS, &HexEditor::OnBlur, this);
    Bind(wxEVT_SHOW, &HexEditor::OnShow, this);
    Bind(wxEVT_ERASE_BACKGROUND, &HexEditor::OnEraseBackground, this);
    Bind(wxEVT_LEFT_DOWN, &HexEditor::OnLMouseDown, this);
    Bind(wxEVT_LEFT_UP, &HexEditor::OnLMouseUp, this);
    Bind(wxEVT_RIGHT_DOWN, &HexEditor::OnRMouseDown, this);
    Bind(wxEVT_MOTION, &HexEditor::OnMouseMove, this);
    Bind(wxEVT_SET_CURSOR, &HexEditor::OnSetCursor, this);
    Bind(wxEVT_TIMER, &HexEditor::OnScrollTimer, this);
    GetDC();
    UpdateCaret(0);
    SetDoubleBuffered(true);
}

HexEditor::~HexEditor() {
    if (surface_) delete surface_;
    if (dc_) delete dc_;
}

bufsize HexEditor::Buffer(bufsize beg, bufsize end, bufsize off) {
    if (end <= beg) return 0;
    return document().read(off, bytespan(buffer_.data() + beg, end - beg));
}

void HexEditor::SetOffset(bufsize offset) {
    if (off_ != offset) {
        bufsize oo = off_;
        byte* p = buffer_.data();
        if (offset < off_) {
            bufsize diff = offset - off_;
            if (diff < bufn_) {
                memCopyBack(p + diff, p, bufn_ - diff);
                bufn_ = (bufn_ - diff) + Buffer(0, diff, off_);
            } else
                bufn_ =
                    Buffer(0, std::min<bufsize>(bufc_, (off_ + bufn_) - offset),
                           offset);
        } else if (offset > off_) {
            bufsize diff = off_ - offset;
            if (diff < bufn_) {
                memCopy(p, p + diff, bufn_ - diff);
                bufn_ = diff + Buffer(bufn_ - diff, bufn_, off_ + bufn_ - diff);
            } else
                bufn_ = Buffer(0, bufn_, offset);
        }
        off_ = offset;
        if (seldown_) {
            dragPoint_.offset += off_ - oo;
            UpdateCaret(dragPoint_.offset, false);
            UpdateSelectionDrag();
        } else {
            UpdateCaret(cur_, false);
            if (seln_) UpdateSelectionDrag();
        }
        RedrawLite();
    }
}

void HexEditor::HideCaret() { caret_.Hide(); }

void HexEditor::ShowCaret() { caret_.Show(); }

void HexEditor::UpdateCaret() {
    UpdateCaret(cur_);
    if (seln_) UpdateSelectionDrag();
}

void HexEditor::UpdateCaret(bufsize cur, bool redraw, bool eol) {
    bool changed = cur_ != cur;
    bufsize off2 = off_ + rows_ * columns_;
    if (bufn_ < bufc_) {
        if (cur >= off_) cur = std::min(cur, off_ + bufn_);
        if (sel_ >= off_) sel_ = std::min(sel_, off_ + bufn_);
        if (seln_) {
            seln_ = std::min(seln_, (off_ + bufn_) - sel_);
            parent_->OnSelectChanged();
        }
    }
    bool shouldBeVisible = off_ <= cur && cur < off2;
    int oldRow = caretshow_ ? (cur_ - off_) / columns_ : -1;
    int newRow = shouldBeVisible ? (cur - off_) / columns_ : -1;
    if (eol && newRow) --newRow;
    if (careteol_ && !eol) --oldRow;
    careteol_ = eol;
    if (shouldBeVisible) {
        if (careteol_) {
            caret_.Move((curtext_ ? textX_ + charWidth_ * columns_
                                  : GetColumnX(columns_) - charWidth_) -
                            1,
                        newRow * lineHeight_);
        } else {
            unsigned newColumn = (cur - off_) % columns_;
            caret_.Move((curtext_ ? textX_ + charWidth_ * newColumn
                                  : GetColumnX(newColumn) +
                                        (curnibble_ ? charWidth_ : 0)) -
                            1,
                        newRow * lineHeight_);
        }
    }
    if (caretshow_ != shouldBeVisible) {
        if (shouldBeVisible) {
            if (HasFocus()) ShowCaret();
        } else
            HideCaret();
        caretshow_ = shouldBeVisible;
    }
    cur_ = cur;
    if (redraw) {
        if (newRow != oldRow) RedrawRow(oldRow);
        RedrawRow(newRow);
    }
    UpdateStatusBar();
    if (changed) parent_->OnCaretMoved();
}

void HexEditor::Rebuffer() {
    bufn_ = Buffer(0, bufc_, off_);
    Redraw();
}

void HexEditor::SetRows(unsigned rows) {
    size_t oldSize = bufc_, newSize;
    rows_ = rows;
    bufviewable_ = rows_ * columns_;
    bufc_ = newSize = bufviewable_ + MAX_LOOKAHEAD;
    buffer_.resize(newSize, 0);
    if (oldSize == bufn_ && newSize > oldSize)
        bufn_ = oldSize + Buffer(oldSize, newSize, off_ + oldSize);
    Redraw();
}

void HexEditor::SetColumns(unsigned columns) {
    size_t oldSize = bufc_, newSize;
    columns_ = columns;
    unsigned gs = static_cast<unsigned>(config().groupSize);
    group_ = std::bit_width(gs) - 1;
    HEXBED_ASSERT(columns_ > 0 && (!group_ || !(columns_ % gs)));
    bufviewable_ = rows_ * columns_;
    bufc_ = newSize = bufviewable_ + MAX_LOOKAHEAD;
    buffer_.resize(newSize, 0);
    if (oldSize == bufn_ && newSize > oldSize)
        bufn_ = oldSize + Buffer(oldSize, newSize, off_ + oldSize);
    Redraw();
}

void HexEditor::GetDC() {
    auto* oldSurface = surface_;
    auto* oldDc = dc_;
    try {
        wxSize sz = GetSize();
        sz.IncTo(wxSize(1, 1));
        surface_ = new wxBitmap(sz);
        if (oldSurface) delete oldSurface;
    } catch (const std::bad_alloc& e) {
        if (!oldSurface) throw;
        surface_ = oldSurface;
    }
    try {
        dc_ = new wxMemoryDC();
        if (oldDc) delete oldDc;
    } catch (const std::bad_alloc& e) {
        if (!oldDc) throw;
        dc_ = oldDc;
    }
    dc_->SelectObject(*surface_);
    InitDraw();
}

void HexEditor::InitConfig() {
    backColor_ = wxColor(config().backgroundColor);
    textColor_ = wxColor(config().textColor);
    textNPColor_ = wxColor(config().textNonprintableColor);
    oddColor_ = wxColor(config().hexColorOdd);
    evenColor_ = wxColor(config().hexColorEven);
    alignColor_ = wxColor(config().alignColor);
    selBgColor_ = wxColor(config().selectBackgroundColor);
    selFgColor_ = wxColor(config().selectForegroundColor);
    selBg2Color_ = wxColor(config().selectAltBackgroundColor);
    selFg2Color_ = wxColor(config().selectAltForegroundColor);
}

unsigned HexEditor::FitColumns(wxCoord width) {
    // how many columns can we fit in width pixels?
    /* unsigned w = byteWidth_ * (c << group_)
                  + charWidth_ * ((c << group_) + c + 3); */
    if (width < 3 * charWidth_) return group_;
    wxCoord w = width - 3 * charWidth_;
    wxCoord c = w / (((byteWidth_ + charWidth_) << group_) + charWidth_);
    c &= ~((1 << group_) - 1);
    return std::min(std::max(group_, static_cast<unsigned>(c)), MAX_COLUMNS);
}

unsigned HexEditor::GetLineHeight() { return lineHeight_; }

bool HexEditor::TestVisibility() const noexcept { return IsShownOnScreen(); }

void HexEditor::QueueRefresh() { waitRedraw_ = true; }

void HexEditor::WhenVisible() {
    if (waitRedraw_) {
        Redraw();
        waitRedraw_ = false;
    }
}

void HexEditor::OnEditorCopy() { parent_->OnEditorCopy(); }

void HexEditor::InitDraw() {
    dc_->SetFont(GetFont());
    hasText_ = config().showColumnTypes & 1;
    hasHex_ = config().showColumnTypes & 2;
    charWidth_ = dc_->GetTextExtent("0").GetWidth();
    byteWidth_ = dc_->GetTextExtent("00").GetWidth();
    lineHeight_ = dc_->GetMultiLineTextExtent("a\nb").GetHeight() -
                  dc_->GetTextExtent("b").GetHeight();
    char rowText[MAX_COLUMNS + 1];
    unsigned columns = columns_;
    for (unsigned i = 0; i < columns; ++i) rowText[i] = '.';
    rowText[columns_] = 0;
    textWidth_ = dc_->GetTextExtent(rowText).GetWidth();
    if (hasHex_) {
        hexMaxX_ = GetColumnX(columns);
        textX_ = hexMaxX_ + 2 * charWidth_;
    } else {
        textX_ = hexMaxX_ = 0;
    }
    textMaxX_ = hasText_ ? textX_ + textWidth_ : textX_;
    wxCoord width = textMaxX_ + charWidth_;
    SetMinClientSize(wxSize(config().autoFit ? byteWidth_ : width, 1));
    SetVirtualSize(wxSize{width, static_cast<int>(lineHeight_ * rows_)});
    caret_.SetSize(2, lineHeight_);
}

static wxString rowTextT = " ";

template <bool selectCheck>
HEXBED_INLINE void HexEditor::DrawRow_(const char* hex, const byte* si,
                                       wxCoord y, bufsize off) {
    char rowHex[4];
    char32_t rowText[MAX_COLUMNS + 1];
    auto wch = rowTextT.GetWritableChar(0);
    bool selected = false, sel, sp;
    bufsize sele = sel_ + seln_ - 1;
    bufsize offe = off + columns_, eof = off_ + bufn_;
    unsigned j,
        g = 1 << group_, gm = g - 1,
        caret = careteol_
                    ? (cur_ == offe ? columns_ : static_cast<unsigned>(-1))
                : off <= cur_ && cur_ < offe ? cur_ - off
                                             : static_cast<unsigned>(-1);
    rowHex[2] = rowHex[3] = 0;
    wxCoord x = 0;
    dc_->SetTextBackground(backColor_);
    if (hasHex_) {
        for (j = 0; j < columns_ && off + j < eof; ++j) {
            sp = j && !(j & gm);
            if (sp) x += charWidth_;
            if constexpr (selectCheck) sel = sel_ <= off + j && off + j <= sele;
            byte b = *si++;
            fastByteConv(rowHex, hex, b);
            rowText[j] = sbcs.toPrintableChar(b);
            if constexpr (selectCheck) {
                if (sel && !selected) {
                    dc_->SetTextBackground(curtext_ ? selBg2Color_
                                                    : selBgColor_);
                    dc_->SetTextForeground(curtext_ ? selFg2Color_
                                                    : selFgColor_);
                    rowHex[2] = ' ';
                } else if (!sel) {
                    if (selected) dc_->SetTextBackground(backColor_);
                    dc_->SetTextForeground((j >> group_) & 1 ? evenColor_
                                                             : oddColor_);
                    rowHex[2] = 0;
                }
                if (sel && off + j == sele) rowHex[2] = 0;
                selected = sel;
            } else
                dc_->SetTextForeground((j >> group_) & 1 ? evenColor_
                                                         : oddColor_);
            dc_->DrawText(rowHex, x, y);
            x += byteWidth_;
        }
    } else {
        HEXBED_ASSERT(hasText_);
        for (j = 0; j < columns_ && off + j < eof; ++j) {
            byte b = *si++;
            rowText[j] = sbcs.toPrintableChar(b);
        }
    }
    rowText[j] = 0;
    if (hasText_) {
        x = textX_;
        selected = false;
        if constexpr (selectCheck) dc_->SetTextBackground(backColor_);
        for (j = 0; j < columns_ && off + j < eof; ++j) {
            bool printable;
            if constexpr (selectCheck) sel = sel_ <= off + j && off + j <= sele;
            printable = !!rowText[j];
            if (!printable)
                wch = '.';
            else if (std::numeric_limits<wchar_t>::max() >= 0x110000UL ||
                     rowText[j] < 0x10000)
                wch = rowText[j];
            else
                wch = 0xFFFDU;
            if (!selectCheck || !sel) {
                dc_->SetTextForeground(printable ? textColor_ : textNPColor_);
            }
            if constexpr (selectCheck) {
                if (sel != selected) {
                    if (sel) {
                        dc_->SetTextBackground(!curtext_ ? selBg2Color_
                                                         : selBgColor_);
                        dc_->SetTextForeground(!curtext_ ? selFg2Color_
                                                         : selFgColor_);
                    } else {
                        dc_->SetTextBackground(backColor_);
                        dc_->SetTextForeground(printable ? textColor_
                                                         : textNPColor_);
                    }
                    selected = sel;
                }
            }
            dc_->DrawText(rowTextT, x, y);
            x += charWidth_;
        }
    }
    if (caret <= columns_ && !seln_ && hasText_ && hasHex_) {
        dc_->SetPen(wxPen(alignColor_, 1, wxPENSTYLE_DOT));
        dc_->SetBrush(wxNullBrush);
        if (curtext_)
            dc_->DrawRectangle(careteol_ ? GetColumnX(columns_) - charWidth_
                                         : GetColumnX(caret),
                               y, byteWidth_, lineHeight_);
        else
            dc_->DrawRectangle(textX_ + charWidth_ * caret, y, charWidth_,
                               lineHeight_);
    }
}

void HexEditor::DrawRow(bufsize r) {
    bufsize c = r * columns_;
    if (seln_)
        DrawRow_<true>(config().uppercase ? HEX_UPPERCASE : HEX_LOWERCASE,
                       buffer_.data() + c, r * lineHeight_, off_ + c);
    else
        DrawRow_<false>(config().uppercase ? HEX_UPPERCASE : HEX_LOWERCASE,
                        buffer_.data() + c, r * lineHeight_, off_ + c);
}

void HexEditor::RedrawRow(int r) {
    if (r >= 0) {
        const std::lock_guard<std::mutex> lock(mutex_);
        wxRect region(0, r * lineHeight_, GetClientSize().GetWidth(),
                      lineHeight_);
        dc_->SetPen(wxNullPen);
        dc_->SetBrush(wxBrush(backColor_));
        dc_->DrawRectangle(region);
        DrawRow(r);
        RefreshRect(region);
    }
}

void HexEditor::DrawFull() {
    static_assert(MAX_COLUMNS > 0, "must have at least one column");
    const std::lock_guard<std::mutex> lock(mutex_);
    const char* hex = config().uppercase ? HEX_UPPERCASE : HEX_LOWERCASE;
    const byte* si = buffer_.data();

    dc_->SetFont(GetFont());
    dc_->SetBackground(wxBrush(backColor_));
    dc_->SetBackgroundMode(wxSOLID);
    dc_->Clear();

    bufsize off = 0;
    wxCoord y = 0;
    for (unsigned i = 0; i < rows_ && off < bufn_;
         ++i, y += lineHeight_, off += columns_) {
        if (seln_)
            DrawRow_<true>(hex, si + off, y, off_ + off);
        else
            DrawRow_<false>(hex, si + off, y, off_ + off);
    }
}

void HexEditor::RedrawLite() {
    if (!TestVisibility()) {
        QueueRefresh();
        return;
    }
    DrawFull();
    Refresh();
    UpdateCaret();
}

void HexEditor::Redraw() {
    HEXBED_ASSERT(hasText_ || hasHex_);
    curtext_ = (curtext_ && hasText_) || !hasHex_;
    RedrawLite();
    UpdateCaret();
}

void HexEditor::FullRefresh() {
    InitDraw();
    Redraw();
}

void HexEditor::OnEraseBackground(wxEraseEvent& e) {}

void HexEditor::SetStatusBar(wxStatusBar* sbar) {
    sbar_ = sbar;
    UpdateStatusBar();
}

inline unsigned convertRow(bufsize off, bufsize base, bufsize rows,
                           bufsize cols) {
    bufsize vis = rows * cols;
    if (off < base) return 0;
    if (off >= base + vis) return rows - 1;
    return (off - base) / cols;
}

void HexEditor::SelectBytes(bufsize start, bufsize length, SelectFlags flags) {
    bufsize end = start + length;
    sel_ = start;
    seln_ = length;
    selst_ = flags.isCaretAtEnd() ? start : end;
    seldown_ = false;
    UpdateCaret(flags.isCaretAtEnd() ? end : start, false,
                flags.isCaretAtEnd() && end && !(end % columns_));
    if (flags.isHighlightEnd()) parent_->BringOffsetToScreen(end);
    if (flags.isHighlightBegin()) parent_->BringOffsetToScreen(start);
    UpdateStatusBar();
    parent_->OnSelectChanged();
    RedrawLite();
}

void HexEditor::SelectNone() {
    seldown_ = false;
    Deselect();
}

void HexEditor::GetSelection(bufsize& start, bufsize& length, bool& text) {
    start = seln_ ? sel_ : cur_;
    length = seldown_ ? 0 : seln_;
    text = curtext_;
}

HexBedPeekRegion HexEditor::PeekBufferAtCursor() {
    const byte* buf = buffer_.data();
    const byte* bufend = buf + bufn_;
    bufsize caret = cur_;
    if (caret < off_ || caret >= off_ + bufviewable_)
        return HexBedPeekRegion{&document(), caret, const_bytespan{}};
    const byte* bufat = buf + (caret - off_);
    return HexBedPeekRegion{&document(), caret, const_bytespan{bufat, bufend}};
}

void HexEditor::HintByteChanged(bufsize offset) {
    if (offset < off_ || offset >= off_ + bufn_) return;
    if (!TestVisibility()) {
        QueueRefresh();
        return;
    }
    bufsize sub = offset - off_;
    Buffer(sub, sub + 1, offset);
    unsigned r = convertRow(offset, off_, rows_, columns_);
    RedrawRow(r);
}

void HexEditor::HintBytesChanged(bufsize begin) {
    if (begin > off_ + bufn_) return;
    if (!TestVisibility()) {
        QueueRefresh();
        return;
    }
    Rebuffer();
    unsigned r = convertRow(begin, off_, rows_, columns_);
    for (unsigned i = r; i < rows_; ++i) RedrawRow(i);
}

void HexEditor::HintBytesChanged(bufsize begin, bufsize end) {
    if (end < off_ || begin > off_ + bufn_) return;
    if (!TestVisibility()) {
        QueueRefresh();
        return;
    }
    bufsize sub = begin >= off_ ? begin - off_ : 0;
    bufsize subend = std::min(bufc_, end + 1 - off_);
    Buffer(sub, subend, off_ + sub);
    unsigned r = convertRow(begin, off_, rows_, columns_);
    unsigned r2 = convertRow(end, off_, rows_, columns_);
    for (unsigned i = r; i <= r2; ++i) RedrawRow(i);
}

void HexEditor::DoReplaceByte(byte v) {
    if (seln_) Deselect();
    document().impose(cur_, v);
    // HintByteChanged(cur_); roundtrip
}

void HexEditor::DoInsertByte(byte v) {
    if (seln_) {
        cur_ = sel_;
        document().replace(sel_, seln_, 1, v);
        Deselect();
        parent_->OnCaretMoved();
    } else {
        document().insert(cur_, v);
    }
    // HintBytesChanged(cur_); roundtrip
}

void HexEditor::DoRemoveByte(bufsize offset) {
    document().remove(offset);
    // HintBytesChanged(offset); roundtrip
}

void HexEditor::DoRemoveBytes(bufsize offset, bufsize len) {
    document().remove(offset, len);
    // HintBytesChanged(offset); roundtrip
}

void HexEditor::HandleHexInput(char c) {
    if (document().readOnly()) return;
    int v = hexDigitToNum(c);
    if (v < 0) return;
    parent_->BringOffsetToScreen(cur_);
    if (curnibble_) {
        DoReplaceByte((buffer_[cur_ - off_] & 0xF0) | v);
        curnibble_ = false;
        UpdateCaret(cur_ + 1);
        return;
    }
    if (context().state.insert || cur_ == off_ + bufn_) {
        DoInsertByte(v << 4);
    } else {
        DoReplaceByte((buffer_[cur_ - off_] & 0x0F) | (v << 4));
    }
    curnibble_ = true;
    UpdateCaret();
}

void HexEditor::HandleTextInput(byte b) {
    if (document().readOnly()) return;
    parent_->BringOffsetToScreen(cur_);
    if (context().state.insert || cur_ == off_ + bufn_) {
        DoInsertByte(b);
    } else {
        DoReplaceByte(b);
    }
    UpdateCaret(cur_ + 1);
}

void HexEditor::OnChar(wxKeyEvent& e) {
    if (!HasFocus()) {
        e.Skip();
        return;
    }
    auto kc = e.GetKeyCode();
    switch (kc) {
    case WXK_LEFT:
        if (curnibble_ && !e.ShiftDown()) {
            curnibble_ = false;
            UpdateCaret();
            break;
        }
        if (cur_ > 0) {
            UpdateCaretSelect(e.ShiftDown(), cur_ - 1);
            parent_->BringOffsetToScreen(cur_);
        }
        break;
    case WXK_RIGHT: {
        bufsize sz = document().size();
        if (curnibble_) curnibble_ = false;
        if (cur_ < sz) {
            UpdateCaretSelect(e.ShiftDown(), cur_ + 1);
            parent_->BringOffsetToScreen(cur_);
        }
        break;
    }
    case WXK_UP:
        if (cur_ >= columns_ + (careteol_ ? 1 : 0)) {
            UpdateCaretSelect(e.ShiftDown(), cur_ - columns_, true, careteol_);
            parent_->BringOffsetToScreen(cur_);
        }
        break;
    case WXK_DOWN: {
        bufsize sz = document().size();
        bufsize cur = std::min<bufsize>(cur_ + columns_, sz);
        if (cur_ != cur) {
            UpdateCaretSelect(e.ShiftDown(), cur, true,
                              careteol_ && !(cur_ % columns_));
            parent_->BringOffsetToScreen(cur_);
        }
        break;
    }
    case WXK_HOME:
        if (e.ControlDown()) {
            UpdateCaretSelect(e.ShiftDown(), 0);
            parent_->BringOffsetToScreen(cur_);
        } else {
            UpdateCaretSelect(e.ShiftDown(), careteol_
                                                 ? cur_ - columns_
                                                 : cur_ - cur_ % columns_);
            parent_->BringOffsetToScreen(cur_);
        }
        break;
    case WXK_END:
        if (e.ControlDown()) {
            UpdateCaretSelect(e.ShiftDown(), document().size());
            parent_->BringOffsetToScreen(cur_);
        } else if (careteol_) {
            parent_->BringOffsetToScreen(cur_);
        } else {
            bufsize sz = document().size();
            bufsize newcur = cur_ - cur_ % columns_ + columns_;
            UpdateCaretSelect(e.ShiftDown(), std::min(newcur, sz), true,
                              newcur <= sz);
            parent_->BringOffsetToScreen(cur_);
        }
        break;
    case WXK_PAGEUP: {
        parent_->BringOffsetToScreen(cur_);
        bufsize shift = (rows_ - 1) * columns_;
        bufsize cur = cur_ > shift ? cur_ - shift : 0;
        UpdateCaretSelect(e.ShiftDown(), cur);
        parent_->ScrollPage(-1);
        break;
    }
    case WXK_PAGEDOWN: {
        parent_->BringOffsetToScreen(cur_);
        bufsize cur =
            std::min(document().size(), cur_ + (rows_ - 1) * columns_);
        UpdateCaretSelect(e.ShiftDown(), cur);
        parent_->ScrollPage(1);
        break;
    }
    case WXK_BACK:
    case WXK_DELETE:
        if (document().readOnly()) return;
        if (seln_) {
            cur_ = sel_;
            DoRemoveBytes(sel_, seln_);
            Deselect();
            UpdateCaret();
            parent_->OnCaretMoved();
        } else if (kc == WXK_BACK && curnibble_) {
            DoRemoveByte(cur_);
            curnibble_ = false;
            UpdateCaret();
        } else if (kc == WXK_BACK && cur_ > 0) {
            DoRemoveByte(--cur_);
            UpdateCaret();
        } else if (kc == WXK_DELETE) {
            bufsize sz = document().size();
            if (cur_ < sz) DoRemoveByte(cur_);
        }
        break;
    case WXK_SPACE: {
        if (curtext_) goto textinput;
        bufsize sz = document().size();
        if (cur_ >= sz) break;
        Deselect();
        if (!curnibble_) {
            curnibble_ = true;
            UpdateCaret();
        } else {
            curnibble_ = false;
            UpdateCaret(cur_ + 1);
        }
        break;
    }
    // clang-format off
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case 'a': case 'A':
    case 'b': case 'B':
    case 'c': case 'C':
    case 'd': case 'D':
    case 'e': case 'E':
    case 'f': case 'F':
        // clang-format on
        if (!curtext_ && !e.AltDown()) {
            HandleHexInput(kc);
            break;
        }
        [[fallthrough]];
    default:
    textinput:
        if (curtext_ && !e.ControlDown() && !e.AltDown()) {
            // text input...?
            wxChar u = e.GetUnicodeKey();
            if (u != WXK_NONE) {
                int v = sbcs.fromChar(u);
                if (v >= 0) {
                    HandleTextInput((byte)v);
                    break;
                }
            }
        }
        e.Skip();
    }
}

wxCoord HexEditor::GetColumnX(unsigned c) const noexcept {
    return byteWidth_ * c + charWidth_ * (c >> group_);
}

void HexEditor::GetColumnFromX(wxCoord x, unsigned& div,
                               unsigned& rem) const noexcept {
    unsigned long col = x << group_;
    col /= (byteWidth_ << group_) + charWidth_;
    div = col;
    rem = x - GetColumnX(div);
}

bool HexEditor::HitPos(wxCoord x, wxCoord y, HitPoint& p, bool drag) {
    bool hitInside = x < hexMaxX_ || (textX_ <= x && x < textMaxX_);
    if (hitInside) {
        unsigned row = y / lineHeight_;
        if (seldown_ && rows_ > 1) row = std::min<unsigned>(row, rows_ - 1);
        if (x < hexMaxX_) {
            bufsize maxs = bufn_ < bufc_ ? off_ + bufn_ : BUFSIZE_MAX;
            int xo = x + (charWidth_ / 2);
            unsigned div, rem;
            GetColumnFromX(xo, div, rem);
            unsigned sub = std::min<unsigned>(columns_ - 1, div);
            unsigned cell = std::min(2U, rem / charWidth_);
            p.offset = off_ + row * columns_ + sub;
            if (p.offset >= maxs) {
                p.offset = maxs;
                cell = 0;
            }
            p.text = false;
            if (cell == 1 && drag) {
                p.nibble = false;
                ++cell;
            }
            p.nibble = cell == 1;
            if (cell == 2) {
                p.eol = sub == columns_ - 1;
                ++p.offset;
            } else
                p.eol = false;
        } else if (x < textMaxX_) {
            int xo = x - textX_ + (charWidth_ / 2);
            unsigned sub = static_cast<unsigned>(xo / charWidth_);
            p.offset = off_ + row * columns_ + sub;
            p.text = true;
            p.nibble = false;
            p.eol = sub == columns_;
        }
    }
    return hitInside;
}

void HexEditor::StartSelection(bufsize offset) {
    if (!seln_) selst_ = offset;
}

void HexEditor::Deselect() {
    bool hadSelect = seln_ != 0;
    sel_ = seln_ = 0;
    if (hadSelect) {
        Redraw();
        UpdateStatusBar();
        parent_->OnSelectChanged();
    }
}

void HexEditor::UpdateCaretSelect(bool shift, bufsize cur, bool redraw,
                                  bool eol) {
    if (shift) {
        StartSelection(cur_);
        UpdateCaret(cur, redraw, eol);
        UpdateSelectionDrag();
        parent_->OnSelectChanged();
    } else {
        UpdateCaret(cur, redraw && !seln_, eol);
        Deselect();
    }
}

void HexEditor::OnLMouseDown(wxMouseEvent& event) {
    HitPoint hit;
    if (HitPos(event.GetX(), event.GetY(), hit, false)) {
        seldown_ = true;
        selst_ = hit.offset;
        curtext_ = hit.text;
        curnibble_ = hit.nibble;
        dragPoint_ = hit;
        Deselect();
        UpdateCaret(hit.offset, true, hit.eol);
    }
    event.Skip();
}

void HexEditor::OnLMouseUp(wxMouseEvent& event) {
    seldown_ = false;
    timer_.Stop();
    parent_->OnSelectChanged();
    event.Skip();
}

void HexEditor::OnRMouseDown(wxMouseEvent& event) {
    HitPoint hit;
    if (HitPos(event.GetX(), event.GetY(), hit, false)) {
        if (std::exchange(curtext_, hit.text) != hit.text) Redraw();
    }
}

void HexEditor::OnMouseMove(wxMouseEvent& event) {
    if (!(seldown_ && event.Dragging())) return;
    wxCoord buf = static_cast<wxCoord>(lineHeight_ / 2);
    HitPoint hit;
    if (event.GetY() < buf) {
        int speed = buf - event.GetY();
        speed *= speed;
        speed /= lineHeight_ * SCROLL_DIVISOR;
        if (!speed) speed = 1;
        scrollDir_ = -speed;
        if (!timer_.IsRunning()) {
            timer_.Start(SCROLL_SPEED);
            timer_.Notify();
        }
    } else if (event.GetY() > GetClientSize().GetHeight() - buf) {
        int speed = event.GetY() - (GetClientSize().GetHeight() - buf);
        speed *= speed;
        speed /= lineHeight_ * SCROLL_DIVISOR;
        if (!speed) speed = 1;
        scrollDir_ = speed;
        if (!timer_.IsRunning()) {
            timer_.Start(SCROLL_SPEED);
            timer_.Notify();
        }
    } else if (timer_.IsRunning())
        timer_.Stop();
    if (HitPos(event.GetX(), event.GetY(), hit, true)) {
        dragPoint_ = hit;
        UpdateCaret(hit.offset, true, hit.eol);
        UpdateSelectionDrag();
    }
}

void HexEditor::OnScrollTimer(wxTimerEvent& event) {
    if (!seldown_) {
        timer_.Stop();
        return;
    }
    parent_->ScrollLine(scrollDir_);
}

void HexEditor::OnSetCursor(wxSetCursorEvent& event) {
    wxCoord x = event.GetX();
    bool hitInside = x < hexMaxX_ || (textX_ <= x && x < textMaxX_);
    event.SetCursor(hitInside ? wxCursor(wxCURSOR_IBEAM) : *wxSTANDARD_CURSOR);
}

static int selectTest(bufsize s0, bufsize s1, bufsize r0, bufsize r1) {
    if (s0 <= r0 && r1 <= s1) return 1;
    return 0;
}

void HexEditor::UpdateSelectionDrag() {
    bufsize selold = sel_, seleold = sel_ + seln_;
    unsigned oldSelRow0 = convertRow(sel_, off_, rows_, columns_);
    unsigned oldSelRow1 = convertRow(sel_ + seln_, off_, rows_, columns_);
    if (cur_ >= selst_) {
        sel_ = selst_;
        seln_ = cur_ - selst_;
    } else {
        sel_ = cur_;
        seln_ = selst_ - cur_;
    }
    if (seln_ && selold != seleold) curnibble_ = false;
    bufsize sele = sel_ + seln_;
    unsigned selRow0 = convertRow(sel_, off_, rows_, columns_);
    unsigned selRow1 = convertRow(sel_ + seln_, off_, rows_, columns_);
    if (selRow0 > oldSelRow0) selRow0 = oldSelRow0;
    if (selRow1 < oldSelRow1) selRow1 = oldSelRow1;
    for (unsigned row = selRow0; row <= selRow1; ++row) {
        bufsize rowst = off_ + row * columns_;
        int so = selectTest(selold, seleold, rowst, rowst + columns_);
        int sn = selectTest(sel_, sele, rowst, rowst + columns_);
        if (!(so & sn)) RedrawRow(row);
    }
    // shift caret backwards, maybe
    if (seln_ && !curtext_ && !careteol_ && cur_ % columns_ > 0 &&
        cur_ == sel_ + seln_) {
        int x, y;
        caret_.GetPosition(&x, &y);
        caret_.Move(x - charWidth_, y);
    }
    UpdateStatusBar();
    if (sel_ != selold || sele != seleold) parent_->OnSelectChanged();
}

void HexEditor::OnPaint(wxPaintEvent& e) {
    if (!columns_) return;
    auto dc = wxPaintDC(this);
    int vX, vY, vW, vH;
    wxRegionIterator upd(GetUpdateRegion());
    while (upd) {
        vX = upd.GetX();
        vY = upd.GetY();
        vW = upd.GetW();
        vH = upd.GetH();
        dc.Blit(vX, vY, vW, vH, dc_, vX, vY, wxCOPY);
        ++upd;
    }
}

void HexEditor::OnFocus(wxFocusEvent& event) {
    if (caretshow_ && caret_.IsOk() && !caret_.IsVisible()) ShowCaret();
}

void HexEditor::OnBlur(wxFocusEvent& event) {
    if (caret_.IsVisible()) HideCaret();
}

void HexEditor::OnShow(wxShowEvent& event) { WhenVisible(); }

void HexEditor::ResizeDone() {
    GetDC();
    if (IsShownOnScreen()) Redraw();
}

void HexEditor::Selected() { WhenVisible(); }

void HexEditor::OnResize(wxSizeEvent& e) {
    /* ResizeDone(); */
    e.Skip();
}

};  // namespace ui
};  // namespace hexbed
