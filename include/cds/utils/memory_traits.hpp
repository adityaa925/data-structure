#pragma once

#include <memory>
#include <utility>
#include <cstddef>
#include <type_traits>

namespace cds::detail {

template <typename T>
struct MemoryTraits {
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    static pointer allocate(size_type n) {
        if (n == 0) return nullptr;
        return static_cast<pointer>(::operator new(n * sizeof(T), std::align_val_t(alignof(T))));
    }

    static void deallocate(pointer p, size_type n) noexcept {
        if (p) ::operator delete(p, n * sizeof(T), std::align_val_t(alignof(T)));
    }

    template <typename... Args>
    static void construct_at(pointer p, Args&&... args) {
        ::new (static_cast<void*>(p)) T(std::forward<Args>(args)...);
    }

    static void destroy_at(pointer p) noexcept {
        if (p) p->~T();
    }

    template <typename U>
    static void destroy_range(pointer first, pointer last) noexcept {
        for (; first != last; ++first) {
            destroy_at(first);
        }
    }

    template <typename U>
    static void uninitialized_copy(pointer first, pointer last, pointer d_first) {
        pointer current = d_first;
        try {
            for (; first != last; ++first, ++current) {
                construct_at(current, *first);
            }
        } catch (...) {
            destroy_range(d_first, current);
            throw;
        }
    }

    template <typename U>
    static void uninitialized_move(pointer first, pointer last, pointer d_first) {
        pointer current = d_first;
        try {
            for (; first != last; ++first, ++current) {
                construct_at(current, std::move_if_noexcept(*first));
            }
        } catch (...) {
            destroy_range(d_first, current);
            throw;
        }
    }
};

template <typename T>
constexpr bool is_nothrow_move_constructible_v = std::is_nothrow_move_constructible_v<T>;

} // namespace cds::detail