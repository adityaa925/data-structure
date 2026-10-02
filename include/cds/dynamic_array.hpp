#pragma once

#include <cstddef>
#include <stdexcept>
#include <algorithm>
#include <utility>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include "utils/memory_traits.hpp"
#include "utils/iterator_base.hpp"

namespace cds {

template <typename T>
class DynamicArray {
public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

private:
    using Traits = detail::MemoryTraits<T>;
    pointer data_ = nullptr;
    size_type size_ = 0;
    size_type capacity_ = 0;

    static constexpr size_type growth_factor_num = 3;
    static constexpr size_type growth_factor_den = 2;

    size_type calculate_growth(size_type new_size) const {
        size_type new_cap = capacity_ ? capacity_ * growth_factor_num / growth_factor_den : 1;
        return new_cap < new_size ? new_size : new_cap;
    }

    void allocate_and_copy(size_type new_cap, size_type new_size) {
        pointer new_data = Traits::allocate(new_cap);
        try {
            Traits::uninitialized_move(data_, data_ + size_, new_data);
        } catch (...) {
            Traits::deallocate(new_data, new_cap);
            throw;
        }
        Traits::destroy_range(data_, data_ + size_);
        Traits::deallocate(data_, capacity_);
        data_ = new_data;
        capacity_ = new_cap;
        size_ = new_size;
    }

    void reallocate(size_type new_cap) {
        pointer new_data = Traits::allocate(new_cap);
        try {
            Traits::uninitialized_move(data_, data_ + size_, new_data);
        } catch (...) {
            Traits::deallocate(new_data, new_cap);
            throw;
        }
        Traits::destroy_range(data_, data_ + size_);
        Traits::deallocate(data_, capacity_);
        data_ = new_data;
        capacity_ = new_cap;
    }

public:
    class Iterator;
    class ConstIterator;

    class Iterator : public detail::RandomAccessIteratorFacade<
                         Iterator, std::random_access_iterator_tag, T,
                         difference_type, pointer, reference> {
        friend class DynamicArray;
        pointer ptr_ = nullptr;

        constexpr explicit Iterator(pointer p) noexcept : ptr_(p) {}

        constexpr reference dereference() const noexcept { return *ptr_; }
        constexpr reference dereference_at(difference_type n) const noexcept { return ptr_[n]; }
        constexpr void increment() noexcept { ++ptr_; }
        constexpr void decrement() noexcept { --ptr_; }
        constexpr void advance(difference_type n) noexcept { ptr_ += n; }
        constexpr difference_type distance_to(const Iterator& other) const noexcept { return other.ptr_ - ptr_; }
        constexpr bool equal(const Iterator& other) const noexcept { return ptr_ == other.ptr_; }

    public:
        Iterator() = default;
    };

    class ConstIterator : public detail::RandomAccessIteratorFacade<
                              ConstIterator, std::random_access_iterator_tag, T,
                              difference_type, const_pointer, const_reference> {
        friend class DynamicArray;
        const_pointer ptr_ = nullptr;

        constexpr explicit ConstIterator(const_pointer p) noexcept : ptr_(p) {}

        constexpr const_reference dereference() const noexcept { return *ptr_; }
        constexpr const_reference dereference_at(difference_type n) const noexcept { return ptr_[n]; }
        constexpr void increment() noexcept { ++ptr_; }
        constexpr void decrement() noexcept { --ptr_; }
        constexpr void advance(difference_type n) noexcept { ptr_ += n; }
        constexpr difference_type distance_to(const ConstIterator& other) const noexcept { return other.ptr_ - ptr_; }
        constexpr bool equal(const ConstIterator& other) const noexcept { return ptr_ == other.ptr_; }

    public:
        ConstIterator() = default;
        constexpr ConstIterator(const Iterator& other) noexcept : ptr_(other.ptr_) {}
    };

    DynamicArray() noexcept = default;

    explicit DynamicArray(size_type count) {
        if (count > 0) {
            data_ = Traits::allocate(count);
            capacity_ = count;
            try {
                for (size_type i = 0; i < count; ++i) {
                    Traits::construct_at(data_ + i);
                }
                size_ = count;
            } catch (...) {
                Traits::deallocate(data_, capacity_);
                throw;
            }
        }
    }

    DynamicArray(size_type count, const T& value) {
        if (count > 0) {
            data_ = Traits::allocate(count);
            capacity_ = count;
            try {
                for (size_type i = 0; i < count; ++i) {
                    Traits::construct_at(data_ + i, value);
                }
                size_ = count;
            } catch (...) {
                Traits::destroy_range(data_, data_ + count);
                Traits::deallocate(data_, capacity_);
                throw;
            }
        }
    }

    DynamicArray(std::initializer_list<T> init) : DynamicArray(init.size()) {
        size_type i = 0;
        for (const auto& val : init) {
            data_[i++] = val;
        }
    }

    DynamicArray(const DynamicArray& other) {
        if (other.size_ > 0) {
            data_ = Traits::allocate(other.capacity_);
            capacity_ = other.capacity_;
            try {
                Traits::uninitialized_copy(other.data_, other.data_ + other.size_, data_);
                size_ = other.size_;
            } catch (...) {
                Traits::deallocate(data_, capacity_);
                throw;
            }
        }
    }

    DynamicArray(DynamicArray&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    ~DynamicArray() {
        clear();
        Traits::deallocate(data_, capacity_);
    }

    DynamicArray& operator=(const DynamicArray& other) {
        if (this != &other) {
            DynamicArray tmp(other);
            swap(tmp);
        }
        return *this;
    }

    DynamicArray& operator=(DynamicArray&& other) noexcept {
        if (this != &other) {
            clear();
            Traits::deallocate(data_, capacity_);
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    DynamicArray& operator=(std::initializer_list<T> init) {
        DynamicArray tmp(init);
        swap(tmp);
        return *this;
    }

    void swap(DynamicArray& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    constexpr reference operator[](size_type pos) noexcept { return data_[pos]; }
    constexpr const_reference operator[](size_type pos) const noexcept { return data_[pos]; }

    constexpr reference at(size_type pos) {
        if (pos >= size_) throw std::out_of_range("DynamicArray::at");
        return data_[pos];
    }
    constexpr const_reference at(size_type pos) const {
        if (pos >= size_) throw std::out_of_range("DynamicArray::at");
        return data_[pos];
    }

    constexpr reference front() noexcept { return data_[0]; }
    constexpr const_reference front() const noexcept { return data_[0]; }
    constexpr reference back() noexcept { return data_[size_ - 1]; }
    constexpr const_reference back() const noexcept { return data_[size_ - 1]; }
    constexpr pointer data() noexcept { return data_; }
    constexpr const_pointer data() const noexcept { return data_; }

    constexpr bool empty() const noexcept { return size_ == 0; }
    constexpr size_type size() const noexcept { return size_; }
    constexpr size_type capacity() const noexcept { return capacity_; }

    void reserve(size_type new_cap) {
        if (new_cap > capacity_) {
            reallocate(new_cap);
        }
    }

    void shrink_to_fit() {
        if (size_ < capacity_) {
            if (size_ == 0) {
                Traits::deallocate(data_, capacity_);
                data_ = nullptr;
                capacity_ = 0;
            } else {
                reallocate(size_);
            }
        }
    }

    void clear() noexcept {
        Traits::destroy_range(data_, data_ + size_);
        size_ = 0;
    }

    void resize(size_type new_size) {
        if (new_size > size_) {
            if (new_size > capacity_) {
                reserve(calculate_growth(new_size));
            }
            for (size_type i = size_; i < new_size; ++i) {
                Traits::construct_at(data_ + i);
            }
        } else if (new_size < size_) {
            Traits::destroy_range(data_ + new_size, data_ + size_);
        }
        size_ = new_size;
    }

    void resize(size_type new_size, const T& value) {
        if (new_size > size_) {
            if (new_size > capacity_) {
                reserve(calculate_growth(new_size));
            }
            for (size_type i = size_; i < new_size; ++i) {
                Traits::construct_at(data_ + i, value);
            }
        } else if (new_size < size_) {
            Traits::destroy_range(data_ + new_size, data_ + size_);
        }
        size_ = new_size;
    }

    void push_back(const T& value) {
        if (size_ == capacity_) {
            reserve(calculate_growth(size_ + 1));
        }
        Traits::construct_at(data_ + size_, value);
        ++size_;
    }

    void push_back(T&& value) {
        if (size_ == capacity_) {
            reserve(calculate_growth(size_ + 1));
        }
        Traits::construct_at(data_ + size_, std::move(value));
        ++size_;
    }

    template <typename... Args>
    reference emplace_back(Args&&... args) {
        if (size_ == capacity_) {
            reserve(calculate_growth(size_ + 1));
        }
        Traits::construct_at(data_ + size_, std::forward<Args>(args)...);
        return data_[size_++];
    }

    void pop_back() noexcept {
        if (size_ > 0) {
            Traits::destroy_at(data_ + size_ - 1);
            --size_;
        }
    }

    Iterator begin() noexcept { return Iterator(data_); }
    Iterator end() noexcept { return Iterator(data_ + size_); }
    ConstIterator begin() const noexcept { return ConstIterator(data_); }
    ConstIterator end() const noexcept { return ConstIterator(data_ + size_); }
    ConstIterator cbegin() const noexcept { return begin(); }
    ConstIterator cend() const noexcept { return end(); }
};

template <typename T>
inline bool operator==(const DynamicArray<T>& lhs, const DynamicArray<T>& rhs) {
    if (lhs.size() != rhs.size()) return false;
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T>
inline bool operator!=(const DynamicArray<T>& lhs, const DynamicArray<T>& rhs) {
    return !(lhs == rhs);
}

template <typename T>
inline bool operator<(const DynamicArray<T>& lhs, const DynamicArray<T>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename T>
inline bool operator>(const DynamicArray<T>& lhs, const DynamicArray<T>& rhs) {
    return rhs < lhs;
}

template <typename T>
inline bool operator<=(const DynamicArray<T>& lhs, const DynamicArray<T>& rhs) {
    return !(rhs < lhs);
}

template <typename T>
inline bool operator>=(const DynamicArray<T>& lhs, const DynamicArray<T>& rhs) {
    return !(lhs < rhs);
}

template <typename T>
void swap(DynamicArray<T>& lhs, DynamicArray<T>& rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace cds