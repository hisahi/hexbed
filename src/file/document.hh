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
// file/document.hh -- header for the HexBed document class

#ifndef HEXBED_FILE_DOCUMENT_HH
#define HEXBED_FILE_DOCUMENT_HH

#include <deque>
#include <memory>

#include "common/logger.hh"
#include "common/types.hh"
#include "file/context.hh"
#include "file/search.hh"
#include "file/task.hh"
#include "file/treble.hh"

namespace hexbed {

using WriteCallback = std::function<void(VirtualBuffer&)>;

struct HexBedRange {
    bufsize offset{0};
    bufsize length{0};
};

class HexBedBuffer {
  public:
    virtual bufsize read(bufoffset offset, bytespan data) = 0;
    virtual void write(HexBedContext& ctx, WriteCallback write,
                       const std::string& filename) = 0;
    virtual void writeOverlay(HexBedContext& ctx, WriteCallback write,
                              const std::string& filename) = 0;
    virtual void writeNew(HexBedContext& ctx, WriteCallback write,
                          const std::string& filename) = 0;
    virtual void writeCopy(HexBedContext& ctx, WriteCallback write,
                           const std::string& filename) = 0;

    virtual bufsize size() noexcept;
    virtual bufsize size() const noexcept = 0;

    virtual ~HexBedBuffer() noexcept;
};

class HexBedDocument;

enum class HexBedUndoType {
    ReplaceOne,
    ReplaceOneOriginal,
    ReplaceMany,
    ReplaceDiffSize,
    Insert,
    Delete
};

static constexpr bufsize UNDOSTRIPE_MAX = BUFSIZE_MAX >> 1;

// ugly but does the job with minimal memory
struct HexBedUndoStripe {
    bufsize data;
    inline HexBedUndoStripe(bool orig, bufsize sz)
        : data((sz << 1) | (orig ? 1 : 0)) {
        HEXBED_ASSERT(sz < UNDOSTRIPE_MAX, "too long of a stripe");
    }
    inline HexBedUndoStripe(bufsize raw) : data(raw) {}

    inline bufsize size() const noexcept { return data >> 1; }
    inline bool original() const noexcept { return data & 1; }
    inline bufsize raw() const noexcept { return data; }
};

struct HexBedUndoEntry {
    struct HexBedUndoEntrySwapRange {
        std::vector<byte> oldValues;
        std::vector<HexBedUndoStripe> oldStripes;
    };

    HexBedUndoType type;
    byte oldValue;
    bool wasDirty;
    bool compress;
    bufsize offset;
    bufsize size;
    std::vector<byte> oldValues;
    std::vector<HexBedUndoStripe> oldStripes;

    HexBedRange undo(HexBedDocument& doc);
    HexBedRange redo(HexBedDocument& doc);
    void detach();

  private:
    template <bool insert, bool adjust>
    void replant(HexBedDocument& doc);
    byte swapValue(HexBedDocument& doc);
    HexBedUndoEntrySwapRange swapRange(HexBedDocument& doc, bool sameSize);
    void applySwapRange(HexBedUndoEntrySwapRange& range);
};

class UndoToken {
  public:
    inline UndoToken() : undos_(nullptr), index_(0), stored_(true) {}
    inline UndoToken(std::deque<HexBedUndoEntry>* undos, size_t x)
        : undos_(undos), index_(x), stored_(false) {}
    inline void commit() noexcept { stored_ = true; }

    UndoToken(const UndoToken& copy) = delete;
    UndoToken(UndoToken&& move) = delete;
    UndoToken& operator=(const UndoToken& copy) = delete;
    UndoToken& operator=(UndoToken&& move) = delete;
    inline ~UndoToken() {
        if (undos_ && !stored_)
            undos_->erase(undos_->begin() + index_,
                          undos_->begin() + index_ + 1);
    }

  private:
    std::deque<HexBedUndoEntry>* undos_;
    size_t index_;
    bool stored_;
};

class UndoGroupToken {
  public:
    inline UndoGroupToken(std::deque<HexBedUndoEntry>* undos, bool* flag)
        : undos_(undos),
          extr_(0),
          flag_(flag),
          flagOld_(flag && std::exchange(*flag, true)) {}
    inline void tick() { ++extr_; }
    void commit();

    UndoGroupToken(const UndoGroupToken& copy) = delete;
    UndoGroupToken(UndoGroupToken&& move) = delete;
    UndoGroupToken& operator=(const UndoGroupToken& copy) = delete;
    UndoGroupToken& operator=(UndoGroupToken&& move) = delete;
    ~UndoGroupToken();

  private:
    std::deque<HexBedUndoEntry>* undos_;
    bufsize extr_;
    bool stored_;
    bool* flag_;
    bool flagOld_;
};

class HexBedDocument {
  public:
    HexBedDocument(std::shared_ptr<HexBedContext> context);
    HexBedDocument(std::shared_ptr<HexBedContext> context,
                   const std::string& filename);
    HexBedDocument(std::shared_ptr<HexBedContext> context,
                   const std::string& filename, bool readOnly);

    HexBedDocument(HexBedDocument& copy) = delete;
    HexBedDocument(HexBedDocument&& move) = default;
    HexBedDocument& operator=(HexBedDocument& copy) = delete;
    HexBedDocument& operator=(HexBedDocument&& move) = default;
    ~HexBedDocument();

    bufsize read(bufoffset offset, bytespan data) const;

    bool impose(bufoffset offset, byte value);
    bool impose(bufoffset offset, bufsize size, byte value);
    bool impose(bufoffset offset, bufsize nsize, bufsize ssize,
                const byte* svalues);
    bool impose(bufoffset offset, const_bytespan data);
    bool replace(bufoffset offset, bufsize size, bufsize newsize, byte value);
    bool replace(bufoffset offset, bufsize size, const_bytespan data);
    bool replace(bufoffset offset, bufsize size, bufsize nsize, bufsize ssize,
                 const byte* svalues);
    bool insert(bufoffset offset, byte value);
    bool insert(bufoffset offset, bufsize size, byte value);
    bool insert(bufoffset offset, const_bytespan data);
    bool insert(bufoffset offset, bufsize nsize, bufsize ssize,
                const byte* svalues);
    bool remove(bufoffset offset);
    bool remove(bufoffset offset, bufsize size);

    bool map(bufoffset offset, bufsize size,
             std::function<bool(bufoffset, bytespan)> mapper);

    SearchResult searchForward(bufoffset start, bufoffset end,
                               const_bytespan data);
    SearchResult searchBackward(bufoffset start, bufoffset end,
                                const_bytespan data);

    SearchResult searchForwardFull(bufoffset start, bool wrap,
                                   const_bytespan data);
    SearchResult searchBackwardFull(bufoffset start, bool wrap,
                                    const_bytespan data);

    bool compareEqual(bufoffset offset, bufoffset size, const_bytespan data);

    bufsize size() const noexcept;
    bool filed() const noexcept;
    bool unsaved() const noexcept;
    std::string path() const;
    bool canUndo() const noexcept;
    bool canRedo() const noexcept;
    bool readOnly() const noexcept;

    void discard();
    void commit();
    void commitAs(const std::string& filename);
    void commitTo(const std::string& filename);

    UndoGroupToken undoGroup();
    HexBedRange undo();
    HexBedRange redo();

  private:
    std::shared_ptr<HexBedContext> context_;
    std::string filename_;
    std::unique_ptr<HexBedBuffer> buffer_;
    std::deque<HexBedUndoEntry> undos_;
    bufsize undoDepth_{0};
    Treble treble_;
    bool dirty_{false};
    bool readOnly_{false};
    bool noUndoLimit_{false};

    void grow();

    bool trebleReplaceDiffSize(bufoffset offset, bufsize old, bufsize cnt,
                               byte v);
    bool trebleReplaceDiffSize(bufoffset offset, bufsize old, bufsize cnt,
                               const byte* data);
    bool trebleReplaceDiffSize(bufoffset offset, bufsize old, bufsize cnt,
                               bufsize sn, const byte* sb);

    UndoToken addUndo(HexBedUndoEntry&& entry);
    UndoToken addUndoReplaceOne(bufsize off);
    UndoToken addUndoReplaceMany(bufsize off, bufsize cnt);
    UndoToken addUndoReplaceDiffSize(bufsize off, bufsize old, bufsize cnt);
    UndoToken addUndoInsert(bufsize off, bufsize cnt);
    UndoToken addUndoRemove(bufsize off, bufsize cnt);
    void truncateUndo();

    friend class HexBedUndoEntry;
};

};  // namespace hexbed

#endif /* HEXBED_FILE_DOCUMENT_HH */
