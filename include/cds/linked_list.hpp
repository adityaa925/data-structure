#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <iterator>
#include <initializer_list>
#include <memory>
#include "utils/memory_traits.hpp"
#include "utils/iterator_base.hpp"

namespace cds {

template <typename T>
class LinkedList {
public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

private:
    struct Node {
        T data;
        Node* next;
        Node* prev;

        template <typename... Args>
        Node(Args&&... args) : data(std::forward<Args>(args)...), next(nullptr), prev(nullptr) {}
    };

    using NodeTraits = detail::MemoryTraits<Node>;

    Node sentinel_;
    size_type size_ = 0;

    void init_sentinel() {
        sentinel_.next = &sentinel_;
        sentinel_.prev = &sentinel_;
    }

    void link_nodes(Node* prev, Node* new_node, Node* next) {
        new_node->prev = prev;
        new_node->next = next;
        prev->next = new_node;
        next->prev = new_node;
    }

    void unlink_node(Node* node) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    template <typename... Args>
    Node* create_node(Args&&... args) {
        Node* node = NodeTraits::allocate(1);
        try {
            NodeTraits::construct_at(node, std::forward<Args>(args)...);
        } catch (...) {
            NodeTraits::deallocate(node, 1);
            throw;
        }
        return node;
    }

    void destroy_node(Node* node) {
        NodeTraits::destroy_at(node);
        NodeTraits::deallocate(node, 1);
    }

    void clear_nodes() noexcept {
        Node* current = sentinel_.next;
        while (current != &sentinel_) {
            Node* next = current->next;
            destroy_node(current);
            current = next;
        }
        init_sentinel();
        size_ = 0;
    }

public:
    class Iterator;
    class ConstIterator;

    class Iterator : public detail::BidirectionalIteratorFacade<
                         Iterator, std::bidirectional_iterator_tag, T,
                         difference_type, pointer, reference> {
        friend class LinkedList;
        Node* node_ = nullptr;

        constexpr explicit Iterator(Node* n) noexcept : node_(n) {}

        constexpr reference dereference() const noexcept { return node_->data; }
        constexpr void increment() noexcept { node_ = node_->next; }
        constexpr void decrement() noexcept { node_ = node_->prev; }
        constexpr bool equal(const Iterator& other) const noexcept { return node_ == other.node_; }

    public:
        Iterator() = default;
    };

    class ConstIterator : public detail::BidirectionalIteratorFacade<
                              ConstIterator, std::bidirectional_iterator_tag, T,
                              difference_type, const_pointer, const_reference> {
        friend class LinkedList;
        const Node* node_ = nullptr;

        constexpr explicit ConstIterator(const Node* n) noexcept : node_(n) {}

        constexpr const_reference dereference() const noexcept { return node_->data; }
        constexpr void increment() noexcept { node_ = node_->next; }
        constexpr void decrement() noexcept { node_ = node_->prev; }
        constexpr bool equal(const ConstIterator& other) const noexcept { return node_ == other.node_; }

    public:
        ConstIterator() = default;
        constexpr ConstIterator(const Iterator& other) noexcept : node_(other.node_) {}
    };

    LinkedList() { init_sentinel(); }

    LinkedList(size_type count) { 
        init_sentinel();
        for (size_type i = 0; i < count; ++i) {
            push_back();
        }
    }

    LinkedList(size_type count, const T& value) {
        init_sentinel();
        for (size_type i = 0; i < count; ++i) {
            push_back(value);
        }
    }

    LinkedList(std::initializer_list<T> init) {
        init_sentinel();
        for (const auto& val : init) {
            push_back(val);
        }
    }

    LinkedList(const LinkedList& other) {
        init_sentinel();
        for (const auto& val : other) {
            push_back(val);
        }
    }

    LinkedList(LinkedList&& other) noexcept : size_(other.size_) {
        sentinel_.next = other.sentinel_.next;
        sentinel_.prev = other.sentinel_.prev;
        sentinel_.next->prev = &sentinel_;
        sentinel_.prev->next = &sentinel_;
        other.init_sentinel();
        other.size_ = 0;
    }

    ~LinkedList() { clear_nodes(); }

    LinkedList& operator=(const LinkedList& other) {
        if (this != &other) {
            LinkedList tmp(other);
            swap(tmp);
        }
        return *this;
    }

    LinkedList& operator=(LinkedList&& other) noexcept {
        if (this != &other) {
            clear_nodes();
            sentinel_.next = other.sentinel_.next;
            sentinel_.prev = other.sentinel_.prev;
            sentinel_.next->prev = &sentinel_;
            sentinel_.prev->next = &sentinel_;
            size_ = other.size_;
            other.init_sentinel();
            other.size_ = 0;
        }
        return *this;
    }

    LinkedList& operator=(std::initializer_list<T> init) {
        LinkedList tmp(init);
        swap(tmp);
        return *this;
    }

    void swap(LinkedList& other) noexcept {
        if (this != &other) {
            if (empty() && other.empty()) return;
            if (empty()) {
                sentinel_.next = other.sentinel_.next;
                sentinel_.prev = other.sentinel_.prev;
                sentinel_.next->prev = &sentinel_;
                sentinel_.prev->next = &sentinel_;
                other.init_sentinel();
            } else if (other.empty()) {
                other.sentinel_.next = sentinel_.next;
                other.sentinel_.prev = sentinel_.prev;
                other.sentinel_.next->prev = &other.sentinel_;
                other.sentinel_.prev->next = &other.sentinel_;
                init_sentinel();
            } else {
                std::swap(sentinel_.next, other.sentinel_.next);
                std::swap(sentinel_.prev, other.sentinel_.prev);
                sentinel_.next->prev = &sentinel_;
                sentinel_.prev->next = &sentinel_;
                other.sentinel_.next->prev = &other.sentinel_;
                other.sentinel_.prev->next = &other.sentinel_;
            }
            std::swap(size_, other.size_);
        }
    }

    constexpr reference front() noexcept { return sentinel_.next->data; }
    constexpr const_reference front() const noexcept { return sentinel_.next->data; }
    constexpr reference back() noexcept { return sentinel_.prev->data; }
    constexpr const_reference back() const noexcept { return sentinel_.prev->data; }

    constexpr bool empty() const noexcept { return size_ == 0; }
    constexpr size_type size() const noexcept { return size_; }

    void clear() noexcept { clear_nodes(); }

    template <typename... Args>
    void emplace_front(Args&&... args) {
        Node* node = create_node(std::forward<Args>(args)...);
        link_nodes(&sentinel_, node, sentinel_.next);
        ++size_;
    }

    template <typename... Args>
    void emplace_back(Args&&... args) {
        Node* node = create_node(std::forward<Args>(args)...);
        link_nodes(sentinel_.prev, node, &sentinel_);
        ++size_;
    }

    void push_front(const T& value) { emplace_front(value); }
    void push_front(T&& value) { emplace_front(std::move(value)); }
    void push_back(const T& value) { emplace_back(value); }
    void push_back(T&& value) { emplace_back(std::move(value)); }

    void pop_front() noexcept {
        if (!empty()) {
            Node* node = sentinel_.next;
            unlink_node(node);
            destroy_node(node);
            --size_;
        }
    }

    void pop_back() noexcept {
        if (!empty()) {
            Node* node = sentinel_.prev;
            unlink_node(node);
            destroy_node(node);
            --size_;
        }
    }

    Iterator begin() noexcept { return Iterator(sentinel_.next); }
    Iterator end() noexcept { return Iterator(&sentinel_); }
    ConstIterator begin() const noexcept { return ConstIterator(sentinel_.next); }
    ConstIterator end() const noexcept { return ConstIterator(&sentinel_); }
    ConstIterator cbegin() const noexcept { return begin(); }
    ConstIterator cend() const noexcept { return end(); }

    Iterator insert(ConstIterator pos, const T& value) {
        Node* node = create_node(value);
        link_nodes(pos.node_->prev, node, pos.node_);
        ++size_;
        return Iterator(node);
    }

    Iterator insert(ConstIterator pos, T&& value) {
        Node* node = create_node(std::move(value));
        link_nodes(pos.node_->prev, node, pos.node_);
        ++size_;
        return Iterator(node);
    }

    template <typename... Args>
    Iterator emplace(ConstIterator pos, Args&&... args) {
        Node* node = create_node(std::forward<Args>(args)...);
        link_nodes(pos.node_->prev, node, pos.node_);
        ++size_;
        return Iterator(node);
    }

    Iterator erase(ConstIterator pos) noexcept {
        Node* node = pos.node_;
        if (node == &sentinel_) return end();
        Node* next = node->next;
        unlink_node(node);
        destroy_node(node);
        --size_;
        return Iterator(next);
    }

    Iterator erase(ConstIterator first, ConstIterator last) noexcept {
        while (first != last) {
            first = erase(first);
        }
        return Iterator(last.node_);
    }
};

template <typename T>
inline bool operator==(const LinkedList<T>& lhs, const LinkedList<T>& rhs) {
    if (lhs.size() != rhs.size()) return false;
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T>
inline bool operator!=(const LinkedList<T>& lhs, const LinkedList<T>& rhs) {
    return !(lhs == rhs);
}

template <typename T>
inline bool operator<(const LinkedList<T>& lhs, const LinkedList<T>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename T>
inline bool operator>(const LinkedList<T>& lhs, const LinkedList<T>& rhs) {
    return rhs < lhs;
}

template <typename T>
inline bool operator<=(const LinkedList<T>& lhs, const LinkedList<T>& rhs) {
    return !(rhs < lhs);
}

template <typename T>
inline bool operator>=(const LinkedList<T>& lhs, const LinkedList<T>& rhs) {
    return !(lhs < rhs);
}

template <typename T>
void swap(LinkedList<T>& lhs, LinkedList<T>& rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace cds