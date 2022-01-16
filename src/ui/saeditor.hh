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
// ui/saeditor.hh -- header for the HexBed standalone editor class

#ifndef HEXBED_UI_SAEDITOR_HH
#define HEXBED_UI_SAEDITOR_HH

#include "ui/editor.hh"

namespace hexbed {
namespace ui {

class HexBedStandaloneEditor : public wxPanel, public HexEditorParent {
  public:
    HexBedStandaloneEditor(wxWindow* parent, HexBedContextMain* ctx,
                           std::shared_ptr<HexBedDocument>&& document);
    inline HexBedDocument& document() override { return *document_; }
    inline HexBedContextMain& context() override { return *ctx_; }
    inline std::shared_ptr<HexBedDocument> copyDocument() { return document_; }

    void Selected();

    void FullUpdate();
    void LayoutUpdate();
    void ResizeUpdate();
    void FileSizeUpdate();
    void DisplayUpdate();

    void BringOffsetToScreen(bufsize offset) override;
    bool ScrollLine(int dir) override;
    bool ScrollPage(int dir) override;
    bufsize GetColumnCount() const override;
    void OnCaretMoved() override;
    void OnSelectChanged() override;
    void OnEditorCopy() override;
    void OnUndoRedo() override;

    inline void ReloadConfig() override { FullUpdate(); }
    void ReloadFile() override;
    void SelectBytes(bufsize start, bufsize length, SelectFlags flags) override;
    void SelectAll(SelectFlags flags);
    void SelectNone() override;
    void GetSelection(bufsize& start, bufsize& length, bool& text) override;
    HexBedPeekRegion PeekBufferAtCursor() override;
    void HintByteChanged(bufsize offset) override;
    void HintBytesChanged(bufsize begin) override;
    void HintBytesChanged(bufsize begin, bufsize end) override;

    void DoCtrlCut() override;
    void DoCtrlCopy() override;
    void DoCtrlPasteInsert() override;
    void DoCtrlPasteOverwrite() override;
    void DoCtrlUndo() override;
    void DoCtrlRedo() override;

    void FocusEditor();

  protected:
    void OnResize(wxSizeEvent& event);
    void OnResizeTimer(wxTimerEvent& event);
    void OnScroll(wxScrollEvent& event);
    void OnMouseWheel(wxMouseEvent& event);

  private:
    std::shared_ptr<HexBedDocument> document_;
    HexBedContextMain* ctx_;
    HexEditor* hexEdit_;
    LongScrollBar* scroll_;
    unsigned rows_{0};
    unsigned cols_{0};
    unsigned group_{0};
    bufsize frows_{0};
    bufsize fsize_{0};
    int mouseDelta_{0};
    unsigned rowHeight_{0};
    wxTimer timer_;
};

};  // namespace ui
};  // namespace hexbed

#endif /* HEXBED_UI_SAEDITOR_HH */
