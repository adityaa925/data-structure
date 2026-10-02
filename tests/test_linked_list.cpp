#include "test_framework.hpp"
#include <cds/linked_list.hpp>
#include <algorithm>
#include <stdexcept>

struct ThrowOnCopy {
    int value;
    static int copy_count;
    static bool should_throw;

    ThrowOnCopy(int v = 0) : value(v) {}
    ThrowOnCopy(const ThrowOnCopy& other) {
        if (should_throw) throw std::runtime_error("Copy throw");
        value = other.value;
        ++copy_count;
    }
    ThrowOnCopy(ThrowOnCopy&& other) noexcept : value(other.value) {}
    ThrowOnCopy& operator=(const ThrowOnCopy& other) {
        if (should_throw) throw std::runtime_error("Copy assign throw");
        value = other.value;
        ++copy_count;
        return *this;
    }
    ThrowOnCopy& operator=(ThrowOnCopy&& other) noexcept {
        value = other.value;
        return *this;
    }
    bool operator==(const ThrowOnCopy& other) const { return value == other.value; }
};

int ThrowOnCopy::copy_count = 0;
bool ThrowOnCopy::should_throw = false;

struct NoDefaultCtor {
    int value;
    explicit NoDefaultCtor(int v) : value(v) {}
    bool operator==(const NoDefaultCtor& other) const { return value == other.value; }
};

void run_linked_list_tests(cds::test::TestRunner& runner) {
    runner.run("LinkedList: Default constructor", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> list;
        t.assert_true(list.empty());
        t.assert_eq(list.size(), size_t(0));
    });

    runner.run("LinkedList: Size constructor", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> list(5);
        t.assert_eq(list.size(), size_t(5));
        for (auto& v : list) t.assert_eq(v, 0);
    });

    runner.run("LinkedList: Size + value constructor", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> list(5, 42);
        t.assert_eq(list.size(), size_t(5));
        for (auto& v : list) t.assert_eq(v, 42);
    });

    runner.run("LinkedList: Initializer list constructor", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> list = {1, 2, 3, 4, 5};
        t.assert_eq(list.size(), size_t(5));
        int i = 1;
        for (auto v : list) t.assert_eq(v, i++);
    });

    runner.run("LinkedList: Copy constructor", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> list1 = {1, 2, 3};
        cds::LinkedList<int> list2(list1);
        t.assert_eq(list2.size(), size_t(3));
        int i = 1;
        for (auto v : list2) t.assert_eq(v, i++);
        list1.front() = 100;
        t.assert_eq(list2.front(), 1);
    });

    runner.run("LinkedList: Move constructor", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> list1 = {1, 2, 3};
        cds::LinkedList<int> list2(std::move(list1));
        t.assert_eq(list2.size(), size_t(3));
        t.assert_true(list1.empty());
    });

    runner.run("LinkedList: Copy assignment", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> list1 = {1, 2, 3};
        cds::LinkedList<int> list2 = {4, 5};
        list2 = list1;
        t.assert_eq(list2.size(), size_t(3));
        t.assert_eq(list2.front(), 1);
    });

    runner.run("LinkedList: Move assignment", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> list1 = {1, 2, 3};
        cds::LinkedList<int> list2 = {4, 5};
        list2 = std::move(list1);
        t.assert_eq(list2.size(), size_t(3));
        t.assert_true(list1.empty());
    });

    runner.run("LinkedList: Self copy assignment", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> list = {1, 2, 3};
        list = list;
        t.assert_eq(list.size(), size_t(3));
        t.assert_eq(list.front(), 1);
    });

    runner.run("LinkedList: Self move assignment", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> list = {1, 2, 3};
        list = std::move(list);
        t.assert_eq(list.size(), size_t(3));
        t.assert_eq(list.front(), 1);
    });

    runner.run("LinkedList: push_front/push_back", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> list;
        list.push_front(1);
        list.push_back(2);
        list.push_front(0);
        t.assert_eq(list.size(), size_t(3));
        t.assert_eq(list.front(), 0);
        t.assert_eq(list.back(), 2);
    });

    runner.run("LinkedList: emplace_front/emplace_back", [](cds::test::TestRunner& t) {
        cds::LinkedList<std::pair<int, int>> list;
        list.emplace_front(1, 2);
        list.emplace_back(3, 4);
        t.assert_eq(list.size(), size_t(2));
        t.assert_eq(list.front().first, 1);
        t.assert_eq(list.back().second, 4);
    });

    runner.run("LinkedList: pop_front/pop_back", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> list = {1, 2, 3};
        list.pop_front();
        t.assert_eq(list.size(), size_t(2));
        t.assert_eq(list.front(), 2);
        list.pop_back();
        t.assert_eq(list.size(), size_t(1));
        t.assert_eq(list.front(), 2);
        t.assert_eq(list.back(), 2);
        list.pop_front();
        t.assert_true(list.empty());
    });

    runner.run("LinkedList: front/back", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> list = {1, 2, 3};
        t.assert_eq(list.front(), 1);
        t.assert_eq(list.back(), 3);
        list.front() = 10;
        list.back() = 20;
        t.assert_eq(list.front(), 10);
        t.assert_eq(list.back(), 20);
    });

    runner.run("LinkedList: clear", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> list = {1, 2, 3};
        list.clear();
        t.assert_true(list.empty());
        t.assert_eq(list.size(), size_t(0));
    });

    runner.run("LinkedList: Iterator begin/end", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> list = {1, 2, 3, 4, 5};
        int sum = 0;
        for (int v : list) sum += v;
        t.assert_eq(sum, 15);
    });

    runner.run("LinkedList: Const iterator", [](cds::test::TestRunner& t) {
        const cds::LinkedList<int> list = {1, 2, 3};
        int sum = 0;
        for (int v : list) sum += v;
        t.assert_eq(sum, 6);
    });

    runner.run("LinkedList: Bidirectional iterator", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> list = {1, 2, 3, 4, 5};
        auto it = list.begin();
        t.assert_eq(*it, 1);
        ++it; t.assert_eq(*it, 2);
        ++it; t.assert_eq(*it, 3);
        --it; t.assert_eq(*it, 2);
        --it; t.assert_eq(*it, 1);
    });

    runner.run("LinkedList: insert at position", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> list = {1, 3};
        auto it = list.begin();
        ++it; // points to 3
        list.insert(it, 2);
        t.assert_eq(list.size(), size_t(3));
        int i = 1;
        for (auto v : list) t.assert_eq(v, i++);
    });

    runner.run("LinkedList: insert at begin", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> list = {2, 3};
        list.insert(list.begin(), 1);
        t.assert_eq(list.size(), size_t(3));
        t.assert_eq(list.front(), 1);
    });

    runner.run("LinkedList: insert at end", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> list = {1, 2};
        list.insert(list.end(), 3);
        t.assert_eq(list.size(), size_t(3));
        t.assert_eq(list.back(), 3);
    });

    runner.run("LinkedList: emplace at position", [](cds::test::TestRunner& t) {
        cds::LinkedList<std::pair<int, int>> list = {{1, 1}, {3, 3}};
        auto it = list.begin();
        ++it;
        list.emplace(it, 2, 2);
        t.assert_eq(list.size(), size_t(3));
        t.assert_eq(list.begin()->first, 1);
        t.assert_eq(std::next(list.begin())->first, 2);
        t.assert_eq(std::next(list.begin(), 2)->first, 3);
    });

    runner.run("LinkedList: erase single", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> list = {1, 2, 3};
        auto it = list.begin();
        ++it; // points to 2
        it = list.erase(it);
        t.assert_eq(list.size(), size_t(2));
        t.assert_eq(*it, 3);
        t.assert_eq(list.front(), 1);
        t.assert_eq(list.back(), 3);
    });

    runner.run("LinkedList: erase first", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> list = {1, 2, 3};
        list.erase(list.begin());
        t.assert_eq(list.size(), size_t(2));
        t.assert_eq(list.front(), 2);
    });

    runner.run("LinkedList: erase range", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> list = {1, 2, 3, 4, 5};
        auto it1 = list.begin();
        ++it1; // 2
        auto it2 = it1;
        ++it2; ++it2; // 4
        list.erase(it1, it2);
        t.assert_eq(list.size(), size_t(3));
        int i = 1;
        for (auto v : list) { t.assert_eq(v, i); i += 2; } // 1, 3, 5
    });

    runner.run("LinkedList: NoDefaultCtor elements", [](cds::test::TestRunner& t) {
        cds::LinkedList<NoDefaultCtor> list;
        list.emplace_back(1);
        list.emplace_back(2);
        t.assert_eq(list.size(), size_t(2));
        t.assert_eq(list.front().value, 1);
        t.assert_eq(list.back().value, 2);
    });

    runner.run("LinkedList: Exception safety on copy throw", [](cds::test::TestRunner& t) {
        ThrowOnCopy::copy_count = 0;
        ThrowOnCopy::should_throw = true;
        cds::LinkedList<ThrowOnCopy> list;
        list.emplace_back(1);
        try {
            cds::LinkedList<ThrowOnCopy> list2(list);
            t.assert_true(false);
        } catch (...) {
            t.assert_eq(ThrowOnCopy::copy_count, 0);
        }
        ThrowOnCopy::should_throw = false;
    });

    runner.run("LinkedList: swap", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> list1 = {1, 2, 3};
        cds::LinkedList<int> list2 = {4, 5};
        list1.swap(list2);
        t.assert_eq(list1.size(), size_t(2));
        t.assert_eq(list1.front(), 4);
        t.assert_eq(list2.size(), size_t(3));
        t.assert_eq(list2.front(), 1);
    });

    runner.run("LinkedList: Comparisons", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> a = {1, 2, 3};
        cds::LinkedList<int> b = {1, 2, 3};
        cds::LinkedList<int> c = {1, 2, 4};
        t.assert_true(a == b);
        t.assert_false(a != b);
        t.assert_true(a < c);
        t.assert_true(c > a);
        t.assert_true(a <= b);
        t.assert_true(a >= b);
    });

    runner.run("LinkedList: STL algorithms", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> list = {5, 3, 1, 4, 2};
        int count = std::count(list.begin(), list.end(), 3);
        t.assert_eq(count, 1);
        auto it = std::find(list.begin(), list.end(), 4);
        t.assert_true(it != list.end());
        t.assert_eq(*it, 4);
    });

    runner.run("LinkedList: Large number of operations", [](cds::test::TestRunner& t) {
        cds::LinkedList<int> list;
        for (int i = 0; i < 1000; ++i) list.push_back(i);
        t.assert_eq(list.size(), size_t(1000));
        for (int i = 0; i < 500; ++i) list.pop_front();
        t.assert_eq(list.size(), size_t(500));
        t.assert_eq(list.front(), 500);
    });
}