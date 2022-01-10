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
// common/aligneduniqueptr.hh -- header for aligned unique_ptrs

#ifndef HEXBED_COMMON_ALIGNEDUNIQUEPTR_HH
#define HEXBED_COMMON_ALIGNEDUNIQUEPTR_HH

#include <concepts>
#include <memory>

namespace hexbed {

template <typename T, std::size_t N>
struct alignas(N) AlignedWrapper {
    T obj;

    template <typename... Args>
    AlignedWrapper(Args&&... args) : obj(std::forward<Args>(args)...) {}
};

template <typename T, std::size_t N>
class AlignedUniquePtr {
  public:
    using pointer = T*;
    using element_type = T;
    using inner_type = T;
    static constexpr std::size_t alignment = N;
    static constexpr bool has_array_type = false;

    constexpr AlignedUniquePtr() noexcept : ptr_(nullptr) {}
    constexpr AlignedUniquePtr(std::nullptr_t) noexcept : ptr_(nullptr) {}
    explicit AlignedUniquePtr(pointer ptr) noexcept
        : ptr_(reinterpret_cast<decltype(ptr_)>(reinterpret_cast<void*>(ptr))) {
    }
    AlignedUniquePtr(const AlignedUniquePtr<T, N>& copy) = delete;
    AlignedUniquePtr(AlignedUniquePtr<T, N>&& move) noexcept
        : ptr_(std::exchange(move.ptr_, nullptr)) {}

    AlignedUniquePtr& operator=(const AlignedUniquePtr<T, N>& copy) = delete;
    AlignedUniquePtr& operator=(AlignedUniquePtr<T, N>&& move) {
        ptr_ = std::exchange(move.ptr_, nullptr);
        return *this;
    };

    ~AlignedUniquePtr() noexcept {
        if (ptr_) delete ptr_;
    }

    pointer release() noexcept { return std::exchange(ptr_, nullptr); }
    void reset(pointer ptr = pointer{}) noexcept {
        this->~AlignedUniquePtr();
        ptr_ = reinterpret_cast<decltype(ptr)>(reinterpret_cast<void*>(ptr));
    }
    void swap(AlignedUniquePtr<T, N>& other) noexcept {
        std::swap(ptr_, other.ptr_);
    }

    pointer get() const noexcept { return &ptr_->obj; }
    explicit operator bool() const noexcept { return ptr_ != nullptr; }

    typename std::add_lvalue_reference<T>::type operator*() const
        noexcept(noexcept(*std::declval<pointer>())) {
        return ptr_->obj;
    }
    pointer operator->() const noexcept { return &ptr_->obj; }

  private:
    AlignedWrapper<T, N>* ptr_;
};

// clang-format off

template <typename T, std::size_t N, typename... Args>
requires(!std::is_array_v<T>)
AlignedUniquePtr<T, N> makeAlignedUnique(Args&&... args) {
    return AlignedUniquePtr<T, N>(new (std::align_val_t(N))
                                      T(std::forward<Args>(args)...));
}

template <typename T, std::size_t N>
requires(std::is_unbounded_array_v<T>)
AlignedUniquePtr<T, N> makeAlignedUnique(std::size_t n) = delete;

template <typename T, std::size_t N, typename... Args>
requires(std::is_bounded_array_v<T>)
AlignedUniquePtr<T, N> makeAlignedUnique(Args&&...) = delete;

template <typename T, std::size_t N, typename... Args>
requires(!std::is_array_v<T>)
AlignedUniquePtr<T, N> makeAlignedUniqueNothrow(Args&&... args) {
    return AlignedUniquePtr<T, N>(new (std::align_val_t(N), std::nothrow)
                                      T(std::forward<Args>(args)...));
}

template <typename T, std::size_t N>
requires(std::is_unbounded_array_v<T>)
AlignedUniquePtr<T, N> makeAlignedUniqueNothrow(std::size_t n) = delete;

template <typename T, std::size_t N, typename... Args>
requires(std::is_bounded_array_v<T>)
AlignedUniquePtr<T, N> makeAlignedUniqueNothrow(Args&&...) = delete;

// clang-format on

template <typename Tptr>
struct std_unique_ptr_inner_type;

template <typename T>
struct std_unique_ptr_inner_type<std::unique_ptr<T>> {
    using type = T;
};

template <typename T>
struct std_unique_ptr_inner_type<std::unique_ptr<T[]>> {
    using type = T[];
};

template <typename Tptr, typename... Args>
auto makeUniqueOf(Args&&... args) {
    return std::make_unique<typename std_unique_ptr_inner_type<Tptr>::type>(
        std::forward<Args>(args)...);
}

template <typename T, typename... Args>
requires(!std::is_array_v<T>) std::unique_ptr<T> makeUniqueNothrow(
    Args&&... args) {
    return std::unique_ptr<T>(new (std::nothrow)
                                  T(std::forward<Args>(args)...));
}

template <typename T>
requires(std::is_unbounded_array_v<T>) std::unique_ptr<T> makeUniqueNothrow(
    std::size_t n) {
    return std::unique_ptr<T>(new (std::nothrow) std::remove_extent_t<T>[n]());
}

template <typename T, typename... Args>
requires(std::is_bounded_array_v<T>) std::unique_ptr<T> makeUniqueNothrow(
    Args&&...)
= delete;

template <typename Tptr, typename... Args>
auto makeUniqueOfNothrow(Args&&... args) {
    return makeUniqueNothrow<typename std_unique_ptr_inner_type<Tptr>::type>(
        std::forward<Args>(args)...);
}

template <typename Tptr, typename... Args>
auto makeAlignedUniqueOf(Args&&... args) {
    return makeAlignedUnique<typename Tptr::inner_type, Tptr::alignment>(
        std::forward<Args>(args)...);
}

template <typename Tptr, typename... Args>
auto makeAlignedUniqueOfNothrow(Args&&... args) {
    return makeAlignedUniqueNothrow<typename Tptr::inner_type, Tptr::alignment>(
        std::forward<Args>(args)...);
}

};  // namespace hexbed

namespace std {
template <typename T, std::size_t N>
void swap(hexbed::AlignedUniquePtr<T, N>& lhs,
          hexbed::AlignedUniquePtr<T, N>& rhs) {
    lhs.swap(rhs);
}
}  // namespace std

#endif /* HEXBED_COMMON_ALIGNEDUNIQUEPTR_HH */
