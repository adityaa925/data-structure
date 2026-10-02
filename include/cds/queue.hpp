#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <algorithm>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include "utils/memory_traits.hpp"
#include "utils/iterator_base.hpp"

namespace cds {

template <typename T>
class Queue {
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
    size_type capacity_ = 0;
    size_type size_ = 0;
    size_type head_ = 0;
    size_type tail_ = 0;

    static constexpr size_type min_capacity = 8;

    size_type next_index(size_type idx) const noexcept {
        return (idx + 1) % capacity_;
    }

    size_type prev_index(size_type idx) const noexcept {
        return (idx == 0) ? capacity_ - 1 : idx - 1;
    }

    pointer at_index(size_type idx) noexcept {
        return data_ + idx;
    }

    const_pointer at_index(size_type idx) const noexcept {
        return data_ + idx;
    }

    void allocate_buffer(size_type cap) {
        capacity_ = cap;
        data_ = Traits::allocate(capacity_);
        head_ = 0;
        tail_ = 0;
        size_ = 0;
    }

    void deallocate_buffer() noexcept {
        if (data_) {
            Traits::deallocate(data_, capacity_);
            data_ = nullptr;
            capacity_ = 0;
        }
    }

    void destroy_elements() noexcept {
        if (size_ == 0) return;
        if (head_ < tail_) {
            Traits::destroy_range(at_index(head_), at_index(tail_));
        } else {
            Traits::destroy_range(at_index(head_), data_ + capacity_);
            Traits::destroy_range(data_, at_index(tail_));
        }
    }

    void copy_elements(const Queue& other) {
        if (other.size_ == 0) return;
        if (other.head_ < other.tail_) {
            Traits::uninitialized_copy(other.at_index(other.head_), other.at_index(other.tail_), at_index(head_));
        } else {
            Traits::uninitialized_copy(other.at_index(other.head_), other.data_ + other.capacity_, at_index(head_));
            Traits::uninitialized_copy(other.data_, other.at_index(other.tail_), at_index(head_) + (other.capacity_ - other.head_));
        }
    }

    void move_elements(Queue& other) noexcept {
        if (other.size_ == 0) return;
        if (other.head_ < other.tail_) {
            Traits::uninitialized_move(other.at_index(other.head_), other.at_index(other.tail_), at_index(head_));
        } else {
            Traits::uninitialized_move(other.at_index(other.head_), other.data_ + other.capacity_, at_index(head_));
            Traits::uninitialized_move(other.data_, other.at_index(other.tail_), at_index(head_) + (other.capacity_ - other.head_));
        }
    }

    void reallocate(size_type new_cap) {
        pointer new_data = Traits::allocate(new_cap);
        size_type new_head = 0;
        try {
            if (size_ > 0) {
                if (head_ < tail_) {
                    Traits::uninitialized_move(at_index(head_), at_index(tail_), new_data);
                } else {
                    size_type first_part = capacity_ - head_;
                    Traits::uninitialized_move(at_index(head_), data_ + capacity_, new_data);
                    Traits::uninitialized_move(data_, at_index(tail_), new_data + first_part);
                }
            }
        } catch (...) {
            Traits::deallocate(new_data, new_cap);
            throw;
        }
        destroy_elements();
        deallocate_buffer();
        data_ = new_data;
        capacity_ = new_cap;
        head_ = 0;
        tail_ = size_;
    }

    void ensure_capacity(size_type min_cap) {
        if (min_cap <= capacity_) return;
        size_type new_cap = std::max(min_cap, capacity_ == 0 ? min_capacity : capacity_ * 2);
        reallocate(new_cap);
    }

public:
    class Iterator;
    class ConstIterator;

    class Iterator : public detail::RandomAccessIteratorFacade<
                         Iterator, std::random_access_iterator_tag, T,
                         difference_type, pointer, reference> {
        friend class Queue;
        Queue* queue_ = nullptr;
        size_type index_ = 0;

        constexpr Iterator(Queue* q, size_type idx) noexcept : queue_(q), index_(idx) {}

        constexpr reference dereference() const noexcept { return *queue_->at_index(index_); }
        constexpr reference dereference_at(difference_type n) const noexcept { 
            size_type idx = (index_ + n) % queue_->capacity_;
            return *queue_->at_index(idx); 
        }
        constexpr void increment() noexcept { index_ = queue_->next_index(index_); }
        constexpr void decrement() noexcept { index_ = queue_->prev_index(index_); }
        constexpr void advance(difference_type n) noexcept { 
            if (n >= 0) {
                index_ = (index_ + n) % queue_->capacity_;
            } else {
                difference_type abs_n = -n;
                index_ = (index_ + queue_->capacity_ - (abs_n % queue_->capacity_)) % queue_->capacity_;
            }
        }
        constexpr difference_type distance_to(const Iterator& other) const noexcept {
            if (other.index_ >= index_) return other.index_ - index_;
            return other.index_ + queue_->capacity_ - index_;
        }
        constexpr bool equal(const Iterator& other) const noexcept { return index_ == other.index_; }

    public:
        Iterator() = default;
    };

    class ConstIterator : public detail::RandomAccessIteratorFacade<
                              ConstIterator, std::random_access_iterator_tag, T,
                              difference_type, const_pointer, const_reference> {
        friend class Queue;
        const Queue* queue_ = nullptr;
        size_type index_ = 0;

        constexpr ConstIterator(const Queue* q, size_type idx) noexcept : queue_(q), index_(idx) {}

        constexpr const_reference dereference() const noexcept { return *queue_->at_index(index_); }
        constexpr const_reference dereference_at(difference_type n) const noexcept { 
            size_type idx = (index_ + n) % queue_->capacity_;
            return *queue_->at_index(idx); 
        }
        constexpr void increment() noexcept { index_ = queue_->next_index(index_); }
        constexpr void decrement() noexcept { index_ = queue_->prev_index(index_); }
        constexpr void advance(difference_type n) noexcept { 
            if (n >= 0) {
                index_ = (index_ + n) % queue_->capacity_;
            } else {
                difference_type abs_n = -n;
                index_ = (index_ + queue_->capacity_ - (abs_n % queue_->capacity_)) % queue_->capacity_;
            }
        }
        constexpr difference_type distance_to(const ConstIterator& other) const noexcept {
            if (other.index_ >= index_) return other.index_ - index_;
            return other.index_ + queue_->capacity_ - index_;
        }
        constexpr bool equal(const ConstIterator& other) const noexcept { return index_ == other.index_; }

    public:
        ConstIterator() = default;
        constexpr ConstIterator(const Iterator& other) noexcept : queue_(other.queue_), index_(other.index_) {}
    };

    Queue() = default;

    explicit Queue(size_type count) {
        allocate_buffer(std::max(count, min_capacity));
        try {
            for (size_type i = 0; i < count; ++i) {
                Traits::construct_at(at_index(tail_));
                tail_ = next_index(tail_);
                ++size_;
            }
        } catch (...) {
            destroy_elements();
            deallocate_buffer();
            throw;
        }
    }

    Queue(size_type count, const T& value) {
        allocate_buffer(std::max(count, min_capacity));
        try {
            for (size_type i = 0; i < count; ++i) {
                Traits::construct_at(at_index(tail_), value);
                tail_ = next_index(tail_);
                ++size_;
            }
        } catch (...) {
            destroy_elements();
            deallocate_buffer();
            throw;
        }
    }

    Queue(std::initializer_list<T> init) {
        allocate_buffer(std::max(init.size(), min_capacity));
        try {
            for (const auto& val : init) {
                Traits::construct_at(at_index(tail_), val);
                tail_ = next_index(tail_);
                ++size_;
            }
        } catch (...) {
            destroy_elements();
            deallocate_buffer();
            throw;
        }
    }

    Queue(const Queue& other) {
        if (other.size_ > 0) {
            allocate_buffer(other.capacity_);
            copy_elements(other);
            size_ = other.size_;
            tail_ = (head_ + size_) % capacity_;
        }
    }

    Queue(Queue&& other) noexcept
        : data_(other.data_), capacity_(other.capacity_), size_(other.size_),
          head_(other.head_), tail_(other.tail_) {
        other.data_ = nullptr;
        other.capacity_ = 0;
        other.size_ = 0;
        other.head_ = 0;
        other.tail_ = 0;
    }

    ~Queue() {
        destroy_elements();
        deallocate_buffer();
    }

    Queue& operator=(const Queue& other) {
        if (this != &other) {
            Queue tmp(other);
            swap(tmp);
        }
        return *this;
    }

    Queue& operator=(Queue&& other) noexcept {
        if (this != &other) {
            destroy_elements();
            deallocate_buffer();
            data_ = other.data_;
            capacity_ = other.capacity_;
            size_ = other.size_;
            head_ = other.head_;
            tail_ = other.tail_;
            other.data_ = nullptr;
            other.capacity_ = 0;
            other.size_ = 0;
            other.head_ = 0;
            other.tail_ = 0;
        }
        return *this;
    }

    Queue& operator=(std::initializer_list<T> init) {
        Queue tmp(init);
        swap(tmp);
        return *this;
    }

    void swap(Queue& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
        std::swap(head_, other.head_);
        std::swap(tail_, other.tail_);
    }

    constexpr reference front() noexcept { return *at_index(head_); }
    constexpr const_reference front() const noexcept { return *at_index(head_); }
    constexpr reference back() noexcept { return *at_index(prev_index(tail_)); }
    constexpr const_reference back() const noexcept { return *at_index(prev_index(tail_)); }

    constexpr bool empty() const noexcept { return size_ == 0; }
    constexpr size_type size() const noexcept { return size_; }
    constexpr size_type capacity() const noexcept { return capacity_; }

    void reserve(size_type new_cap) {
        if (new_cap > capacity_) {
            reallocate(new_cap);
        }
    }

    void shrink_to_fit() {
        if (size_ < capacity_ && size_ > 0) {
            reallocate(size_);
        } else if (size_ == 0 && capacity_ > 0) {
            deallocate_buffer();
        }
    }

    void clear() noexcept {
        destroy_elements();
        head_ = 0;
        tail_ = 0;
        size_ = 0;
    }

    template <typename... Args>
    void emplace(Args&&... args) {
        ensure_capacity(size_ + 1);
        Traits::construct_at(at_index(tail_), std::forward<Args>(args)...);
        tail_ = next_index(tail_);
        ++size_;
    }

    void push(const T& value) { emplace(value); }
    void push(T&& value) { emplace(std::move(value)); }

    void pop() noexcept {
        if (size_ > 0) {
            Traits::destroy_at(at_index(head_));
            head_ = next_index(head_);
            --size_;
        }
    }

    Iterator begin() noexcept { return Iterator(this, head_); }
    Iterator end() noexcept { return Iterator(this, tail_); }
    ConstIterator begin() const noexcept { return ConstIterator(this, head_); }
    ConstIterator end() const noexcept { return ConstIterator(this, tail_); }
    ConstIterator cbegin() const noexcept { return begin(); }
    ConstIterator cend() const noexcept { return end(); }
};

template <typename T>
inline bool operator==(const Queue<T>& lhs, const Queue<T>& rhs) {
    if (lhs.size() != rhs.size()) return false;
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T>
inline bool operator!=(const Queue<T>& lhs, const Queue<T>& rhs) {
    return !(lhs == rhs);
}

template <typename T>
inline bool operator<(const Queue<T>& lhs, const Queue<T>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename T>
inline bool operator>(const Queue<T>& lhs, const Queue<T>& rhs) {
    return rhs < lhs;
}

template <typename T>
inline bool operator<=(const Queue<T>& lhs, const Queue<T>& rhs) {
    return !(rhs < lhs);
}

template <typename T>
inline bool operator>=(const Queue<T>& lhs, const Queue<T>& rhs) {
    return !(lhs < rhs);
}

template <typename T>
void swap(Queue<T>& lhs, Queue<T>& rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace cds