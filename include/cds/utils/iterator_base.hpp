#pragma once

#include <iterator>
#include <type_traits>

namespace cds::detail {

template <typename Iterator>
struct IteratorTraits {
    using iterator_category = typename Iterator::iterator_category;
    using value_type = typename Iterator::value_type;
    using difference_type = typename Iterator::difference_type;
    using pointer = typename Iterator::pointer;
    using reference = typename Iterator::reference;
};

template <typename T>
struct IteratorTraits<T*> {
    using iterator_category = std::random_access_iterator_tag;
    using value_type = std::remove_cv_t<T>;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;
};

template <typename T>
struct IteratorTraits<const T*> {
    using iterator_category = std::random_access_iterator_tag;
    using value_type = std::remove_cv_t<T>;
    using difference_type = std::ptrdiff_t;
    using pointer = const T*;
    using reference = const T&;
};

template <typename Category, typename T, typename Distance = std::ptrdiff_t,
          typename Pointer = T*, typename Reference = T&>
struct IteratorBase {
    using iterator_category = Category;
    using value_type = T;
    using difference_type = Distance;
    using pointer = Pointer;
    using reference = Reference;
};

template <typename Derived, typename Category, typename T, typename Distance = std::ptrdiff_t,
          typename Pointer = T*, typename Reference = T&>
struct IteratorFacade : IteratorBase<Category, T, Distance, Pointer, Reference> {
private:
    friend Derived;
    constexpr Derived& derived() noexcept { return static_cast<Derived&>(*this); }
    constexpr const Derived& derived() const noexcept { return static_cast<const Derived&>(*this); }

protected:
    IteratorFacade() = default;

public:
    constexpr Reference operator*() const noexcept { return derived().dereference(); }
    constexpr Pointer operator->() const noexcept { return std::addressof(derived().dereference()); }

    constexpr Derived& operator++() noexcept { derived().increment(); return derived(); }
    constexpr Derived operator++(int) noexcept { Derived tmp = derived(); derived().increment(); return tmp; }

    constexpr bool operator==(const Derived& other) const noexcept { return derived().equal(other); }
    constexpr bool operator!=(const Derived& other) const noexcept { return !derived().equal(other); }
};

template <typename Derived, typename Category, typename T, typename Distance = std::ptrdiff_t,
          typename Pointer = T*, typename Reference = T&>
struct BidirectionalIteratorFacade : IteratorFacade<Derived, Category, T, Distance, Pointer, Reference> {
private:
    friend Derived;
    constexpr Derived& derived() noexcept { return static_cast<Derived&>(*this); }
    constexpr const Derived& derived() const noexcept { return static_cast<const Derived&>(*this); }

protected:
    BidirectionalIteratorFacade() = default;

public:
    constexpr Derived& operator--() noexcept { derived().decrement(); return derived(); }
    constexpr Derived operator--(int) noexcept { Derived tmp = derived(); derived().decrement(); return tmp; }
};

template <typename Derived, typename Category, typename T, typename Distance = std::ptrdiff_t,
          typename Pointer = T*, typename Reference = T&>
struct RandomAccessIteratorFacade : BidirectionalIteratorFacade<Derived, Category, T, Distance, Pointer, Reference> {
private:
    friend Derived;
    constexpr Derived& derived() noexcept { return static_cast<Derived&>(*this); }
    constexpr const Derived& derived() const noexcept { return static_cast<const Derived&>(*this); }

protected:
    RandomAccessIteratorFacade() = default;

public:
    constexpr Reference operator[](Distance n) const noexcept { return derived().dereference_at(n); }

    constexpr Derived& operator+=(Distance n) noexcept { derived().advance(n); return derived(); }
    constexpr Derived& operator-=(Distance n) noexcept { derived().advance(-n); return derived(); }

    constexpr Derived operator+(Distance n) const noexcept { Derived tmp = derived(); tmp.advance(n); return tmp; }
    constexpr Derived operator-(Distance n) const noexcept { Derived tmp = derived(); tmp.advance(-n); return tmp; }
    constexpr Distance operator-(const Derived& other) const noexcept { return derived().distance_to(other); }

    constexpr bool operator<(const Derived& other) const noexcept { return derived().distance_to(other) > 0; }
    constexpr bool operator>(const Derived& other) const noexcept { return other < derived(); }
    constexpr bool operator<=(const Derived& other) const noexcept { return !(other < derived()); }
    constexpr bool operator>=(const Derived& other) const noexcept { return !(derived() < other); }
};

} // namespace cds::detail