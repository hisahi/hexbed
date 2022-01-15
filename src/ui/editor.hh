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
// ui/editor.hh -- header for the HexBed editor class

#ifndef HEXBED_UI_EDITOR_HH
#define HEXBED_UI_EDITOR_HH

#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/gbsizer.h>
#include <wx/panel.h>
#include <wx/scrolbar.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statusbr.h>
#include <wx/timer.h>

#include <memory>

#include "file/document.hh"
#include "ui/context.hh"
#include "ui/hexbed-fwd.hh"
#include "ui/hexedit-fwd.hh"

namespace hexbed {
namespace ui {

wxDECLARE_EVENT(HEX_EDIT_EVENT, wxCommandEvent);
wxDECLARE_EVENT(HEX_CARET_EVENT, wxCommandEvent);
wxDECLARE_EVENT(HEX_SELECT_EVENT, wxCommandEvent);

constexpr int RESIZE_TIMEOUT = 200;

struct SelectFlags {
    constexpr SelectFlags() : value(0) {}

    constexpr SelectFlags& caretAtBeginning() noexcept {
        value &= ~1;
        return *this;
    }
    constexpr SelectFlags& caretAtEnd() noexcept {
        value |= ~1;
        return *this;
    }
    constexpr SelectFlags& highlightNone() noexcept {
        value &= ~6;
        return *this;
    }
    constexpr SelectFlags& highlightCaret() noexcept {
        if (isCaretAtEnd())
            highlightEnd();
        else
            highlightBeginning();
        return *this;
    }
    constexpr SelectFlags& highlightBeginning() noexcept {
        value &= ~6;
        value |= 2;
        return *this;
    }
    constexpr SelectFlags& highlightEnd() noexcept {
        value &= ~6;
        value |= 4;
        return *this;
    }

    constexpr bool isCaretAtBeginning() const noexcept {
        return (value & 1) == 0;
    }
    constexpr bool isCaretAtEnd() const noexcept { return (value & 1) != 0; }
    constexpr bool isHighlight() const noexcept { return (value & 6) == 0; }
    constexpr bool isHighlightBegin() const noexcept {
        return (value & 6) == 2;
    }
    constexpr bool isHighlightEnd() const noexcept { return (value & 6) == 4; }

    unsigned value;
};

class wxScrolledWindowFocusRedirect : public wxScrolledWindow {
  public:
    wxScrolledWindowFocusRedirect(wxWindow* parent)
        : wxScrolledWindow(parent) {}
    void SetFocus() override {
        if (focusTarget_)
            focusTarget_->SetFocus();
        else
            wxScrolledWindow::SetFocus();
    }
    void SetFocusTarget(wxWindow* target) { focusTarget_ = target; }

  private:
    wxWindow* focusTarget_{nullptr};
};

class LongScrollBar : public wxScrollBar {
  public:
    using scroll_t = bufsize;
    using wxScrollBar::wxScrollBar;

    virtual void SetScrollbar(int position, int thumbSize, int range,
                              int pageSize, bool refresh = true) override;
    virtual void SetThumbPosition(int viewStart) override;

    virtual scroll_t GetPageSizeLong() const;
    virtual scroll_t GetRangeLong() const;
    virtual scroll_t GetThumbPositionLong() const;
    virtual scroll_t GetThumbSizeLong() const;
    virtual void SetScrollbarLong(scroll_t position, scroll_t thumbSize,
                                  scroll_t range, scroll_t pageSize,
                                  bool refresh = true);
    virtual void SetThumbPositionLong(scroll_t viewStart);

  private:
    size_t shift_{0};
    scroll_t posRem_{0};
    scroll_t thumbRem_{0};
    scroll_t rangeRem_{0};
    scroll_t pageRem_{0};
};

class HexEditorParent {
  public:
    virtual HexBedDocument& document() = 0;
    virtual HexBedContextMain& context() = 0;
    virtual void BringOffsetToScreen(bufsize offset) = 0;
    virtual bool ScrollLine(int dir) = 0;
    virtual bool ScrollPage(int dir) = 0;
    virtual bufsize GetColumnCount() const = 0;
    virtual void OnCaretMoved() = 0;
    virtual void OnSelectChanged() = 0;
    virtual void OnEditorCopy() = 0;

    virtual void ReloadConfig() = 0;
    virtual void ReloadFile() = 0;
    virtual void SelectBytes(bufsize start, bufsize length,
                             SelectFlags flags) = 0;
    virtual void SelectNone() = 0;
    virtual void GetSelection(bufsize& start, bufsize& length, bool& text) = 0;
    virtual HexBedPeekRegion PeekBufferAtCursor() = 0;
    virtual void HintByteChanged(bufsize offset) = 0;
    virtual void HintBytesChanged(bufsize begin) = 0;
    virtual void HintBytesChanged(bufsize begin, bufsize end) = 0;
};

class HexBedEditor : public wxPanel, public HexEditorParent {
  public:
    HexBedEditor(HexBedMainFrame* frame, wxWindow* parent,
                 HexBedContextMain* ctx,
                 std::shared_ptr<HexBedDocument>&& document);
    HexBedEditor(HexBedMainFrame* frame, wxWindow* parent,
                 HexBedContextMain* ctx, HexBedDocument&& document);
    inline HexBedDocument& document() override { return *document_; }
    inline HexBedContextMain& context() override { return *ctx_; }
    inline std::shared_ptr<HexBedDocument> copyDocument() { return document_; }

    void Selected();
    void UpdateStatusBar();
    void SetStatusBar(wxStatusBar* sbar);
    bool DidUnsavedChange(bool unsaved);

    void FullUpdate();
    bool AutoFitUpdate();
    void LayoutUpdate();
    void ResizeUpdate();
    void FileSizeUpdate();
    void ScrollUpdate();
    void DisplayUpdate();

    inline void ReloadConfig() override { FullUpdate(); }
    void ReloadFile() override;
    void BringOffsetToScreen(bufsize offset) override;
    bool ScrollLine(int dir) override;
    bool ScrollPage(int dir) override;
    bufsize GetColumnCount() const override;
    void OnCaretMoved() override;
    void OnSelectChanged() override;
    void OnEditorCopy() override;

    void SelectBytes(bufsize start, bufsize length, SelectFlags flags) override;
    void SelectNone() override;
    void GetSelection(bufsize& start, bufsize& length, bool& text) override;
    HexBedPeekRegion PeekBufferAtCursor() override;
    void HintByteChanged(bufsize offset) override;
    void HintBytesChanged(bufsize begin) override;
    void HintBytesChanged(bufsize begin, bufsize end) override;

    void FocusEditor();

  protected:
    void OnResize(wxSizeEvent& event);
    void OnResizeTimer(wxTimerEvent& event);
    void OnScroll(wxScrollEvent& event);
    void OnMouseWheel(wxMouseEvent& event);

  private:
    HexBedMainFrame* frame_;
    std::shared_ptr<HexBedDocument> document_;
    HexBedContextMain* ctx_;
    wxBoxSizer* box_;
    wxFlexGridSizer* grid_;
    wxStaticText* rowTop_;
    wxStaticText* colOffset_;
    HexEditor* hexEdit_;
    LongScrollBar* scroll_;
    wxSizerItem* shift_;
    wxScrolledWindowFocusRedirect* scroller_;
    unsigned rows_{0};
    unsigned cols_{1};
    unsigned group_{0};
    bufsize frows_{0};
    bufsize fsize_{0};
    int offsetWidth_{0};
    int mouseDelta_{0};
    float charWidth_{0};
    unsigned rowHeight_{0};
    bool wasUnsaved_{false};
    std::vector<char> offsetBuf_;
    wxTimer timer_;
};

};  // namespace ui
};  // namespace hexbed

#endif /* HEXBED_UI_EDITOR_HH */
