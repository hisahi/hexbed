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
// file/document.cc -- implementation for the HexBed document and buffer classes

#include "file/document.hh"

#include <filesystem>

#include "app/config.hh"
#include "common/memory.hh"
#include "file/bfile.hh"
//#include "file/bmmap.hh"
#include "file/bnew.hh"

namespace hexbed {

template <typename T>
const T* const_this(T* p) {
    return static_cast<const T*>(p);
}

HexBedBuffer::~HexBedBuffer() {}

bufsize HexBedBuffer::size() noexcept { return const_this(this)->size(); }

static std::unique_ptr<HexBedBuffer> bufferNew() {
    return std::make_unique<HexBedBufferNew>();
}

#define TRY_OPEN(T, filename)                 \
    try {                                     \
        return std::make_unique<T>(filename); \
    } catch (...) {                           \
    }

static std::unique_ptr<HexBedBuffer> bufferOpen(const std::string& filename) {
    [[maybe_unused]] bool szknown;
    [[maybe_unused]] bufsize sz;
    try {
        sz = std::filesystem::file_size(filename);
        szknown = true;
    } catch (std::filesystem::filesystem_error& e) {
        sz = 0;
        szknown = false;
    }
#if HEXBED_MMAP_OK
    TRY_OPEN(HexBedBufferMmap, filename);
#endif
    return std::make_unique<HexBedBufferFile>(filename);
}

void HexBedDocument::grow() {}

HexBedDocument::HexBedDocument(HexBedContext* ctx)
    : context_(ctx), filename_(), buffer_(bufferNew()), treble_(0) {}

HexBedDocument::HexBedDocument(HexBedContext* ctx, const std::string& filename)
    : HexBedDocument(ctx, filename, false) {}

HexBedDocument::HexBedDocument(HexBedContext* ctx, const std::string& filename,
                               bool readOnly)
    : context_(ctx),
      filename_(filename),
      buffer_(bufferOpen(filename)),
      treble_(buffer_->size()),
      readOnly_(readOnly) {
    LOG_TRACE("opened file as %s", typeid(*buffer_.get()).name());
}

HexBedDocument::~HexBedDocument() {}

bufsize HexBedDocument::read(bufoffset offset, bytespan data) const {
    return treble_.read(*buffer_, data.data(), offset, data.size());
}

bool HexBedDocument::canUndo() const noexcept {
    return undoDepth_ < undos_.size();
}

bool HexBedDocument::canRedo() const noexcept { return undoDepth_ > 0; }

UndoGroupToken HexBedDocument::undoGroup() {
    return UndoGroupToken(&undos_, &noUndoLimit_);
}

void UndoGroupToken::commit() {
    if (extr_) {
        // TODO
    }
    if (flag_) *flag_ = flagOld_;
    stored_ = true;
}

UndoGroupToken::~UndoGroupToken() {
    if (!stored_ && extr_) undos_->erase(undos_->end() - extr_, undos_->end());
}

HexBedRange HexBedDocument::undo() {
    if (readOnly()) return HexBedRange{};
    size_t c = undos_.size();
    return undoDepth_ < c ? undos_[c - ++undoDepth_].undo(*this)
                          : HexBedRange{};
}

HexBedRange HexBedDocument::redo() {
    if (readOnly()) return HexBedRange{};
    return undoDepth_ ? undos_[undos_.size() - undoDepth_--].redo(*this)
                      : HexBedRange{};
}

UndoToken HexBedDocument::addUndo(HexBedUndoEntry&& entry) {
    if (readOnly()) return UndoToken();
    truncateUndo();
    if (!noUndoLimit_)
        while (undos_.size() >=
               static_cast<size_t>(config().undoHistoryMaximum))
            undos_.pop_front();
    size_t z;
addUndoAgain:
    try {
        z = undos_.size();
        undos_.push_back(std::move(entry));
    } catch (const std::bad_alloc& e) {
        undos_.pop_front();
        goto addUndoAgain;
    }
    return UndoToken(&undos_, z);
}

class UndoWriter {
  public:
    UndoWriter(HexBedBuffer& buf, std::vector<byte>& b,
               std::vector<HexBedUndoStripe>& s)
        : buf(buf), b(b), s(s) {}
    void raw(bufsize n, const byte* r) {
        bufsize z = b.size();
        b.resize(z + n);
        memCopy(b.data() + z, r, n);
        addStripe<false>(n, 0);
    }
    void copy(bufsize n, bufsize o) {
        bufsize z = b.size();
        b.resize(z + n);
        z = buf.read(o, bytespan{b.data() + z, n});
        if (z < n)
            throw std::runtime_error("could not read everything we need!");
        addStripe<true>(n, o);
    }

  private:
    HexBedBuffer& buf;
    std::vector<byte>& b;
    std::vector<HexBedUndoStripe>& s;

    template <bool orig>
    void addStripe(bufsize n, bufsize o) {
        while (n) {
            bufsize z = std::min(n, UNDOSTRIPE_MAX);
            s.emplace_back(orig, z);
            // add original offset
            if (orig) s.emplace_back(o);
            n -= z;
            o += z;
        }
    }
};

struct ByteReader {
  public:
    ByteReader(HexBedBuffer& buf) : buf(buf){};
    byte operator()(bufsize off) {
        byte b;
        [[maybe_unused]] bufsize z = buf.read(off, {&b, 1});
        HEXBED_ASSERT(z);
        return b;
    }

  private:
    HexBedBuffer& buf;
};

UndoToken HexBedDocument::addUndoReplaceOne(bufsize off) {
    if (!config().undoHistoryMaximum) return UndoToken();
    ByteReader reader(*buffer_);
    auto result = treble_.readByte(reader, off);
    return addUndo(HexBedUndoEntry{
        .type = result.original ? HexBedUndoType::ReplaceOneOriginal
                                : HexBedUndoType::ReplaceOne,
        .oldValue = result.value,
        .wasDirty = dirty_,
        .compress = false,
        .offset = off,
        .size = 1,
        .oldValues = {},
        .oldStripes = {}});
}

UndoToken HexBedDocument::addUndoReplaceMany(bufsize off, bufsize cnt) {
    if (!config().undoHistoryMaximum) return UndoToken();
    std::vector<byte> vecb;
    std::vector<HexBedUndoStripe> vecs;
    UndoWriter writer(*buffer_, vecb, vecs);
    [[maybe_unused]] bufsize z = treble_.render(writer, off, cnt);
    HEXBED_ASSERT(z == cnt);
    vecb.shrink_to_fit();
    vecs.shrink_to_fit();
    return addUndo(HexBedUndoEntry{.type = HexBedUndoType::ReplaceMany,
                                   .oldValue = 0,
                                   .wasDirty = dirty_,
                                   .compress = false,
                                   .offset = off,
                                   .size = cnt,
                                   .oldValues = std::move(vecb),
                                   .oldStripes = std::move(vecs)});
}

UndoToken HexBedDocument::addUndoReplaceDiffSize(bufsize off, bufsize old,
                                                 bufsize cnt) {
    if (!config().undoHistoryMaximum) return UndoToken();
    std::vector<byte> vecb;
    std::vector<HexBedUndoStripe> vecs;
    UndoWriter writer(*buffer_, vecb, vecs);
    treble_.render(writer, off, old);
    vecb.shrink_to_fit();
    vecs.shrink_to_fit();
    return addUndo(HexBedUndoEntry{.type = HexBedUndoType::ReplaceDiffSize,
                                   .oldValue = 0,
                                   .wasDirty = dirty_,
                                   .compress = false,
                                   .offset = off,
                                   .size = cnt,
                                   .oldValues = std::move(vecb),
                                   .oldStripes = std::move(vecs)});
}

UndoToken HexBedDocument::addUndoInsert(bufsize off, bufsize cnt) {
    if (!config().undoHistoryMaximum) return UndoToken();
    return addUndo(HexBedUndoEntry{.type = HexBedUndoType::Insert,
                                   .oldValue = 0,
                                   .wasDirty = dirty_,
                                   .compress = false,
                                   .offset = off,
                                   .size = cnt,
                                   .oldValues = {},
                                   .oldStripes = {}});
}

UndoToken HexBedDocument::addUndoRemove(bufsize off, bufsize cnt) {
    if (!config().undoHistoryMaximum) return UndoToken();
    std::vector<byte> vecb;
    std::vector<HexBedUndoStripe> vecs;
    UndoWriter writer(*buffer_, vecb, vecs);
    [[maybe_unused]] bufsize z = treble_.render(writer, off, cnt);
    HEXBED_ASSERT(z == cnt);
    vecb.shrink_to_fit();
    vecs.shrink_to_fit();
    return addUndo(HexBedUndoEntry{.type = HexBedUndoType::Delete,
                                   .oldValue = 0,
                                   .wasDirty = dirty_,
                                   .compress = false,
                                   .offset = off,
                                   .size = cnt,
                                   .oldValues = std::move(vecb),
                                   .oldStripes = std::move(vecs)});
}

static bufsize getPreferredSearchBufferSize(bufsize s) {
    if (s >= (1 << 24)) return s << 1;
    if (s >= (1 << 20)) return s << 2;
    if (s >= (1 << 16)) return s << 3;
    if (s >= (1 << 12)) return s << 4;
    return (1 << 12) << 4;
}

static bufsize getMinimalSearchBufferSize(bufsize s) { return s * 2; }

bool HexBedDocument::compareEqual(bufoffset offset, bufoffset size,
                                  const_bytespan data) {
    bufsize z = 64, rr, r, c, o = offset;
    bufsize mins = getMinimalSearchBufferSize(z),
            prefs = getPreferredSearchBufferSize(z);
    byte* bp = new (std::nothrow) byte[(c = prefs)];
    if (!bp) bp = new byte[(c = mins)];
    const byte* si = data.data();
    bufsize cx = data.size();
    std::unique_ptr<byte[]> buffer(bp);
    while ((rr = std::min(cx, c)) && (r = read(o, bytespan(bp, rr)))) {
        if (r < rr) return false;
        if (!memEqual(bp, si, r)) return false;
        o += r;
        cx -= r;
        si += r;
    }
    return true;
}

SearchResult HexBedDocument::searchForward(bufoffset start, bufoffset end,
                                           const_bytespan data) {
    bufsize r, rr, o = start, z = data.size(), c, hc;
    if (start >= end) return SearchResult{};
    if (!z) return SearchResult{SearchResultType::Full, start};
    bufsize mins = getMinimalSearchBufferSize(z),
            prefs = getPreferredSearchBufferSize(z);
    byte* bp = new (std::nothrow) byte[(c = prefs)];
    if (!bp) bp = new byte[(c = mins)];
    std::unique_ptr<byte[]> buffer(bp);
    const byte* si = data.data();
    hc = c >> 1;
    byte* flippers[2];
    flippers[0] = bp;
    flippers[1] = bp + hc;
    byte* flip = flippers[0];
    int flipindex = 1;

    SearchResult pres{};
    while ((rr = std::min(end - o, hc)), (r = read(o, bytespan(flip, rr)))) {
        if (pres.type == SearchResultType::Partial) {
            pres = searchFullForward(hc, flippers[flipindex], r, flip, z, si,
                                     pres.offset);
            if (pres)
                return SearchResult{SearchResultType::Full, o + pres.offset};
        }
        pres = searchPartialForward(r, flip, z, si, r == hc);
        if (pres.type == SearchResultType::Full)
            return SearchResult{SearchResultType::Full, o + pres.offset};
        if (pres.type == SearchResultType::Partial) {
            flip = flippers[flipindex];
            flipindex = flipindex ^ 1;
        }
        if (r < rr) break;
        o += r;
    }
    return SearchResult{};
}

SearchResult HexBedDocument::searchForwardFull(bufoffset start, bool wrap,
                                               const_bytespan data) {
    bufsize dz = size(), sz = data.size();
    if (sz > dz) return SearchResult{};
    SearchResult res = searchForward(start, dz, data);
    if (!res && wrap) {
        bufoffset end = start + sz - 1;
        if (end < start) end = dz;
        res = searchForward(0, end, data);
    }
    return res;
}

SearchResult HexBedDocument::searchBackward(bufoffset start, bufoffset end,
                                            const_bytespan data) {
    bufsize r, rr, o = end, z = data.size(), c, hc;
    if (start >= end) return SearchResult{};
    if (!z) return SearchResult{SearchResultType::Full, end};
    bufsize mins = getMinimalSearchBufferSize(z),
            prefs = getPreferredSearchBufferSize(z);
    byte* bp = new (std::nothrow) byte[(c = prefs)];
    if (!bp) bp = new byte[(c = mins)];
    std::unique_ptr<byte[]> buffer(bp);
    const byte* si = data.data();
    hc = c >> 1;
    byte* flippers[2];
    flippers[0] = bp;
    flippers[1] = bp + hc;
    byte* flip = flippers[0];
    int flipindex = 1;

    SearchResult pres{};
    while ((rr = std::min(o - start, hc)) &&
           rr == (r = read(o - rr, bytespan(flip, rr)))) {
        o -= r;
        if (pres.type == SearchResultType::Partial) {
            pres = searchFullBackward(hc, flippers[flipindex], r, flip, z, si,
                                      pres.offset);
            if (pres)
                return SearchResult{SearchResultType::Full,
                                    o + pres.offset - z + 1};
        }
        pres = searchPartialBackward(r, flip, z, si, o > 0);
        if (pres.type == SearchResultType::Full)
            return SearchResult{SearchResultType::Full,
                                o + pres.offset - z + 1};
        if (pres.type == SearchResultType::Partial) {
            flip = flippers[flipindex];
            flipindex = flipindex ^ 1;
        }
    }
    return SearchResult{};
}

SearchResult HexBedDocument::searchBackwardFull(bufoffset start, bool wrap,
                                                const_bytespan data) {
    bufsize dz = size(), sz = data.size();
    if (sz > dz) return SearchResult{};
    bufoffset end = start + sz - 1;
    if (end < start) end = dz;
    SearchResult res = searchBackward(0, end, data);
    if (!res && wrap) res = searchBackward(start + 1, dz, data);
    return res;
}

template <bool insert, bool adjust>
void HexBedUndoEntry::replant(HexBedDocument& doc) {
    static_assert(!insert || !adjust);
    const byte* si = oldValues.data();
    bool ins = insert;
    bufsize off = offset;
    bufsize n = oldValues.size();
    bufsize cnt = size;
    bufsize oi = oldStripes.size();
    for (bufsize i = 0; i < oi; ++i) {
        const auto& pair = oldStripes[i];
        bufsize z = pair.size();
        if constexpr (adjust) {
            if (!ins && cnt < z) {
                bufsize ll = cnt;
                if (pair.original())
                    doc.treble_.revert(off, ll);
                else
                    doc.treble_.replace(off, ll, si);
                si += ll, off += ll, n -= ll, cnt -= ll;
                z -= ll;
                ins = true;
            }
        }
        if (pair.original()) {
            ++i;
            if (ins)
                doc.treble_.reinsert(off, z, oldStripes[i].raw());
            else
                doc.treble_.revert(off, z);
        } else {
            if (ins)
                doc.treble_.insert(off, z, si);
            else
                doc.treble_.replace(off, z, si);
        }
        si += z, off += z, n -= z, cnt -= z;
    }
    if (n) {
        if (ins)
            doc.treble_.insert(off, n, si);
        else
            doc.treble_.replace(off, n, si);
    }
    if constexpr (adjust) {
        if (!ins && cnt) doc.treble_.remove(off, cnt);
    }
}

byte HexBedUndoEntry::swapValue(HexBedDocument& doc) {
    ByteReader reader(*doc.buffer_);
    return doc.treble_.readByte(reader, offset).value;
}

HexBedUndoEntry::HexBedUndoEntrySwapRange HexBedUndoEntry::swapRange(
    HexBedDocument& doc, bool sameSize) {
    std::vector<byte> vecb;
    std::vector<HexBedUndoStripe> vecs;
    UndoWriter writer(*doc.buffer_, vecb, vecs);
    [[maybe_unused]] bufsize z = doc.treble_.render(writer, offset, size);
    if (sameSize) HEXBED_ASSERT(z == size);
    return HexBedUndoEntrySwapRange{.oldValues = std::move(vecb),
                                    .oldStripes = std::move(vecs)};
}

void HexBedUndoEntry::applySwapRange(HexBedUndoEntrySwapRange& range) {
    range.oldValues.shrink_to_fit();
    oldValues = std::move(range.oldValues);
    range.oldStripes.shrink_to_fit();
    oldStripes = std::move(range.oldStripes);
}

HexBedRange HexBedUndoEntry::undo(HexBedDocument& doc) {
    doc.dirty_ = wasDirty;
    using enum HexBedUndoType;
    switch (type) {
    case ReplaceOne: {
        byte v = swapValue(doc);
        doc.treble_.replace(offset, 1, oldValue);
        oldValue = v;
        doc.context_->announceBytesChanged(&doc, offset, 1);
        return HexBedRange{offset, 1};
    }
    case ReplaceOneOriginal: {
        byte v = swapValue(doc);
        doc.treble_.revert(offset, 1);
        oldValue = v;
        doc.context_->announceBytesChanged(&doc, offset, 1);
        return HexBedRange{offset, 1};
    }
    case ReplaceMany: {
        std::vector<byte> vecb(size);
        doc.read(offset, bytespan(vecb.data(), size));
        replant<false, false>(doc);
        oldValues = std::move(vecb);
        oldStripes.clear();
        doc.context_->announceBytesChanged(&doc, offset, size);
        return HexBedRange{offset, size};
    }
    case ReplaceDiffSize: {
        HexBedUndoEntrySwapRange range = swapRange(doc, false);
        bufsize z = oldValues.size();
        replant<false, true>(doc);
        size = z;
        applySwapRange(range);
        doc.context_->announceBytesChanged(&doc, offset);
        return HexBedRange{offset, z};
    }
    case Insert: {
        HexBedUndoEntrySwapRange range = swapRange(doc, true);
        doc.treble_.remove(offset, size);
        applySwapRange(range);
        doc.context_->announceBytesChanged(&doc, offset);
        return HexBedRange{offset, 0};
    }
    case Delete:
        replant<true, false>(doc);
        doc.context_->announceBytesChanged(&doc, offset);
        return HexBedRange{offset, oldValues.size()};
    }
    return HexBedRange{};
}

HexBedRange HexBedUndoEntry::redo(HexBedDocument& doc) {
    doc.dirty_ = true;
    using enum HexBedUndoType;
    switch (type) {
    case ReplaceOne:
    case ReplaceOneOriginal: {
        byte v = swapValue(doc);
        doc.treble_.replace(offset, 1, oldValue);
        oldValue = v;
        doc.context_->announceBytesChanged(&doc, offset, 1);
        return HexBedRange{offset, 1};
    }
    case ReplaceMany: {
        HexBedUndoEntrySwapRange range = swapRange(doc, true);
        replant<false, false>(doc);
        applySwapRange(range);
        doc.context_->announceBytesChanged(&doc, offset, size);
        return HexBedRange{offset, size};
    }
    case ReplaceDiffSize: {
        HexBedUndoEntrySwapRange range = swapRange(doc, false);
        bufsize z = oldValues.size();
        replant<false, true>(doc);
        size = z;
        applySwapRange(range);
        doc.context_->announceBytesChanged(&doc, offset);
        return HexBedRange{offset, z};
    }
    case Insert:
        replant<true, false>(doc);
        doc.context_->announceBytesChanged(&doc, offset);
        return HexBedRange{offset, oldValues.size()};
    case Delete: {
        HexBedUndoEntrySwapRange range = swapRange(doc, true);
        doc.treble_.remove(offset, size);
        applySwapRange(range);
        doc.context_->announceBytesChanged(&doc, offset);
        return HexBedRange{offset, 0};
    }
    }
    return HexBedRange{};
}

void HexBedDocument::truncateUndo() {
    if (undoDepth_) {
        undos_.erase(undos_.end() - undoDepth_, undos_.end());
        undoDepth_ = 0;
    }
}

void HexBedDocument::trebleReplaceDiffSize(bufoffset off, bufsize old,
                                           bufsize cnt, byte v) {
    if (old < cnt) {
        treble_.replace(off, old, v);
        treble_.insert(off + old, cnt - old, v);
    } else if (old == cnt) {
        treble_.replace(off, cnt, v);
    } else {  // old > cnt
        treble_.replace(off, cnt, v);
        treble_.remove(off + cnt, old - cnt);
    }
}

void HexBedDocument::trebleReplaceDiffSize(bufoffset off, bufsize old,
                                           bufsize cnt, const byte* data) {
    if (old < cnt) {
        treble_.replace(off, old, data);
        treble_.insert(off + old, cnt - old, data + old);
    } else if (old == cnt) {
        treble_.replace(off, cnt, data);
    } else {  // old > cnt
        treble_.replace(off, cnt, data);
        treble_.remove(off + cnt, old - cnt);
    }
}

void HexBedDocument::impose(bufoffset offset, bufsize size, byte value) {
    if (readOnly()) return;
    bufsize z = HexBedDocument::size();
    if (offset + size > z) {
        grow();
        bufsize lo = z - offset;
        auto token = addUndoReplaceDiffSize(offset, lo, size);
        trebleReplaceDiffSize(offset, lo, size, value);
        token.commit();
    } else {
        auto token = addUndoReplaceMany(offset, size);
        treble_.replace(offset, size, value);
        token.commit();
    }
    dirty_ = true;
    context_->announceUndoChange(this);
    context_->announceBytesChanged(this, offset, size);
}

void HexBedDocument::impose(bufoffset offset, const_bytespan data) {
    if (readOnly()) return;
    bufsize z = HexBedDocument::size(), ds = data.size();
    if (offset + ds > z) {
        grow();
        bufsize lo = z - offset;
        auto token = addUndoReplaceDiffSize(offset, lo, ds);
        trebleReplaceDiffSize(offset, lo, ds, data.data());
        token.commit();
    } else {
        auto token = addUndoReplaceMany(offset, ds);
        treble_.replace(offset, ds, data.data());
        token.commit();
    }
    dirty_ = true;
    context_->announceUndoChange(this);
    context_->announceBytesChanged(this, offset, ds);
}

void HexBedDocument::replace(bufoffset offset, bufsize size, bufsize newsize,
                             byte v) {
    if (!size) {
        insert(offset, newsize, v);
    } else if (!newsize) {
        remove(offset, size);
    } else if (newsize == size) {
        impose(offset, size, v);
    } else {
        auto token = addUndoReplaceDiffSize(offset, size, newsize);
        trebleReplaceDiffSize(offset, size, newsize, v);
        token.commit();
        context_->announceUndoChange(this);
        context_->announceBytesChanged(this, offset);
    }
}

void HexBedDocument::replace(bufoffset offset, bufsize size,
                             const_bytespan data) {
    bufsize newsize = data.size();
    if (!size) {
        insert(offset, data);
    } else if (!newsize) {
        remove(offset, size);
    } else if (newsize == size) {
        impose(offset, data);
    } else {
        auto token = addUndoReplaceDiffSize(offset, size, newsize);
        trebleReplaceDiffSize(offset, size, newsize, data.data());
        token.commit();
        context_->announceUndoChange(this);
        context_->announceBytesChanged(this, offset);
    }
}

void HexBedDocument::insert(bufoffset offset, bufsize size, byte value) {
    if (readOnly()) return;
    grow();
    auto token = addUndoInsert(offset, size);
    treble_.insert(offset, size, value);
    token.commit();
    dirty_ = true;
    context_->announceUndoChange(this);
    context_->announceBytesChanged(this, offset);
}

void HexBedDocument::insert(bufoffset offset, const_bytespan data) {
    if (readOnly()) return;
    grow();
    auto token = addUndoInsert(offset, data.size());
    treble_.insert(offset, data.size(), data.data());
    token.commit();
    dirty_ = true;
    context_->announceUndoChange(this);
    context_->announceBytesChanged(this, offset);
}

void HexBedDocument::remove(bufoffset offset, bufsize size) {
    if (readOnly()) return;
    auto token = addUndoRemove(offset, size);
    treble_.remove(offset, size);
    token.commit();
    dirty_ = true;
    context_->announceUndoChange(this);
    context_->announceBytesChanged(this, offset);
}

void HexBedDocument::impose(bufoffset offset, byte value) {
    impose(offset, 1, value);
}

void HexBedDocument::insert(bufoffset offset, byte value) {
    insert(offset, 1, value);
}

void HexBedDocument::remove(bufoffset offset) { remove(offset, 1); }

bufsize HexBedDocument::size() const noexcept { return treble_.size(); }

bool HexBedDocument::filed() const noexcept { return !filename_.empty(); }

bool HexBedDocument::unsaved() const noexcept { return dirty_; }

std::string HexBedDocument::path() const { return filename_; }

bool HexBedDocument::readOnly() const noexcept { return readOnly_; }

void HexBedDocument::discard() {
    buffer_ = bufferOpen(filename_);
    treble_.clear(buffer_->size());
    dirty_ = false;
    context_->announceFileChanged(this);
}

void HexBedDocument::commit() {
    if (readOnly()) return;
    truncateUndo();
    for (auto& undoEntry : undos_) undoEntry.detach();
    auto lambda = [this](VirtualBuffer& vbuf) {
        treble_.write(vbuf, 0, BUFSIZE_MAX);
    };
    if (treble_.isCleanOverlay())
        buffer_->writeOverlay(lambda, filename_);
    else
        buffer_->write(lambda, filename_);
    discard();
}

void HexBedDocument::commitAs(const std::string& filename) {
    truncateUndo();
    for (auto& undoEntry : undos_) undoEntry.detach();
    buffer_->writeNew(
        [this](VirtualBuffer& vbuf) { treble_.write(vbuf, 0, BUFSIZE_MAX); },
        filename);
    filename_ = filename;
    readOnly_ = false;
    discard();
}

void HexBedDocument::commitTo(const std::string& filename) {
    buffer_->writeCopy(
        [this](VirtualBuffer& vbuf) { treble_.write(vbuf, 0, BUFSIZE_MAX); },
        filename);
}

void HexBedUndoEntry::detach() { oldStripes.clear(); }

};  // namespace hexbed
