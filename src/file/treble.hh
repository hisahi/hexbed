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
// file/treble.hh -- header for the treble

#ifndef HEXBED_FILE_TRABLE_HH
#define HEXBED_FILE_TRABLE_HH

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <limits>
#include <memory>

#include "common/aligneduniqueptr.hh"
#include "common/logger.hh"
#include "common/memory.hh"
#include "common/specs.hh"
#include "common/types.hh"

namespace hexbed {

struct TrebleNode;
class Treble;

class MockTrebleNode {
  private:
    TrebleNode* left_{nullptr};
    TrebleNode* right_{nullptr};
    TrebleNode* parent_{nullptr};
    byte* data_{nullptr};
    bufsize length_{0};
    bufsize leftlen_{0};
    bufsize offset_{0};
    std::size_t capacity_{0};

    MockTrebleNode();
};

namespace internal {

inline constexpr int toTrebleNodeBalance(bufsize z) {
    return static_cast<int>(z & 3);
}

inline constexpr bufsize toTrebleNodeCapacity(bufsize z) { return z & ~3; }

inline constexpr bufsize roundToTrebleNodeCapacity(bufsize z) {
    return toTrebleNodeCapacity(z + 3);
}

};  // namespace internal

using TrebleNodePointer =
    AlignedUniquePtr<TrebleNode,
                     std::bit_ceil<std::size_t>(sizeof(MockTrebleNode))>;
using TrebleDataPointer = std::unique_ptr<byte[]>;

struct TrebleNode {
    inline TrebleNode(TrebleNode* parent, bufsize length)
        : parent_(parent), length_(length) {}
    inline TrebleNode(TrebleNode* parent, bufsize length, byte value)
        : parent_(parent),
          data_(makeUniqueOf<TrebleDataPointer>(
              internal::roundToTrebleNodeCapacity(length))),
          length_(length),
          capacity_(internal::roundToTrebleNodeCapacity(length)) {
        std::fill(data_.get(), data_.get() + length_, value);
    }
    inline TrebleNode(TrebleNode* parent, bufsize length, const byte* src)
        : parent_(parent),
          data_(makeUniqueOf<TrebleDataPointer>(
              internal::roundToTrebleNodeCapacity(length))),
          length_(length),
          capacity_(internal::roundToTrebleNodeCapacity(length)) {
        std::copy(src, src + length, data_.get());
    }

    inline int balance() const noexcept {
        switch (internal::toTrebleNodeBalance(capacity_)) {
        case 0:
            return 0;
        case 1:
            return 1;
        default:
        case 2:
            HEXBED_UNREACHABLE();
            return 0;
        case 3:
            return -1;
        }
    }

    inline void balance(int balance) noexcept {
        HEXBED_ASSERT(balance == -1 || balance == 0 || balance == 1);
        capacity_ = internal::toTrebleNodeCapacity(capacity_);
        switch (balance) {
        case 0:
            break;
        case 1:
            capacity_ |= 1;
            break;
        case -1:
            capacity_ |= 3;
            break;
        default:
            HEXBED_UNREACHABLE();
        }
    }

    inline TrebleNode* left() const noexcept { return left_.get(); }
    inline TrebleNode* right() const noexcept { return right_.get(); }
    inline TrebleNode* left(std::nullptr_t) noexcept {
        left_ = nullptr;
        return nullptr;
    }
    inline TrebleNode* right(std::nullptr_t) noexcept {
        right_ = nullptr;
        return nullptr;
    }
    inline TrebleNode* left(TrebleNodePointer&& node) noexcept {
        left_ = std::move(node);
        return left_.get();
    }
    inline TrebleNode* right(TrebleNodePointer&& node) noexcept {
        right_ = std::move(node);
        return right_.get();
    }
    inline TrebleNodePointer& leftLink() noexcept { return left_; }
    inline TrebleNodePointer& rightLink() noexcept { return right_; }

    inline bufsize length() const noexcept { return length_; }
    inline bufsize length(bufsize n) noexcept { return length_ = n; }
    inline bufsize lengthAdd(bufsize n) noexcept { return length_ += n; }
    inline bufsize lengthSub(bufsize n) noexcept { return length_ -= n; }

    inline bufsize leftlen() const noexcept { return leftlen_; }
    inline bufsize leftlen(bufsize n) noexcept { return leftlen_ = n; }
    inline bufsize leftlenAdd(bufsize n) noexcept { return leftlen_ += n; }
    inline bufsize leftlenSub(bufsize n) noexcept { return leftlen_ -= n; }

    inline TrebleNode* parent() const noexcept { return parent_; }
    inline void parent(TrebleNode* p) noexcept { parent_ = p; }
    inline TrebleNode*& parentLink() noexcept { return parent_; }

    inline bufsize offset() const noexcept { return offset_; }
    inline bufsize offset(bufsize n) noexcept { return offset_ = n; }
    inline bufsize offsetAdd(bufsize n) noexcept { return offset_ += n; }
    inline bufsize offsetSub(bufsize n) noexcept { return offset_ -= n; }

    inline std::size_t capacity() const noexcept {
        return internal::toTrebleNodeCapacity(capacity_);
    }
    inline std::size_t capacity(std::size_t n) noexcept {
        HEXBED_ASSERT(!(n & 3));
        capacity_ = n | (capacity_ & 3);
        return n;
    }

    inline byte* data() const noexcept { return data_.get(); }
    inline void data(TrebleDataPointer&& ptr) noexcept {
        data_ = std::move(ptr);
    }
    inline void dataMove(TrebleNode& node) noexcept {
        data_ = std::move(node.data_);
    }

    bool isRoot() const noexcept;
    bool isLeftChild() const noexcept;
    bool isRightChild() const noexcept;

    const TrebleNode* minimum() const noexcept;
    const TrebleNode* maximum() const noexcept;
    const TrebleNode* predecessor() const noexcept;
    const TrebleNode* successor() const noexcept;

    inline TrebleNode* minimum() noexcept {
        return const_cast<TrebleNode*>(
            const_cast<const TrebleNode*>(this)->minimum());
    }
    inline TrebleNode* maximum() noexcept {
        return const_cast<TrebleNode*>(
            const_cast<const TrebleNode*>(this)->maximum());
    }
    inline TrebleNode* predecessor() noexcept {
        return const_cast<TrebleNode*>(
            const_cast<const TrebleNode*>(this)->predecessor());
    }
    inline TrebleNode* successor() noexcept {
        return const_cast<TrebleNode*>(
            const_cast<const TrebleNode*>(this)->successor());
    }

  private:
    TrebleNodePointer left_{nullptr};
    TrebleNodePointer right_{nullptr};
    TrebleNode* parent_{nullptr};
    TrebleDataPointer data_{nullptr};
    bufsize length_{0};
    bufsize leftlen_{0};
    bufsize offset_{0};
    std::size_t capacity_{0};
};

class TrebleIterator {
  public:
    using iterator = TrebleIterator;
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = TrebleNode;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using pointer = value_type*;

    inline TrebleIterator() noexcept : node_(nullptr) {}
    inline TrebleIterator(pointer node) noexcept : node_(node) {}
    inline TrebleIterator(const iterator& copy) noexcept : node_(copy.node_) {}
    inline TrebleIterator(iterator&& move) noexcept : node_(move.node_) {}
    inline ~TrebleIterator() {}

    inline iterator& operator=(pointer ptr) noexcept {
        node_ = ptr;
        return *this;
    }
    inline iterator& operator=(const iterator& copy) noexcept {
        node_ = copy.node_;
        return *this;
    }
    inline iterator& operator=(iterator&& move) noexcept {
        node_ = move.node_;
        return *this;
    }

    inline TrebleIterator& operator++() noexcept {
        node_ = node_->successor();
        return *this;
    }

    inline TrebleIterator operator++(int) noexcept {
        TrebleIterator copy(*this);
        ++*this;
        return copy;
    }

    inline TrebleIterator& operator--() noexcept {
        node_ = node_->predecessor();
        return *this;
    }

    inline TrebleIterator operator--(int) noexcept {
        TrebleIterator copy(*this);
        --*this;
        return copy;
    }

    inline TrebleIterator previous() noexcept {
        return --TrebleIterator(*this);
    }

    inline TrebleIterator next() noexcept { return ++TrebleIterator(*this); }

    inline reference operator*() noexcept { return *node_; }
    inline pointer operator->() noexcept { return node_; }
    inline pointer get() noexcept { return node_; }

    inline bool operator==(const iterator& rhs) noexcept {
        return node_ == rhs.node_;
    }
    inline bool operator!=(const iterator& rhs) noexcept {
        return node_ != rhs.node_;
    }

  private:
    TrebleNode* node_;
};

struct TrebleReadByteResult {
    byte value;
    bool original;
};

struct TrebleFindResult {
    TrebleIterator it;
    bufsize suboffset;
};

class VirtualBuffer {
  public:
    virtual void raw(bufsize n, const byte* r) = 0;
    virtual void copy(bufsize n, bufsize o) = 0;
};

template <typename T>
class PtrWriteBuffer {
  public:
    inline PtrWriteBuffer(byte* p, T& t) : ptr_(p), ref_(t) {}
    inline void raw(bufsize n, const byte* r) {
        memCopy(ptr_, r, n);
        ptr_ += n;
    }
    inline void copy(bufsize n, bufsize o) {
        ptr_ += ref_.read(o, bytespan(ptr_, n));
    }

  private:
    byte* ptr_;
    T& ref_;
};

class VirtualWriteBuffer {
  public:
    inline VirtualWriteBuffer(VirtualBuffer& b) : buf_(b) {}
    inline void raw(bufsize n, const byte* r) { buf_.raw(n, r); }
    inline void copy(bufsize n, bufsize o) { buf_.copy(n, o); }

  private:
    VirtualBuffer& buf_;
};

class Treble {
  public:
    using value_type = byte;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = byte&;
    using const_reference = const byte&;
    using pointer = byte*;
    using const_pointer = const byte*;
    using iterator = TrebleIterator;

    Treble(bufsize size);

    iterator root() noexcept;
    TrebleFindResult find(bufsize index) const noexcept;
    TrebleReadByteResult readByte(bufsize offset) const noexcept;
    bool isCleanOverlay() const noexcept;
    inline bufsize size() const noexcept { return total_; }

    void replace(bufsize index, bufsize count, byte v);
    void replace(bufsize index, bufsize count, const byte* data);
    void replace(bufsize index, bufsize count, bufsize scount,
                 const byte* sdata, bufsize soffset);
    void revert(bufsize index, bufsize count);

    void insert(bufsize index, bufsize count, byte v);
    void insert(bufsize index, bufsize count, const byte* data);
    void insert(bufsize index, bufsize count, bufsize scount, const byte* sdata,
                bufsize soffset);
    void reinsert(bufsize index, bufsize count, bufsize offset);
    void remove(bufsize index, bufsize count);

    void clear(bufsize newSize);

    template <typename T>
    TrebleReadByteResult readByte(T& in, bufsize index) const noexcept {
        TrebleNode* node = root_.get();
        do {
            if (index < node->leftlen())
                node = node->left();
            else {
                index -= node->leftlen();
                if (index < node->length()) {
                    if (node->data())
                        return TrebleReadByteResult{node->data()[index], false};
                    else
                        return TrebleReadByteResult{in(index), true};
                }
                index -= node->length();
                node = node->right();
            }
        } while (node);
        HEXBED_ASSERT(0, "readByte beyond file");
        return TrebleReadByteResult{0, false};
    }

    template <typename T>
    bufsize render(T& out, bufsize off, bufsize n,
                   const TrebleNode* node) const {
        bufsize w = 0, l;
        if (off < node->leftlen()) {
            HEXBED_ASSERT(node->left());
            l = render(out, off, n, node->left());
            w += l, n -= l;
            if (!n) return w;
            off = 0;
        } else
            off -= node->leftlen();
        if (off < node->length()) {
            l = std::min(node->length() - off, n);
            if (node->data())
                out.raw(l, node->data() + off);
            else
                out.copy(l, node->offset() + off);
            w += l, n -= l;
            if (!n) return w;
            off = 0;
        } else
            off -= node->length();
        if (node->right()) {
            l = render(out, off, n, node->right());
            w += l, n -= l;
        }
        return w;
    }

    template <typename T>
    bufsize render(T& out, bufsize off, bufsize n) const {
        return render(out, off, n, root_.get());
    }

    template <typename T>
    bufsize read(T& in, byte* p, bufsize off, bufsize n) const {
        PtrWriteBuffer<T> buf{p, in};
        return render(buf, off, n, root_.get());
    }

    template <typename T>
    bufsize readBackwards(T& in, byte* p, bufsize off, bufsize n) const {
        PtrWriteBuffer<T> buf{p, in};
        return renderBackwards(buf, off, n, root_.get());
    }

    bufsize write(VirtualBuffer& vbuf, bufsize off, bufsize n) const {
        VirtualWriteBuffer buf{vbuf};
        return render(buf, off, n, root_.get());
    }

  private:
    TrebleNodePointer root_;
    bufsize total_;

    bool isCleanOverlay_(TrebleNode* node, bufsize offset) const noexcept;

    template <typename Feeder>
    void replace_(bufsize index, bufsize count, Feeder& f);

    template <typename Feeder>
    void insert_(bufsize index, bufsize count, Feeder& f);

    void propagate_(TrebleNode* node, bufdiff d);
    void propagate(TrebleNode* node, bufsize was, bufsize now);
    bytespan usurp(TrebleNode* node);
    bytespan usurpNew(TrebleNode* node, bufsize z);
    void compact(TrebleNode* node);

    TrebleNode* rotateL(TrebleNode* node);
    TrebleNode* rotateR(TrebleNode* node);

    TrebleNode* rotateLL(TrebleNode* node);
    TrebleNode* rotateRL(TrebleNode* node);
    TrebleNode* rotateLR(TrebleNode* node);
    TrebleNode* rotateRR(TrebleNode* node);

    template <bool incr>
    void balance(TrebleNode* node, int swing);

    void balanceOnInsert(TrebleNode* node, int swing);
    void balanceOnDelete(TrebleNode* node, int swing);

    void insertLeft(TrebleNode* node, TrebleNodePointer&& child);
    void insertRight(TrebleNode* node, TrebleNodePointer&& child);

    bool isRightChild(TrebleNode* parent, TrebleNode* node);
    TrebleNodePointer& getParentLink(TrebleNode* parent, TrebleNode* node);
    int swing(TrebleNode* parent, TrebleNode* node);

    TrebleNode* erase(TrebleNode* node);
    TrebleNode* tryMerge(TrebleNode* node);

    void splitLeft(TrebleNode* node, bufsize offset);
    void splitRight(TrebleNode* node, bufsize offset, bool adjust,
                    bool move = false);
};

};  // namespace hexbed

#endif /* HEXBED_FILE_TRABLE_HH */

/*template <typename T>
bufsize renderBackwards(T& out, bufsize off, bufsize n,
               const TrebleNode* node) const {
    if (!n) return 0;
    bufsize end = off + n - 1;
    TrebleFindResult res = find(end);
    TrebleNode* node = res.it.get();
    if (!node) return 0;
    bufsize w = 0;
    bufsize eoff = res.suboffset + 1;
    while (node) {
        bufsize s = n >= eoff ? 0 : eoff - n, l = eoff - s;
        if (node->data)
            out.raw(l, node->data.get() + s);
        else
            out.copy(l, node->offset + s);
        node = node->predecessor();
        if (node) eoff = node->length;
        w += l, n -= l;
    }
    return w;
}

template <typename T>
bufsize renderBackwards(T& out, bufsize off, bufsize n) const {
    return renderBackwards(out, off, n, root_.get());
}
*/