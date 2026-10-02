#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>
#include "dynamic_array.hpp"
#include "linked_list.hpp"

namespace cds {

template <typename T, typename Container = DynamicArray<T>>
class Stack {
public:
    using container_type = Container;
    using value_type = typename Container::value_type;
    using reference = typename Container::reference;
    using const_reference = typename Container::const_reference;
    using size_type = typename Container::size_type;

protected:
    Container c_;

public:
    Stack() = default;

    explicit Stack(const Container& cont) : c_(cont) {}
    explicit Stack(Container&& cont) : c_(std::move(cont)) {}

    template <typename InputIt>
    Stack(InputIt first, InputIt last) : c_(first, last) {}

    Stack(std::initializer_list<T> init) : c_(init) {}

    constexpr bool empty() const noexcept { return c_.empty(); }
    constexpr size_type size() const noexcept { return c_.size(); }

    constexpr reference top() noexcept { return c_.back(); }
    constexpr const_reference top() const noexcept { return c_.back(); }

    void push(const T& value) { c_.push_back(value); }
    void push(T&& value) { c_.push_back(std::move(value)); }

    template <typename... Args>
    void emplace(Args&&... args) { c_.emplace_back(std::forward<Args>(args)...); }

    void pop() { c_.pop_back(); }

    void swap(Stack& other) noexcept(std::is_nothrow_swappable_v<Container>) {
        using std::swap;
        swap(c_, other.c_);
    }

    friend constexpr bool operator==(const Stack& lhs, const Stack& rhs) noexcept
        requires requires { lhs.c_ == rhs.c_; }
    { return lhs.c_ == rhs.c_; }

    friend constexpr bool operator!=(const Stack& lhs, const Stack& rhs) noexcept
        requires requires { lhs.c_ != rhs.c_; }
    { return lhs.c_ != rhs.c_; }

    friend constexpr bool operator<(const Stack& lhs, const Stack& rhs) noexcept
        requires requires { lhs.c_ < rhs.c_; }
    { return lhs.c_ < rhs.c_; }

    friend constexpr bool operator>(const Stack& lhs, const Stack& rhs) noexcept
        requires requires { lhs.c_ > rhs.c_; }
    { return rhs < lhs; }

    friend constexpr bool operator<=(const Stack& lhs, const Stack& rhs) noexcept
        requires requires { lhs.c_ <= rhs.c_; }
    { return !(rhs < lhs); }

    friend constexpr bool operator>=(const Stack& lhs, const Stack& rhs) noexcept
        requires requires { lhs.c_ >= rhs.c_; }
    { return !(lhs < rhs); }
};

template <typename T>
using NodeStack = Stack<T, LinkedList<T>>;

template <typename T, typename Container>
void swap(Stack<T, Container>& lhs, Stack<T, Container>& rhs) noexcept
    noexcept(noexcept(lhs.swap(rhs))) {
    lhs.swap(rhs);
}

} // namespace cds