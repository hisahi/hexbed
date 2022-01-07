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
// file/treble.cc -- impl for the treble

#include "file/treble.hh"

#include <cmath>
#if TREBLE_DEBUG
#include <iostream>
#endif

#include "common/logger.hh"

namespace hexbed {

constexpr bufsize INSERT_SUBBLOCK_THRESHOLD = 65536;
constexpr bufsize COMPACT_MULTIPLICATIVE_THRESHOLD = 3;
constexpr bufsize COMPACT_ADDITIVE_THRESHOLD = 256;

template <bool newRegion>
static constexpr bufsize roundCapacity(bufsize l) {
    return newRegion ? std::max<bufsize>(l, sizeof(int)) : l;
}

template <class T>
requires std::unsigned_integral<T>
static T intsqrt(T v) { return T(std::sqrt(v)); }

static bufsize newCapacity(bufsize l, bufsize add) {
    return roundCapacity<false>(std::max(l + add, l + intsqrt(l)));
}

template <typename T>
static void renew(std::unique_ptr<T[]>& p, size_t n0, size_t n1) {
    auto arr = std::make_unique_for_overwrite<T[]>(n1);
    std::move(p.get(), p.get() + std::min(n0, n1), arr.get());
    p = std::move(arr);
}

template <typename T>
static bool renewShrink(std::unique_ptr<T[]>& p, size_t n0, size_t n1) {
    T* a = new (std::nothrow) T[n1];
    if (a) {
        auto arr = std::unique_ptr<T[]>(a);
        std::move(p.get(), p.get() + std::min(n0, n1), arr.get());
        p = std::move(arr);
        return true;
    }
    return false;
}

bool TrebleNode::isRoot() const noexcept { return !parent; }

bool TrebleNode::isLeftChild() const noexcept {
    return parent && parent->left() == this;
}

bool TrebleNode::isRightChild() const noexcept {
    return parent && parent->right() == this;
}

const TrebleNode* TrebleNode::minimum() const noexcept {
    const TrebleNode* n = this;
    const TrebleNode* c;
    while ((c = n->left())) n = c;
    return n;
}

const TrebleNode* TrebleNode::maximum() const noexcept {
    const TrebleNode* n = this;
    const TrebleNode* c;
    while ((c = n->right())) n = c;
    return n;
}

const TrebleNode* TrebleNode::predecessor() const noexcept {
    if (left_) return left()->maximum();
    const TrebleNode* n = this;
    const TrebleNode* p;
    while ((p = n->parent) && p->left() == n) n = p;
    return p;
}

const TrebleNode* TrebleNode::successor() const noexcept {
    if (right_) return right()->minimum();
    const TrebleNode* n = this;
    const TrebleNode* p;
    while ((p = n->parent) && p->right() == n) n = p;
    return p;
}

Treble::Treble(bufsize size)
    : root_(std::make_unique<TrebleNode>(nullptr, size)), total_(size) {}

Treble::iterator Treble::root() noexcept { return iterator(root_.get()); }

#if TREBLE_DEBUG
static void printTreble(TrebleNode* node, unsigned depth = 0) {
    std::string pref(depth, ' ');
    if (!depth) std::cerr << "=========== TREBLE DEBUG ===========\n";
    std::cerr << pref << "| " << node
              << "\tBF=" << static_cast<int>(node->balance())
              << "\tLL=" << node->leftlen << "\tO=" << !node->data
              << "\tL=" << node->length << "\tP=" << node->offset << "\n";
    std::cerr << pref << "\\ Left";
    if (node->left()) {
        std::cerr << "\n";
        HEXBED_ASSERT(node->left()->parent == node, "broken parent link");
        printTreble(node->left(), depth + 2);
    } else
        std::cerr << " ---";
    std::cerr << "\n";
    std::cerr << pref << "\\ Right";
    if (node->right()) {
        std::cerr << "\n";
        HEXBED_ASSERT(node->right()->parent == node, "broken parent link");
        printTreble(node->right(), depth + 2);
    } else
        std::cerr << " ---";
    std::cerr << "\n";
}
#define PRINT_TREBLE() printTreble(root_.get())
#else
#define PRINT_TREBLE() HEXBED_NOOP
#endif

void Treble::clear(bufsize size) {
    root_ = std::make_unique<TrebleNode>(nullptr, size);
    total_ = size;
}

TrebleFindResult Treble::find(bufsize index) const noexcept {
    TrebleNode* node = root_.get();
    do {
        if (index < node->leftlen)
            node = node->left();
        else {
            index -= node->leftlen;
            if (index < node->length)
                return TrebleFindResult{iterator(node), index};
            index -= node->length;
            node = node->right();
        }
    } while (node);
    return TrebleFindResult{iterator(node), index};
}

bool Treble::isCleanOverlay_(TrebleNode* node, bufsize offset) const noexcept {
    if (node->leftlen) {
        HEXBED_ASSERT(node->left_);
        if (!isCleanOverlay_(node->left(), offset)) return false;
        offset += node->leftlen;
    }
    if (!node->data && node->offset != offset) return false;
    offset += node->length;
    return !node->right_ || isCleanOverlay_(node->right(), offset);
}

bool Treble::isCleanOverlay() const noexcept {
    return isCleanOverlay_(root_.get(), 0);
}

template <typename T>
static void rotate3(T& a, T& b, T& c) {
    /* (a, b, c = b, c, a) */
    /*
    T tmp = std::move(a);
    a = std::move(b);
    b = std::move(c);
    c = std::move(tmp);
    */
    std::swap(a, b);
    std::swap(b, c);
}

bool Treble::isRightChild(TrebleNode* parent, TrebleNode* node) {
    return parent->right() == node;
}

std::unique_ptr<TrebleNode>& Treble::getParentLink(TrebleNode* parent,
                                                   TrebleNode* node) {
    HEXBED_ASSERT(node->parent == parent);
    return !parent                      ? root_
           : isRightChild(parent, node) ? parent->right_
                                        : parent->left_;
}

int Treble::swing(TrebleNode* parent, TrebleNode* node) {
    return isRightChild(parent, node) ? 1 : -1;
}

TrebleNode* Treble::rotateL(TrebleNode* node) {
    // rotate left. if node is the root, node->right becomes the new root
    TrebleNode* parent = node->parent;
    TrebleNode* child = node->right();
    TrebleNode* sub = child->left();
    std::unique_ptr<TrebleNode>& plink = getParentLink(parent, node);
    rotate3(plink, node->right_, child->left_);
    if (sub)
        rotate3(sub->parent, child->parent, node->parent);
    else
        child->parent = std::exchange(node->parent, child);
    child->leftlen += node->leftlen + node->length;
    return child;
}

TrebleNode* Treble::rotateR(TrebleNode* node) {
    // rotate right. if node is the root, node->left becomes the new root
    TrebleNode* parent = node->parent;
    TrebleNode* child = node->left();
    TrebleNode* sub = child->right();
    std::unique_ptr<TrebleNode>& plink = getParentLink(parent, node);
    rotate3(plink, node->left_, child->right_);
    if (sub)
        rotate3(sub->parent, child->parent, node->parent);
    else
        child->parent = std::exchange(node->parent, child);
    node->leftlen -= child->leftlen + child->length;
    return child;
}

TrebleNode* Treble::rotateLL(TrebleNode* node) {
    TrebleNode* child = rotateL(node);
    if (!child->balance()) {
        node->balance(1);
        child->balance(-1);
    } else {
        node->balance(0);
        child->balance(0);
    }
    return child;
}

TrebleNode* Treble::rotateRR(TrebleNode* node) {
    TrebleNode* child = rotateR(node);
    if (!child->balance()) {
        node->balance(-1);
        child->balance(1);
    } else {
        node->balance(0);
        child->balance(0);
    }
    return child;
}

TrebleNode* Treble::rotateLR(TrebleNode* node) {
    TrebleNode* child = node->left();
    TrebleNode* sub = rotateL(child);
    rotateR(node);
    int bal = sub->balance();
    node->balance(bal >= 0 ? 0 : 1);
    child->balance(bal <= 0 ? 0 : -1);
    sub->balance(0);
    return sub;
}

TrebleNode* Treble::rotateRL(TrebleNode* node) {
    TrebleNode* child = node->right();
    TrebleNode* sub = rotateR(child);
    rotateL(node);
    int bal = sub->balance();
    node->balance(bal <= 0 ? 0 : -1);
    child->balance(bal >= 0 ? 0 : 1);
    sub->balance(0);
    return sub;
}

template <bool incr>
void Treble::balance(TrebleNode* node, int swing) {
    TrebleNode* parent;
    while (node) {
        int bf = node->balance() + swing;
        switch (bf) {
        case 0:
        case -1:
        case 1:
            node->balance(bf);
            break;
        case -2:  // too left-heavy
            HEXBED_ASSERT(node->left(), "no left child on left-heavy tree?");
            if (node->left()->balance() > 0)
                node = rotateLR(node);
            else
                node = rotateRR(node);
            break;
        case 2:  // too right-heavy
            HEXBED_ASSERT(node->right(), "no right child on right-heavy tree?");
            if (node->right()->balance() < 0)
                node = rotateRL(node);
            else
                node = rotateLL(node);
            break;
        }
        if (!bf) break;
        parent = node->parent;
        if (!parent) break;
        swing = (incr ? 1 : -1) * Treble::swing(parent, node);
        node = parent;
    }
}

void Treble::insertLeft(TrebleNode* node, std::unique_ptr<TrebleNode>&& child) {
    HEXBED_ASSERT(!node->left_);
    HEXBED_ASSERT(child->parent == node);
    node->left(std::move(child));
    balance<true>(node, -1);
}

void Treble::insertRight(TrebleNode* node,
                         std::unique_ptr<TrebleNode>&& child) {
    HEXBED_ASSERT(!node->right_);
    HEXBED_ASSERT(child->parent == node);
    node->right(std::move(child));
    balance<true>(node, 1);
}

void Treble::propagate_(TrebleNode* node, bufdiff d) {
    TrebleNode* p;
    while ((p = node->parent)) {
        if (isRightChild(p, node)) return;
        p->leftlen += d;
        node = p;
    }
}

void Treble::propagate(TrebleNode* node, bufsize was, bufsize now) {
    propagate_(node, static_cast<bufdiff>(now) - static_cast<bufdiff>(was));
}

// returns the successor of the erased node
TrebleNode* Treble::erase(TrebleNode* node) {
    TrebleNode* parent = node->parent;
    // node was the left child?
    bool left = parent && !isRightChild(parent, node);
    std::unique_ptr<TrebleNode>& plink = getParentLink(parent, node);
    // successor
    TrebleNode* succ;
    // temporary pointer holder for removed node
    std::unique_ptr<TrebleNode> owner;

    if (left && node->length) propagate(parent, node->length, 0);

    if (node->right_) {
        TrebleNode* child = node->right();
        if (node->left_) {
            succ = child->minimum();
            if (child == succ) {
                // succ->parent == child, node->right == child
                owner = std::exchange(plink, std::move(node->right_));
                child->parent = parent;
                if ((child->left_ = std::move(node->left_)))
                    child->left_->parent = succ;
                child->leftlen = node->leftlen;
                child->balance(node->balance());
                balance<false>(child, -1);
            } else {
                TrebleNode* sp = succ->parent;
                propagate(succ, succ->length, 0);
                owner = std::exchange(
                    plink,
                    std::exchange(
                        sp->left_,
                        std::exchange(succ->right_, std::move(node->right_))));
                if (sp->left_) sp->left_->parent = sp;
                if ((succ->left_ = std::move(node->left_)))
                    succ->left_->parent = succ;
                succ->leftlen = node->leftlen;
                succ->parent = parent;
                succ->balance(node->balance());
                child->parent = succ;
                child->leftlen -= succ->length;
                balance<false>(sp, 1);
            }
        } else {
            child->parent = parent;
            owner = std::exchange(plink, std::move(node->right_));
            succ = child->minimum();
        }
    } else {
        succ = node->successor();
        TrebleNode* child = node->left();
        if (child) child->parent = parent;
        owner = std::exchange(plink, std::move(node->left_));
    }
    if (parent) balance<false>(parent, left ? 1 : -1);
    return succ;
}

TrebleNode* Treble::tryMerge(TrebleNode* node) {
    if (!node) return nullptr;
    TrebleNode* prec = node->predecessor();
    if (prec && !node->data && !prec->data &&
        prec->offset + prec->length == node->offset) {
        node->offset = prec->offset;
        propagate(prec, prec->length, 0);
        node->length += std::exchange(prec->length, 0);
        [[maybe_unused]] TrebleNode* precsucc = erase(prec);
        HEXBED_ASSERT(node == precsucc);
    }
    return node;
}

bytespan Treble::usurp(TrebleNode* node) {
    if (node->data) {
        return bytespan(node->data.get(), node->length);
    } else {
        bufsize zz = roundCapacity<false>(node->length);
        node->data = std::make_unique<byte[]>(zz);
        node->capacity = zz;
        return bytespan(node->data.get(), node->length);
    }
}

bytespan Treble::usurpNew(TrebleNode* node, bufsize z) {
    HEXBED_ASSERT(!node->data);
    bufsize zz = roundCapacity<true>(z);
    node->data = std::make_unique<byte[]>(zz);
    node->capacity = zz;
    if (node->length != z) {
        propagate(node, node->length, z);
        node->length = z;
    }
    return bytespan(node->data.get(), node->length);
}

void Treble::splitLeft(TrebleNode* node, bufsize offset) {
    auto newnode = std::make_unique<TrebleNode>(node, offset);
    if (node->data) {
        bufsize l = node->length;
        bufsize zz = roundCapacity<false>(offset);
        newnode->data = std::make_unique<byte[]>(zz);
        newnode->capacity = zz;
        node->length -= offset;
        byte* s = node->data.get();
        byte* d = newnode->data.get();
        memCopy(d, s, offset);
        memCopy(s, s + offset, l - offset);
        compact(node);
    } else {
        node->length -= offset;
    }
    newnode->offset = node->offset;
    node->offset += offset;
    node->leftlen += newnode->length;
    if (!node->left_) {
        insertLeft(node, std::move(newnode));
        return;
    }
    node = node->left()->maximum();
    newnode->parent = node;
    insertRight(node, std::move(newnode));
}

void Treble::splitRight(TrebleNode* node, bufsize offset, bool adjust,
                        bool move) {
    bufsize l = node->length, z = l - offset;
    auto newnode = std::make_unique<TrebleNode>(node, z);
    if (node->data) {
        if (move && !offset) {
            newnode->data = std::move(node->data);
            newnode->capacity = std::exchange(node->capacity, 0);
        } else {
            bufsize zz = roundCapacity<false>(z);
            newnode->data = std::make_unique<byte[]>(zz);
            newnode->capacity = zz;
            byte* s = node->data.get();
            memCopy(newnode->data.get(), s + offset, l - offset);
        }
        node->length = offset;
        compact(node);
    } else {
        node->length = offset;
    }
    newnode->offset = adjust ? node->offset + offset : node->offset;
    if (!node->right_) {
        insertRight(node, std::move(newnode));
        return;
    }
    node = node->right()->minimum();
    propagate(node, 0, newnode->length);
    newnode->parent = node;
    insertLeft(node, std::move(newnode));
}

void Treble::compact(TrebleNode* node) {
    if (node->capacity > node->length * COMPACT_MULTIPLICATIVE_THRESHOLD ||
        node->capacity > node->length + COMPACT_ADDITIVE_THRESHOLD) {
        bufsize zz = roundCapacity<false>(node->length);
        if (renewShrink(node->data, node->capacity, zz)) node->capacity = zz;
    }
}

struct FillFeeder {
    byte v;
    template <typename It>
    void operator()(It begin, It end) {
        memFill(&*begin, v, end - begin);
    }
};

struct CopyFeeder {
    const byte* p;
    template <typename It>
    void operator()(It begin, It end) {
        p += memCopy(&*begin, p, end - begin);
    }
};

struct RepeatFeeder {
    bufsize sn;
    const byte* sb;
    bufsize so;
    template <typename It>
    void operator()(It begin, It end) {
        byte* p0 = &*begin;
        byte* p1 = &*end;
        bufsize n;
        // if (p0 >= p1) return;
        if (so) {
            bufsize pd = p1 - p0, sd = sn - so;
            if (pd < sd) {
                n = memCopy(p0, sb + so, pd);
                p0 += n;
                so += n;
                return;
            }
            p0 += memCopy(p0, sb + so, sd);
        }
        so = (p1 - p0) % sn;
        memFillRepeat(p0, sn, sb, p1 - p0);
    }
};

template <typename Feeder>
void Treble::replace_(bufsize index, bufsize count, Feeder& f) {
    if (!count) return;
    TrebleFindResult res = find(index);
    TrebleNode* node = res.it.get();
    HEXBED_ASSERT(node, "trying to replace beyond file!");
    if (count == node->length) {
        /* replace full block */
        bytespan span = usurp(node);
        f(span.begin(), span.end());
        return;
    }
    if (!node->data) {
        if (res.suboffset) {
            splitLeft(node, res.suboffset);
            res.suboffset = 0;
        } else if (index) {
            TrebleNode* pnode = res.it->predecessor();
            if (pnode && pnode->data) {
                bufsize l = pnode->length;
                bufsize nc = newCapacity(l, count);
                if (pnode->capacity < nc) {
                    renew(pnode->data, pnode->capacity, nc);
                    pnode->capacity = nc;
                }
                byte* d = pnode->data.get() + l;
                pnode->length += count;
                propagate(pnode, 0, count);
                f(d, d + count);
                return;
            }
        }
    }
    while (count) {
        bufsize l = node->length - res.suboffset;
        if (count < l && !node->data) {
            l = res.suboffset + count;
            splitRight(node, l, true);
        }
        bytespan span = usurp(node);
        auto p = span.begin() + res.suboffset;
        f(p, p + l);
        if (count <= l) return;
        count -= l;
        node = (++res.it).get();
        res.suboffset = 0;
    }
}

void Treble::replace(bufsize index, bufsize count, byte v) {
    FillFeeder feeder{v};
    replace_(index, count, feeder);
    PRINT_TREBLE();
}

void Treble::replace(bufsize index, bufsize count, const byte* data) {
    CopyFeeder feeder{data};
    replace_(index, count, feeder);
    PRINT_TREBLE();
}

void Treble::replace(bufsize index, bufsize count, bufsize scount,
                     const byte* sdata, bufsize soffset) {
    RepeatFeeder feeder{scount, sdata, soffset};
    replace_(index, count, feeder);
    PRINT_TREBLE();
}

template <typename Feeder>
void Treble::insert_(bufsize index, bufsize count, Feeder& f) {
    if (!count) return;
    HEXBED_ASSERT(index <= total_, "no inserting beyond end of file");
    byte* d;
    TrebleNode* node;
    if (index == total_) {
        // inserting new data to the end of the file
        node = root_->maximum();
        if (!node->data) {
            auto newnode = std::make_unique<TrebleNode>(node, count);
            bufsize zz = roundCapacity<true>(count);
            newnode->offset = index;
            newnode->data = std::make_unique<byte[]>(zz);
            newnode->capacity = zz;
            d = newnode->data.get();
            insertRight(node, std::move(newnode));
        } else {
            bufsize l = node->length;
            bufsize nc = newCapacity(l, count);
            if (node->capacity < nc) {
                renew(node->data, node->capacity, nc);
                node->capacity = nc;
            }
            d = node->data.get() + l;
            node->length += count;
        }
        f(d, d + count);
        return;
    }
    bool zero = !index;
    TrebleFindResult res = find(zero ? index : index - 1);
    if (!zero) ++res.suboffset;
    node = res.it.get();
    if (!node->data && res.suboffset == node->length) {
        TrebleNode* node2 = node->successor();
        if (node2->data && node2->length < INSERT_SUBBLOCK_THRESHOLD) {
            node = node2;
        }
    }
    if (!node->data) {
        if (res.suboffset) splitLeft(node, res.suboffset);
        if (node->length) splitRight(node, 0, false);
        bytespan span = usurpNew(node, count);
        f(span.begin(), span.end());
        return;
    }
    if (node->length - res.suboffset < INSERT_SUBBLOCK_THRESHOLD) {
        bufsize l = node->length;
        bufsize nc = newCapacity(l, count);
        if (node->capacity < nc) {
            renew(node->data, node->capacity, nc);
            node->capacity = nc;
        }
        byte* p = node->data.get();
        d = p + res.suboffset;
        memCopyBack(d + count, d, l - res.suboffset);
    } else {
        splitRight(node, res.suboffset, false);
        bufsize l = res.suboffset;
        bufsize nc = newCapacity(l, count);
        if (node->capacity < nc) {
            renew(node->data, node->capacity, nc);
            node->capacity = nc;
        }
        d = node->data.get() + l;
    }
    node->length += count;
    propagate(node, 0, count);
    f(d, d + count);
}

void Treble::insert(bufsize index, bufsize count, byte v) {
    FillFeeder feeder{v};
    insert_(index, count, feeder);
    total_ += count;
    PRINT_TREBLE();
}

void Treble::insert(bufsize index, bufsize count, const byte* data) {
    CopyFeeder feeder{data};
    insert_(index, count, feeder);
    total_ += count;
    PRINT_TREBLE();
}

void Treble::insert(bufsize index, bufsize count, bufsize scount,
                    const byte* sdata, bufsize soffset) {
    RepeatFeeder feeder{scount, sdata, soffset};
    insert_(index, count, feeder);
    total_ += count;
    PRINT_TREBLE();
}

void Treble::reinsert(bufsize index, bufsize count, bufsize offset) {
    if (!count) return;
    if (index == total_) {
        // inserting old data to the end of the file
        TrebleFindResult res = find(index - 1);
        TrebleNode* node = res.it.get();
        if (!node->data && node->offset + node->length == offset) {
            node->length += count;
            return;
        }
        node = root_->maximum();
        auto newnode = std::make_unique<TrebleNode>(node, count);
        newnode->offset = offset;
        insertRight(node, std::move(newnode));
        return;
    }
    bool zero = !index;
    TrebleFindResult res = find(zero ? index : index - 1);
    if (!zero) ++res.suboffset;
    TrebleNode* node = res.it.get();
    if (!node->data && node->length == res.suboffset &&
        offset == node->offset + node->length) {
        node->length += count;
    } else if (!res.suboffset && !node->data &&
               count + offset == node->offset) {
        node->offset = offset;
        node->length += count;
    } else {
        if (res.suboffset) splitLeft(node, res.suboffset);
        if (node->length) splitRight(node, 0, false, true);
        node->length = count;
        node->offset = offset;
    }
    propagate(node, 0, count);
    total_ += count;
    TrebleNode* succ = node->successor();
    if (succ) tryMerge(succ);
    PRINT_TREBLE();
}

void Treble::revert(bufsize index, bufsize count) {
    if (!count) return;
    TrebleFindResult res = find(index);
    TrebleNode* node = res.it.get();
    HEXBED_ASSERT(node, "trying to revert beyond file!");
    while (count) {
        if (!node->data) {
            bufsize z = node->length - res.suboffset;
            if (count <= z) break;
            count -= z;
        } else {
            if (res.suboffset) splitLeft(node, res.suboffset);
            bufsize z = node->length;
            if (count < z) {
                splitRight(node, count, true);
                count = 0;
            } else
                count -= z;
            node->data = nullptr;
            node = tryMerge(node);
        }
        node = node->successor();
        res.suboffset = 0;
    }
    PRINT_TREBLE();
}

void Treble::remove(bufsize index, bufsize count) {
    if (!count) return;
    TrebleFindResult res = find(index);
    TrebleNode* node = res.it.get();
    bufsize removed = 0;
    HEXBED_ASSERT(node, "trying to remove beyond file!");
    while (count) {
        if (res.suboffset) splitLeft(node, res.suboffset);
        bufsize z = node->length;
        if (count < z) {
            node->offset += count;
            node->length -= count;
            removed += count;
            if (node->data) {
                byte* p = node->data.get();
                memCopy(p, p + count, z - count);
                compact(node);
            }
            propagate(node, count, 0);
            break;
        } else {
            node = tryMerge(erase(node));
            removed += z;
            count -= z;
            HEXBED_ASSERT(node || !count, "trying to remove beyond file!");
        }
        res.suboffset = 0;
    }
    tryMerge(node);
    total_ -= removed;
    PRINT_TREBLE();
}

};  // namespace hexbed
