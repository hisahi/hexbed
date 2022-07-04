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

#ifndef NDEBUG
#define TREBLE_CALL_DEBUG 1
#define TREBLE_TREE_DEBUG 1
#define TREBLE_SANITY_DEBUG 1
#define TREBLE_NOMERGE_DEBUG 0
#endif

#include <cmath>
#if TREBLE_CALL_DEBUG || TREBLE_TREE_DEBUG
#include <iomanip>
#include <iostream>
#endif

#include "common/logger.hh"

namespace hexbed {

constexpr bufsize INSERT_SUBBLOCK_THRESHOLD = 65536;
constexpr bufsize COMPACT_MULTIPLICATIVE_THRESHOLD = 3;
constexpr bufsize COMPACT_ADDITIVE_THRESHOLD = 256;

template <class... Args>
auto newTrebleNode(Args&&... args) -> TrebleNodePointer {
    return makeAlignedUniqueOf<TrebleNodePointer>(std::forward<Args>(args)...);
}

template <bool newRegion>
static constexpr bufsize roundCapacity(bufsize l) {
    bufsize ll = newRegion ? std::max<bufsize>(l, sizeof(int)) : l;
    return internal::roundToTrebleNodeCapacity(ll);
}

template <class T>
requires std::unsigned_integral<T>
static T intsqrt(T v) { return T(std::sqrt(v)); }

static bufsize expandCapacity(bufsize l, bufsize add) {
    return roundCapacity<false>(std::max(l + add, l + intsqrt(l)));
}

/*
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
*/

static TrebleDataPointer newTrebleData(std::size_t c) {
    return makeUniqueOf<TrebleDataPointer>(c);
}

static TrebleDataPointer newTrebleDataNothrow(std::size_t c) {
    return makeUniqueOfNothrow<TrebleDataPointer>(c);
}

static void renewTrebleData(TrebleNode& node, std::size_t nc) {
    auto arr = newTrebleData(nc);
    memCopy(arr.get(), node.data(), std::min<std::size_t>(nc, node.capacity()));
    node.data(std::move(arr));
    node.capacity(nc);
}

static bool renewShrinkTrebleData(TrebleNode& node, std::size_t nc) {
    auto arr = newTrebleDataNothrow(nc);
    if (arr) {
        memCopy(arr.get(), node.data(),
                std::min<std::size_t>(nc, node.capacity()));
        node.data(std::move(arr));
        node.capacity(nc);
        return true;
    }
    return false;
}

bool TrebleNode::isRoot() const noexcept { return !parent_; }

bool TrebleNode::isLeftChild() const noexcept {
    return parent_ && parent_->left() == this;
}

bool TrebleNode::isRightChild() const noexcept {
    return parent_ && parent_->right() == this;
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
    while ((p = n->parent()) && p->left() == n) n = p;
    return p;
}

const TrebleNode* TrebleNode::successor() const noexcept {
    if (right_) return right()->minimum();
    const TrebleNode* n = this;
    const TrebleNode* p;
    while ((p = n->parent()) && p->right() == n) n = p;
    return p;
}

Treble::Treble(bufsize size)
    : root_(newTrebleNode(nullptr, size)), total_(size) {}

Treble::iterator Treble::root() noexcept { return iterator(root_.get()); }

#if TREBLE_TREE_DEBUG
static void printTreblePre(TrebleNode* node, unsigned depth = 0) {
    std::string pref(depth, ' ');
    if (!depth) std::cerr << "=========== TREBLE DEBUG ===========\n";
    std::cerr << pref << "| " << node
              << "\tBF=" << static_cast<int>(node->balance())
              << "\tLL=" << node->leftlen() << "\tEX=" << !!node->data()
              << "\tL=" << node->length() << "\tO=" << node->offset() << "\n";
    std::cerr << pref << "\\ Left";
    if (node->left()) {
        std::cerr << "\n";
        HEXBED_ASSERT(node->left()->parent() == node, "broken parent link");
        printTreblePre(node->left(), depth + 2);
    } else
        std::cerr << " ---";
    std::cerr << "\n";
    std::cerr << pref << "\\ Right";
    if (node->right()) {
        std::cerr << "\n";
        HEXBED_ASSERT(node->right()->parent() == node, "broken parent link");
        printTreblePre(node->right(), depth + 2);
    } else
        std::cerr << " ---";
    std::cerr << "\n";
}

static void printTrebleIn(TrebleNode* node, unsigned depth = 0, int dir = 0) {
    std::string pref(depth, ' ');
    if (!depth) std::cerr << "=========== TREBLE DEBUG ===========\n";
    if (node->left()) {
        HEXBED_ASSERT(node->left()->parent() == node, "broken parent link");
        printTrebleIn(node->left(), depth + 2, -1);
    } else
        std::cerr << pref << "/ ---\n";
    if (dir != 0) {
        std::cerr << std::string(depth - 2, ' ');
        std::cerr << (dir < 0 ? '/' : '\\');
        std::cerr << " + ";
    } else {
        std::cerr << pref << "| ";
    }
    std::cerr << node << "\tBF=" << static_cast<int>(node->balance())
              << "\tLL=" << node->leftlen() << "\tEX=" << !!node->data()
              << "\tL=" << node->length() << "\tO=" << node->offset() << "\n";
    if (node->right()) {
        HEXBED_ASSERT(node->right()->parent() == node, "broken parent link");
        printTrebleIn(node->right(), depth + 2, 1);
    } else
        std::cerr << pref << "\\ ---\n";
    if (!depth) std::cerr << "\n";
}
#define PRINT_TREBLE() printTrebleIn(root_.get())
#else
#define PRINT_TREBLE() HEXBED_NOOP
#endif

#if TREBLE_SANITY_DEBUG
TrebleNode* getRoot(TrebleNode* node) {
    TrebleNode* root = node;
    while (root->parent()) root = root->parent();
    return root;
}

int getDepth(TrebleNode* node) {
    if (!node) return 0;
    return 1 + std::max(getDepth(node->left()), getDepth(node->right()));
}

static void checkTrebleSanity(TrebleNode* node) {
    if (node->left()) {
        TrebleNode* l = node->left();
        HEXBED_ASSERTF(l->parent() == node,
                       "sanity check failed: broken parent link on left child"
                       " (left=%p, node=%p, left.parent=%p)",
                       l, node, l->parent());
        checkTrebleSanity(l);
        bufsize ll = 0;
        do {
            ll += l->leftlen();
            ll += l->length();
        } while ((l = l->right()));
        HEXBED_ASSERTF(node->leftlen() == ll,
                       "sanity check failed: invalid leftlen on node"
                       " (node=%p, expected=%zu, actual=%zu)",
                       node, ll, node->leftlen());
    } else {
        HEXBED_ASSERTF(!node->leftlen(),
                       "sanity check failed: invalid leftlen on node"
                       " (node=%p, expected=0, actual=%zu)",
                       node, node->leftlen());
    }
    if (node->right()) {
        HEXBED_ASSERTF(node->right()->parent() == node,
                       "sanity check failed: broken parent link on right child"
                       " (right=%p, node=%p, right.parent=%p)",
                       node->right(), node, node->right()->parent());
        checkTrebleSanity(node->right());
    }
}

static int getLeftDepth(TrebleNode* node) {
    return node ? getDepth(node->left()) : 0;
}

static int getRightDepth(TrebleNode* node) {
    return node ? getDepth(node->right()) : 0;
}

static int getTrueBalance(TrebleNode* node) {
    return getRightDepth(node) - getLeftDepth(node);
}

#define CHECK_TREBLE_BALANCE(node)                                    \
    HEXBED_ASSERTF(getTrueBalance(node) == node->balance(),           \
                   "sanity check failed: treble balance mismatch"     \
                   " (node=%p, balance=%d, expected=%d, L=%d, R=%d)", \
                   node, node->balance(), getTrueBalance(node),       \
                   getLeftDepth(node), getRightDepth(node));
static void checkTrebleBalance(TrebleNode* node) {
    if (!node) return;
    CHECK_TREBLE_BALANCE(node);
    checkTrebleBalance(node->left());
    checkTrebleBalance(node->right());
}
#if TREBLE_CALL_DEBUG
#define LOG_TREBLE_OK() std::cerr << " = OK\n";
#else
#define LOG_TREBLE_OK() HEXBED_NOOP
#endif
#define CHECK_TREBLE()              \
    checkTrebleSanity(root_.get()); \
    LOG_TREBLE_OK()
#define CHECK_TREBLE_BALANCE_REC() checkTrebleBalance(root_.get())
#else
#define CHECK_TREBLE() HEXBED_NOOP
#define CHECK_TREBLE_BALANCE() HEXBED_NOOP
#define CHECK_TREBLE_BALANCE_REC() HEXBED_NOOP
#endif

#define TREBLE_AFTER_OP() \
    CHECK_TREBLE();       \
    PRINT_TREBLE();       \
    CHECK_TREBLE_BALANCE_REC();

#if TREBLE_CALL_DEBUG
#define L_HEX(v)                                          \
    "0x" << std::hex << std::setw(2) << std::setfill('0') \
         << static_cast<unsigned int>(v) << std::dec
#define L_PTR(v) reinterpret_cast<const void*>(v)
#if TREBLE_SANITY_DEBUG
#define LOG_TREBLE_EOL
#else
#define LOG_TREBLE_EOL << '\n'
#endif
#define LOG_TREBLE(...) \
    std::cerr << "Treble(" << L_PTR(this) << ")->" << __VA_ARGS__ LOG_TREBLE_EOL
#else
#define LOG_TREBLE(...) HEXBED_NOOP
#endif

void Treble::clear(bufsize size) {
    root_ = newTrebleNode(nullptr, size);
    total_ = size;
}

TrebleFindResult Treble::find(bufsize index) const noexcept {
    TrebleNode* node = root_.get();
    do {
        if (index < node->leftlen())
            node = node->left();
        else {
            index -= node->leftlen();
            if (index < node->length())
                return TrebleFindResult{iterator(node), index};
            index -= node->length();
            node = node->right();
        }
    } while (node);
    return TrebleFindResult{iterator(node), index};
}

bool Treble::isCleanOverlay_(TrebleNode* node, bufsize offset) const noexcept {
    if (node->leftlen()) {
        HEXBED_ASSERT(node->left());
        if (!isCleanOverlay_(node->left(), offset)) return false;
        offset += node->leftlen();
    }
    if (!node->data() && node->offset() != offset) return false;
    offset += node->length();
    return !node->right() || isCleanOverlay_(node->right(), offset);
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

TrebleNodePointer& Treble::getParentLink(TrebleNode* parent, TrebleNode* node) {
    HEXBED_ASSERT(node->parent() == parent);
    return !parent                      ? root_
           : isRightChild(parent, node) ? parent->rightLink()
                                        : parent->leftLink();
}

int Treble::swing(TrebleNode* parent, TrebleNode* node) {
    return isRightChild(parent, node) ? 1 : -1;
}

TrebleNode* Treble::rotateL(TrebleNode* node) {
    // rotate left. if node is the root, node->right becomes the new root
    TrebleNode* parent = node->parent();
    TrebleNode* child = node->right();
    TrebleNode* sub = child->left();
    TrebleNodePointer& plink = getParentLink(parent, node);
    rotate3(plink, node->rightLink(), child->leftLink());
    if (sub)
        rotate3(sub->parentLink(), child->parentLink(), node->parentLink());
    else
        child->parent(std::exchange(node->parentLink(), child));
    child->leftlenAdd(node->leftlen() + node->length());
    return child;
}

TrebleNode* Treble::rotateR(TrebleNode* node) {
    // rotate right. if node is the root, node->left becomes the new root
    TrebleNode* parent = node->parent();
    TrebleNode* child = node->left();
    TrebleNode* sub = child->right();
    TrebleNodePointer& plink = getParentLink(parent, node);
    rotate3(plink, node->leftLink(), child->rightLink());
    if (sub)
        rotate3(sub->parentLink(), child->parentLink(), node->parentLink());
    else
        child->parent(std::exchange(node->parentLink(), child));
    node->leftlenSub(child->leftlen() + child->length());
    return child;
}

TrebleNode* Treble::rotateLL(TrebleNode* node) {
    TrebleNode* child = rotateL(node);
    HEXBED_ASSERT(child->balance() >= 0);
    if (!child->balance()) {
        node->balance(1);
        child->balance(-1);
    } else {
        node->balance(0);
        child->balance(0);
    }
    CHECK_TREBLE_BALANCE(node);
    CHECK_TREBLE_BALANCE(child);
    return child;
}

TrebleNode* Treble::rotateRR(TrebleNode* node) {
    TrebleNode* child = rotateR(node);
    HEXBED_ASSERT(child->balance() <= 0);
    if (!child->balance()) {
        node->balance(-1);
        child->balance(1);
    } else {
        node->balance(0);
        child->balance(0);
    }
    CHECK_TREBLE_BALANCE(node);
    CHECK_TREBLE_BALANCE(child);
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
    CHECK_TREBLE_BALANCE(node);
    CHECK_TREBLE_BALANCE(child);
    CHECK_TREBLE_BALANCE(sub);
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
    CHECK_TREBLE_BALANCE(node);
    CHECK_TREBLE_BALANCE(child);
    CHECK_TREBLE_BALANCE(sub);
    return sub;
}

template <bool insert>
void Treble::balance(TrebleNode* node, int swing) {
    TrebleNode* parent;
    while (node) {
        int bf = node->balance() + swing;
        bool proceed;
        switch (bf) {
        case 0:
        case -1:
        case 1:
            node->balance(bf);
            proceed = insert ? bf != 0 : bf == 0;
            break;
        case -2:  // too left-heavy
            HEXBED_ASSERT(node->left(), "no left child on left-heavy tree?");
            if (node->left()->balance() > 0) {
                node = rotateLR(node);
                proceed = !insert;
            } else {
                node = rotateRR(node);
                proceed = !insert && node->balance() <= 0;
            }
            break;
        case 2:  // too right-heavy
            HEXBED_ASSERT(node->right(), "no right child on right-heavy tree?");
            if (node->right()->balance() < 0) {
                node = rotateRL(node);
                proceed = !insert;
            } else {
                node = rotateLL(node);
                proceed = !insert && node->balance() >= 0;
            }
            break;
        }
        CHECK_TREBLE_BALANCE(node);
        if (!proceed || !(parent = node->parent())) break;
        swing = (insert ? 1 : -1) * Treble::swing(parent, node);
        node = parent;
    }
}

void Treble::balanceOnInsert(TrebleNode* node, int swing) {
    balance<true>(node, swing);
}

void Treble::balanceOnDelete(TrebleNode* node, int swing) {
    balance<false>(node, swing);
}

void Treble::insertLeft(TrebleNode* node, TrebleNodePointer&& child) {
    HEXBED_ASSERT(!node->left());
    HEXBED_ASSERT(child->parent() == node);
    node->left(std::move(child));
    balanceOnInsert(node, -1);
}

void Treble::insertRight(TrebleNode* node, TrebleNodePointer&& child) {
    HEXBED_ASSERT(!node->right());
    HEXBED_ASSERT(child->parent() == node);
    node->right(std::move(child));
    balanceOnInsert(node, 1);
}

void Treble::propagate_(TrebleNode* node, bufdiff d) {
    TrebleNode* p;
    while ((p = node->parent())) {
        if (!isRightChild(p, node)) p->leftlenAdd(d);
        node = p;
    }
}

void Treble::propagate(TrebleNode* node, bufsize was, bufsize now) {
    propagate_(node, static_cast<bufdiff>(now) - static_cast<bufdiff>(was));
}

// returns the successor of the erased node
TrebleNode* Treble::erase(TrebleNode* node) {
    TrebleNode* parent = node->parent();
    TrebleNodePointer& plink = getParentLink(parent, node);
    // successor
    TrebleNode* succ;
    // temporary pointer holder for removed node
    TrebleNodePointer owner;

    if (node->length()) propagate(parent, node->length(), 0);

    TrebleNode* child = node->right();
    if (!child) {
        // only has left child (if even that)
        succ = node->successor();
        child = node->left();
        if (child) child->parent(parent);
        owner = std::exchange(plink, std::move(node->leftLink()));
        if (parent)
            balanceOnDelete(parent, &plink == &parent->leftLink() ? 1 : -1);
        return succ;
    } else if (!node->left()) {
        // only has right child
        child->parent(parent);
        owner = std::exchange(plink, std::move(node->rightLink()));
        if (parent)
            balanceOnDelete(parent, &plink == &parent->leftLink() ? 1 : -1);
        return child->minimum();
    } else if (!child->left()) {
        // has both, but the right child has no left child
        // replace node with its right child
        owner = std::exchange(plink, std::move(node->rightLink()));
        child->parent(parent);
        child->leftLink() = std::move(node->leftLink());
        child->left()->parent(child);
        child->leftlen(node->leftlen());
        child->balance(node->balance());
        balanceOnDelete(child, -1);
        CHECK_TREBLE_BALANCE(child);
        return child;
    } else {
        // has both, and the right child has a left child
        succ = child->minimum();
        TrebleNode* sp = succ->parent();
        propagate(succ, succ->length(), 0);
        owner = std::exchange(
            plink, std::exchange(sp->leftLink(),
                                 std::exchange(succ->rightLink(),
                                               std::move(node->rightLink()))));
        succ->parent(parent);
        child->parent(succ);
        if (sp->left()) sp->left()->parent(sp);
        succ->leftLink() = std::move(node->leftLink());
        succ->left()->parent(succ);
        succ->leftlen(node->leftlen());
        succ->balance(node->balance());
        propagate(succ, node->length(), succ->length());
        balanceOnDelete(sp, 1);
        CHECK_TREBLE_BALANCE(sp);
        CHECK_TREBLE_BALANCE(succ);
        return succ;
    }
}

TrebleNode* Treble::tryMerge(TrebleNode* node) {
    if (!node) return nullptr;
#if !TREBLE_NOMERGE_DEBUG
    TrebleNode* prec = node->predecessor();
    if (prec && !node->data() && !prec->data() &&
        prec->offset() + prec->length() == node->offset()) {
        node->offset(prec->offset());

        propagate(node, 0, prec->length());
        node->lengthAdd(prec->length());
        propagate(prec, prec->length(), 0);
        prec->length(0);

        [[maybe_unused]] TrebleNode* precsucc = erase(prec);
        HEXBED_ASSERT(node == precsucc);
    }
#endif
    return node;
}

bytespan Treble::usurp(TrebleNode* node) {
    if (node->data()) {
        return bytespan(node->data(), node->length());
    } else {
        bufsize zz = roundCapacity<false>(node->length());
        node->data(newTrebleData(zz));
        node->capacity(zz);
        return bytespan(node->data(), node->length());
    }
}

bytespan Treble::usurpNew(TrebleNode* node, bufsize z) {
    HEXBED_ASSERT(!node->data());
    bufsize zz = roundCapacity<true>(z);
    node->data(newTrebleData(zz));
    node->capacity(zz);
    if (node->length() != z) {
        propagate(node, node->length(), z);
        node->length(z);
    }
    return bytespan(node->data(), node->length());
}

void Treble::splitLeft(TrebleNode* node, bufsize offset) {
    auto newnode = newTrebleNode(node, offset);
    if (node->data()) {
        bufsize l = node->length();
        bufsize zz = roundCapacity<false>(offset);
        newnode->data(newTrebleData(zz));
        newnode->capacity(zz);
        node->lengthSub(offset);
        byte* s = node->data();
        byte* d = newnode->data();
        memCopy(d, s, offset);
        memCopy(s, s + offset, l - offset);
        compact(node);
    } else {
        node->lengthSub(offset);
    }
    newnode->offset(node->offset());
    node->offsetAdd(offset);
    node->leftlenAdd(offset);
    if (!node->left()) {
        insertLeft(node, std::move(newnode));
    } else {
        node = node->left()->maximum();
        newnode->parent(node);
        insertRight(node, std::move(newnode));
    }
}

void Treble::splitRight(TrebleNode* node, bufsize offset, bool adjust,
                        bool move) {
    bufsize l = node->length(), z = l - offset;
    auto newnode = newTrebleNode(node, z);
    if (node->data()) {
        if (move && !offset) {
            newnode->dataMove(*node);
            newnode->capacity(node->capacity());
            node->capacity(0);
        } else {
            bufsize zz = roundCapacity<false>(z);
            newnode->data(newTrebleData(zz));
            newnode->capacity(zz);
            byte* s = node->data();
            memCopy(newnode->data(), s + offset, l - offset);
        }
        node->length(offset);
        compact(node);
    } else {
        node->length(offset);
    }
    newnode->offset(adjust ? node->offset() + offset : node->offset());
    if (!node->right()) {
        insertRight(node, std::move(newnode));
    } else {
        node = node->right()->minimum();
        propagate(node, 0, newnode->length());
        newnode->parent(node);
        insertLeft(node, std::move(newnode));
    }
}

void Treble::compact(TrebleNode* node) {
    if (node->capacity() > node->length() * COMPACT_MULTIPLICATIVE_THRESHOLD ||
        node->capacity() > node->length() + COMPACT_ADDITIVE_THRESHOLD) {
        bufsize zz = roundCapacity<false>(node->length());
        if (renewShrinkTrebleData(*node, zz)) node->capacity(zz);
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
    if (index == 25045448) {
        HEXBED_BREAKPOINT();
    }
    if (!node->data()) {
        if (res.suboffset) {
            // split original node to the left
            splitLeft(node, res.suboffset);
            res.suboffset = 0;
        } else if (index && count <= node->length()) {
            // special case if we are replacing data within only one
            // implicit node; try to find previous node and check if it's
            // explicit. if so, append there
            TrebleNode* pnode = res.it->predecessor();
            if (pnode && pnode->data()) {
                bufsize l = pnode->length();
                bufsize nc = expandCapacity(l, count);
                if (pnode->capacity() < nc) renewTrebleData(*pnode, nc);
                byte* d = pnode->data() + l;
                propagate(pnode, 0, count);
                pnode->lengthAdd(count);
                node->offsetAdd(count);
                propagate(node, count, 0);
                if (!(node->lengthSub(count))) tryMerge(erase(node));
                f(d, d + count);
                return;
            }
        }
        if (count == node->length()) {
            // replace full block
            bytespan span = usurp(node);
            f(span.begin(), span.end());
            return;
        }
    }
    while (count) {
        bufsize l = node->length() - res.suboffset;
        if (count < l && !node->data()) {
            // split original node to the right
            l = res.suboffset + count;
            splitRight(node, l, true);
        }
        bytespan span = usurp(node);
        auto p = span.begin() + res.suboffset;
        if (count <= l) {
            f(p, p + count);
            return;
        }
        f(p, p + l);
        count -= l;
        node = (++res.it).get();
        res.suboffset = 0;
    }
}

void Treble::replace(bufsize index, bufsize count, byte v) {
    LOG_TREBLE("replace(" << index << ", " << count << ", " << L_HEX(v) << ")");
    FillFeeder feeder{v};
    replace_(index, count, feeder);
    TREBLE_AFTER_OP();
}

void Treble::replace(bufsize index, bufsize count, const byte* data) {
    LOG_TREBLE("replace(" << index << ", " << count << ", " << L_PTR(data)
                          << ")");
    CopyFeeder feeder{data};
    replace_(index, count, feeder);
    TREBLE_AFTER_OP();
}

void Treble::replace(bufsize index, bufsize count, bufsize scount,
                     const byte* sdata, bufsize soffset) {
    LOG_TREBLE("replace(" << index << ", " << count << ", " << scount << ", "
                          << L_PTR(sdata) << ", " << soffset << ")");
    RepeatFeeder feeder{scount, sdata, soffset};
    replace_(index, count, feeder);
    TREBLE_AFTER_OP();
}

template <typename Feeder>
void Treble::insert_(bufsize index, bufsize count, Feeder& f) {
    if (!count) return;
    HEXBED_ASSERT(index <= total_, "trying to insert beyond end of file!");
    byte* d;
    TrebleNode* node;
    if (index == total_) {
        // inserting new data to the end of the file
        node = root_->maximum();
        if (!node->data()) {
            // add new node
            auto newnode = newTrebleNode(node, count);
            bufsize zz = roundCapacity<true>(count);
            newnode->offset(index);
            newnode->data(newTrebleData(zz));
            newnode->capacity(zz);
            d = newnode->data();
            insertRight(node, std::move(newnode));
        } else {
            // append to existing explicit data node
            bufsize l = node->length();
            bufsize nc = expandCapacity(l, count);
            if (node->capacity() < nc) renewTrebleData(*node, nc);
            d = node->data() + l;
            // no propagate, this cannot be a left child of any node
            node->lengthAdd(count);
        }
        f(d, d + count);
        return;
    }
    // find *previous* node
    bool zero = !index;
    TrebleFindResult res = find(zero ? index : index - 1);
    if (!zero) ++res.suboffset;
    node = res.it.get();
    if (!node->data() && res.suboffset == node->length()) {
        // if previous node is implicit, maybe the next node isn't
        // if its size is below the shift threshold, try it instead
        TrebleNode* node2 = node->successor();
        if (node2->data() && node2->length() < INSERT_SUBBLOCK_THRESHOLD) {
            node = node2;
            res.suboffset = 0;
        }
    }
    if (!node->data()) {
        // split implicit node both ways
        if (res.suboffset) splitLeft(node, res.suboffset);
        if (node->length()) splitRight(node, 0, false);
        bytespan span = usurpNew(node, count);
        f(span.begin(), span.end());
        return;
    }
    if (node->length() - res.suboffset <= INSERT_SUBBLOCK_THRESHOLD) {
        // try inserting data into the middle of an explicit node
        bufsize l = node->length();
        bufsize nc = expandCapacity(l, count);
        if (node->capacity() < nc) renewTrebleData(*node, nc);
        byte* p = node->data();
        d = p + res.suboffset;
        memCopyBack(d + count, d, l - res.suboffset);
    } else {
        splitRight(node, res.suboffset, false);
        bufsize l = res.suboffset;
        bufsize nc = expandCapacity(l, count);
        if (node->capacity() < nc) renewTrebleData(*node, nc);
        d = node->data() + l;
    }
    propagate(node, 0, count);
    node->lengthAdd(count);
    f(d, d + count);
}

void Treble::insert(bufsize index, bufsize count, byte v) {
    LOG_TREBLE("insert(" << index << ", " << count << ", " << L_HEX(v) << ")");
    FillFeeder feeder{v};
    insert_(index, count, feeder);
    total_ += count;
    TREBLE_AFTER_OP();
}

void Treble::insert(bufsize index, bufsize count, const byte* data) {
    LOG_TREBLE("insert(" << index << ", " << count << ", " << L_PTR(data)
                         << ")");
    CopyFeeder feeder{data};
    insert_(index, count, feeder);
    total_ += count;
    TREBLE_AFTER_OP();
}

void Treble::insert(bufsize index, bufsize count, bufsize scount,
                    const byte* sdata, bufsize soffset) {
    LOG_TREBLE("insert(" << index << ", " << count << ", " << scount << ", "
                         << L_PTR(sdata) << ", " << soffset << ")");
    RepeatFeeder feeder{scount, sdata, soffset};
    insert_(index, count, feeder);
    total_ += count;
    TREBLE_AFTER_OP();
}

void Treble::reinsert(bufsize index, bufsize count, bufsize offset) {
    if (!count) return;
    LOG_TREBLE("reinsert(" << index << ", " << count << ", " << offset << ")");
    if (index == total_) {
        // inserting old data to the end of the file
        TrebleNode* node = root_->maximum();
        if (!node->data() && node->offset() + node->length() == offset) {
            // no propagate, cannot be the left child of any node
            node->lengthAdd(count);
            return;
        }
        auto newnode = newTrebleNode(node, count);
        newnode->offset(offset);
        insertRight(node, std::move(newnode));
        return;
    }
    bool zero = !index;
    TrebleFindResult res = find(zero ? index : index - 1);
    if (!zero) ++res.suboffset;
    TrebleNode* node = res.it.get();
    if (!node->data() && node->length() == res.suboffset &&
        offset == node->offset() + node->length()) {
        node->lengthAdd(count);
    } else if (!res.suboffset && !node->data() &&
               count + offset == node->offset()) {
        node->offset(offset);
        node->lengthAdd(count);
    } else {
        if (res.suboffset) splitLeft(node, res.suboffset);
        if (node->length()) splitRight(node, 0, false, true);
        node->length(count);
        node->offset(offset);
    }
    propagate(node, 0, count);
    total_ += count;
    TrebleNode* succ = node->successor();
    if (succ) tryMerge(succ);
    TREBLE_AFTER_OP();
}

void Treble::revert(bufsize index, bufsize count) {
    if (!count) return;
    LOG_TREBLE("revert(" << index << ", " << count << ")");
    TrebleFindResult res = find(index);
    TrebleNode* node = res.it.get();
    HEXBED_ASSERT(node, "trying to revert beyond file!");
    while (count) {
        if (!node->data()) {
            bufsize z = node->length() - res.suboffset;
            if (count <= z) break;
            count -= z;
        } else {
            if (res.suboffset) splitLeft(node, res.suboffset);
            bufsize z = node->length();
            if (count < z) {
                splitRight(node, count, true);
                count = 0;
            } else
                count -= z;
            node->data(nullptr);
            node = tryMerge(node);
        }
        node = node->successor();
        res.suboffset = 0;
    }
    TREBLE_AFTER_OP();
}

void Treble::remove(bufsize index, bufsize count) {
    if (!count) return;
    LOG_TREBLE("remove(" << index << ", " << count << ")");
    TrebleFindResult res = find(index);
    TrebleNode* node = res.it.get();
    bufsize removed = 0;
    HEXBED_ASSERT(node, "trying to remove beyond file!");
    while (count) {
        if (res.suboffset) splitLeft(node, res.suboffset);
        bufsize z = node->length();
        if (count < z) {
            node->offsetAdd(count);
            propagate(node, count, 0);
            node->lengthSub(count);
            removed += count;
            if (node->data()) {
                byte* p = node->data();
                memCopy(p, p + count, z - count);
                compact(node);
            }
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
    TREBLE_AFTER_OP();
}

};  // namespace hexbed
