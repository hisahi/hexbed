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
// ui/context.hh -- header for the main program UI

#ifndef HEXBED_UI_CONTEXT_HH
#define HEXBED_UI_CONTEXT_HH

#include <memory>
#include <unordered_map>
#include <vector>

#include "common/types.hh"
#include "file/context.hh"
#include "file/document-fwd.hh"
#include "file/task.hh"
#include "ui/editor-fwd.hh"
#include "ui/hexbed-fwd.hh"

namespace hexbed {

constexpr unsigned MAX_LOOKAHEAD = 256;

struct EditorState {
    bool insert{false};
    bool searchWrapAround{false};
};

class HexBedContextMain;

// a TEMPORARY view into the currently open file
struct HexBedPeekRegion {
    HexBedDocument* document{nullptr};
    bufsize offset{0};
    const_bytespan data{};
};

class HexBedViewer {
  public:
    HexBedViewer(std::shared_ptr<HexBedContextMain> context, bufsize lookahead);
    HexBedViewer(HexBedViewer& copy) = default;
    HexBedViewer(HexBedViewer&& move) = default;
    HexBedViewer& operator=(HexBedViewer& copy) = default;
    HexBedViewer& operator=(HexBedViewer&& move) = default;
    virtual ~HexBedViewer();

    inline bufsize lookahead() const noexcept { return lookahead_; }

    /* peek may become invalid! you must copy the data if you need it
       after onUpdateCursor ends */
    virtual inline void onUpdateCursor(HexBedPeekRegion peek) {}

  protected:
    std::shared_ptr<HexBedContextMain> ctx_;

  private:
    bufsize lookahead_;
};

class HexBedContextMain : public HexBedContext {
  public:
    HexBedContextMain(hexbed::ui::HexBedMainFrame* main_);
    HexBedTaskHandler* getTaskHandler();

    bool shouldBackup();
    FailureResponse ifBackupFails(const char* message);

    void announceBytesChanged(HexBedDocument* doc, bufsize start);
    void announceBytesChanged(HexBedDocument* doc, bufsize start,
                              bufsize length);
    void announceUndoChanged(HexBedDocument* doc);
    void announceCursorUpdate(HexBedPeekRegion peek);

    hexbed::ui::HexEditorParent* activeWindow() noexcept;
    void addWindow(hexbed::ui::HexEditorParent* editor);
    void removeWindow(hexbed::ui::HexEditorParent* editor) noexcept;
    void updateWindows();

    void addViewer(HexBedViewer* viewer);
    void removeViewer(HexBedViewer* viewer) noexcept;

    byte* getSearchBuffer(bufsize n);
    const_bytespan getSearchString() const noexcept;

    byte* getReplaceBuffer(bufsize n);
    const_bytespan getReplaceString() const noexcept;

    EditorState state;

  private:
    hexbed::ui::HexBedMainFrame* main_;
    std::unique_ptr<HexBedTaskHandler> task_;
    std::unordered_map<HexBedDocument*,
                       std::vector<hexbed::ui::HexEditorParent*>>
        open_;
    std::vector<byte> searchBuffer_;
    std::vector<byte> replaceBuffer_;
    std::vector<HexBedViewer*> viewers_;
    HexBedDocument* lastdoc_;
    bufsize lastoff_;
};

}  // namespace hexbed

#endif /* HEXBED_UI_CONTEXT_HH */
