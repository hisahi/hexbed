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
// ui/hexedit.hh -- header for the hex editor class

#ifndef HEXBED_UI_HEXEDIT_HH
#define HEXBED_UI_HEXEDIT_HH

#include <wx/window.h>
// clang-format off
#include <wx/caret.h>
#include <wx/statusbr.h>
#include <wx/timer.h>
// clang-format on

#include <mutex>

#include "common/config.hh"
#include "common/types.hh"
#include "file/document.hh"
#include "ui/context.hh"
#include "ui/editor.hh"

namespace hexbed {
namespace ui {

struct HitPoint {
    bufsize offset;
    bool text;
    bool nibble;
    bool eol;
};

class HexEditor : public wxWindow {
  public:
    HexEditor(wxWindow* parent, HexEditorParent* editor);

    inline HexBedDocument& document() { return parent_->document(); }
    inline HexBedContextMain& context() { return parent_->context(); }

    unsigned GetColumns() const;
    void SetOffset(bufsize offset);
    void SetRows(unsigned rows);
    void SetColumns(unsigned columns);
    void GetDC();
    void Rebuffer();
    void ResizeDone();
    void Redraw();
    void FullRefresh();
    void Selected();

    static void InitConfig();
    unsigned FitColumns(wxCoord width);
    unsigned GetLineHeight();

    void SetStatusBar(wxStatusBar* sbar);
    // in status.cc
    void UpdateStatusBar();

    void SelectBytes(bufsize start, bufsize length, SelectFlags flags);
    void SelectNone();
    void GetSelection(bufsize& start, bufsize& length, bool& text);
    HexBedPeekRegion PeekBufferAtCursor();
    void HintByteChanged(bufsize offset);
    void HintBytesChanged(bufsize begin);
    void HintBytesChanged(bufsize begin, bufsize end);

    void DoCtrlCut();
    void DoCtrlCopy();
    void DoCtrlPasteInsert();
    void DoCtrlPasteOverwrite();
    void DoCtrlUndo();
    void DoCtrlRedo();

    HexEditor(HexEditor& copy) = delete;
    HexEditor(HexEditor&& move) = delete;
    HexEditor& operator=(HexEditor& copy) = delete;
    HexEditor& operator=(HexEditor&& move) = delete;
    ~HexEditor();

  protected:
    void OnChar(wxKeyEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
    void OnPaint(wxPaintEvent& event);
    void OnFocus(wxFocusEvent& event);
    void OnBlur(wxFocusEvent& event);
    void OnShow(wxShowEvent& event);
    void OnResize(wxSizeEvent& event);
    void OnLMouseDown(wxMouseEvent& event);
    void OnLMouseUp(wxMouseEvent& event);
    void OnRMouseDown(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnSetCursor(wxSetCursorEvent& event);
    void OnScrollTimer(wxTimerEvent& event);

  private:
    void InitDraw();

    void DrawFull();
    void DrawRow(bufsize r);
    template <bool selectCheck>
    void DrawRow_(const char* hex, const byte* si, wxCoord y, bufsize off);
    void RedrawRow(int r);
    void RedrawLite();

    void UpdateCaret();
    void UpdateCaret(bufsize cur, bool redraw = true, bool eol = false);
    void ShowCaret();
    void HideCaret();
    void UpdateSelectionDrag();
    void HandleHexInput(char ch);
    void HandleTextInput(byte b);

    void StartSelection(bufsize offset);
    void Deselect();
    void UpdateCaretSelect(bool shift, bufsize cur, bool redraw = true,
                           bool eol = false);

    void DoReplaceByte(byte v);
    void DoInsertByte(byte v);
    void DoRemoveByte(bufsize offset);
    void DoRemoveBytes(bufsize offset, bufsize len);

    wxCoord GetColumnX(unsigned c) const noexcept;
    void GetColumnFromX(wxCoord x, unsigned& div, unsigned& rem) const noexcept;
    bool HitPos(wxCoord x, wxCoord y, HitPoint& point, bool drag);
    bufsize Buffer(bufsize beg, bufsize end, bufsize off);

    void WhenVisible();
    void QueueRefresh();
    bool TestVisibility() const noexcept;

    void OnEditorCopy();
    void OnUndoRedo();
    bool DoEditorCopy();
    void DoEditorPaste(bool insert);

    HexEditorParent* parent_;
    wxMemoryDC* dc_{nullptr};
    wxBitmap* surface_{nullptr};
    bool waitRedraw_{false};

    bufsize off_{0};

    unsigned rows_{0};
    unsigned columns_{0};
    unsigned group_{0};

    bool caretshow_{false};
    bool curtext_{false};
    bool curnibble_{false};
    bufsize cur_{0};
    wxCaret caret_;
    bool careteol_{false};

    bool hasText_{false};
    bool hasHex_{false};

    bool seldown_{false};
    bufsize sel_{0};
    bufsize seln_{0};
    bufsize selst_{0};

    std::vector<byte> buffer_;
    bufsize bufn_{0};
    bufsize bufviewable_{0};
    bufsize bufc_{0};

    std::mutex mutex_;

    wxStatusBar* sbar_{nullptr};
    wxTimer timer_;
    int scrollDir_{0};
    HitPoint dragPoint_;

    /* size measurements */
    wxCoord textWidth_;
    wxCoord textX_;
    wxCoord hexMaxX_;
    wxCoord textMaxX_;
    wxCoord charWidth_;
    wxCoord byteWidth_;
    wxCoord lineHeight_;

    /* color cache */
    static wxColor backColor_;
    static wxColor textColor_;
    static wxColor textNPColor_;
    static wxColor oddColor_;
    static wxColor evenColor_;
    static wxColor alignColor_;
    static wxColor selBgColor_;
    static wxColor selFgColor_;
    static wxColor selBg2Color_;
    static wxColor selFg2Color_;
};

};  // namespace ui
};  // namespace hexbed

#endif /* HEXBED_UI_HEXEDIT_HH */
