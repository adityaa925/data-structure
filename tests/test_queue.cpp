#include "test_framework.hpp"
#include <cds/queue.hpp>
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

void run_queue_tests(cds::test::TestRunner& runner) {
    using cds::Queue;

    runner.run("Queue: Default constructor", [](cds::test::TestRunner& t) {
        Queue<int> q;
        t.assert_true(q.empty());
        t.assert_eq(q.size(), size_t(0));
    });

    runner.run("Queue: Size constructor", [](cds::test::TestRunner& t) {
        Queue<int> q(5);
        t.assert_eq(q.size(), size_t(5));
        for (auto& v : q) t.assert_eq(v, 0);
    });

    runner.run("Queue: Size + value constructor", [](cds::test::TestRunner& t) {
        Queue<int> q(5, 42);
        t.assert_eq(q.size(), size_t(5));
        for (auto& v : q) t.assert_eq(v, 42);
    });

    runner.run("Queue: Initializer list constructor", [](cds::test::TestRunner& t) {
        Queue<int> q = {1, 2, 3, 4, 5};
        t.assert_eq(q.size(), size_t(5));
        int i = 1;
        for (auto v : q) t.assert_eq(v, i++);
    });

    runner.run("Queue: Copy constructor", [](cds::test::TestRunner& t) {
        Queue<int> q1 = {1, 2, 3};
        Queue<int> q2(q1);
        t.assert_eq(q2.size(), size_t(3));
        int i = 1;
        for (auto v : q2) t.assert_eq(v, i++);
        q1.pop();
        t.assert_eq(q2.front(), 1);
    });

    runner.run("Queue: Move constructor", [](cds::test::TestRunner& t) {
        Queue<int> q1 = {1, 2, 3};
        Queue<int> q2(std::move(q1));
        t.assert_eq(q2.size(), size_t(3));
        t.assert_true(q1.empty());
    });

    runner.run("Queue: Copy assignment", [](cds::test::TestRunner& t) {
        Queue<int> q1 = {1, 2, 3};
        Queue<int> q2 = {4, 5};
        q2 = q1;
        t.assert_eq(q2.size(), size_t(3));
        t.assert_eq(q2.front(), 1);
    });

    runner.run("Queue: Move assignment", [](cds::test::TestRunner& t) {
        Queue<int> q1 = {1, 2, 3};
        Queue<int> q2 = {4, 5};
        q2 = std::move(q1);
        t.assert_eq(q2.size(), size_t(3));
        t.assert_true(q1.empty());
    });

    runner.run("Queue: Self copy assignment", [](cds::test::TestRunner& t) {
        Queue<int> q = {1, 2, 3};
        q = q;
        t.assert_eq(q.size(), size_t(3));
        t.assert_eq(q.front(), 1);
    });

    runner.run("Queue: Self move assignment", [](cds::test::TestRunner& t) {
        Queue<int> q = {1, 2, 3};
        q = std::move(q);
        t.assert_eq(q.size(), size_t(3));
        t.assert_eq(q.front(), 1);
    });

    runner.run("Queue: push/pop (FIFO)", [](cds::test::TestRunner& t) {
        Queue<int> q;
        q.push(1);
        q.push(2);
        q.push(3);
        t.assert_eq(q.size(), size_t(3));
        t.assert_eq(q.front(), 1);
        t.assert_eq(q.back(), 3);
        q.pop();
        t.assert_eq(q.front(), 2);
        t.assert_eq(q.size(), size_t(2));
        q.pop();
        t.assert_eq(q.front(), 3);
        q.pop();
        t.assert_true(q.empty());
    });

    runner.run("Queue: push move", [](cds::test::TestRunner& t) {
        Queue<std::string> q;
        std::string str = "hello";
        q.push(std::move(str));
        t.assert_eq(q.size(), size_t(1));
        t.assert_eq(q.front(), "hello");
    });

    runner.run("Queue: emplace", [](cds::test::TestRunner& t) {
        Queue<std::pair<int, int>> q;
        q.emplace(1, 2);
        q.emplace(3, 4);
        t.assert_eq(q.size(), size_t(2));
        t.assert_eq(q.front().first, 1);
        t.assert_eq(q.back().second, 4);
    });

    runner.run("Queue: front/back", [](cds::test::TestRunner& t) {
        Queue<int> q = {1, 2, 3};
        t.assert_eq(q.front(), 1);
        t.assert_eq(q.back(), 3);
        q.front() = 10;
        q.back() = 20;
        t.assert_eq(q.front(), 10);
        t.assert_eq(q.back(), 20);
    });

    runner.run("Queue: clear", [](cds::test::TestRunner& t) {
        Queue<int> q = {1, 2, 3};
        q.clear();
        t.assert_true(q.empty());
        t.assert_eq(q.size(), size_t(0));
    });

    runner.run("Queue: Iterator begin/end", [](cds::test::TestRunner& t) {
        Queue<int> q = {1, 2, 3, 4, 5};
        int sum = 0;
        for (int v : q) sum += v;
        t.assert_eq(sum, 15);
    });

    runner.run("Queue: Const iterator", [](cds::test::TestRunner& t) {
        const Queue<int> q = {1, 2, 3};
        int sum = 0;
        for (int v : q) sum += v;
        t.assert_eq(sum, 6);
    });

    runner.run("Queue: Random access iterator arithmetic", [](cds::test::TestRunner& t) {
        Queue<int> q = {1, 2, 3, 4, 5};
        auto it = q.begin();
        t.assert_eq(*it, 1);
        ++it; t.assert_eq(*it, 2);
        it += 2; t.assert_eq(*it, 4);
        --it; t.assert_eq(*it, 3);
        it -= 1; t.assert_eq(*it, 2);
        t.assert_eq(it[2], 4);
        t.assert_eq(q.end() - q.begin(), 5);
    });

    runner.run("Queue: Wrap-around behavior", [](cds::test::TestRunner& t) {
        Queue<int> q(3);
        q.push(1); q.push(2); q.push(3);
        t.assert_eq(q.size(), size_t(3));
        q.pop(); q.pop();
        q.push(4); q.push(5);
        t.assert_eq(q.size(), size_t(3));
        t.assert_eq(q.front(), 3);
        t.assert_eq(q.back(), 5);
    });

    runner.run("Queue: Dynamic growth", [](cds::test::TestRunner& t) {
        Queue<int> q;
        for (int i = 0; i < 100; ++i) q.push(i);
        t.assert_eq(q.size(), size_t(100));
        for (int i = 0; i < 100; ++i) {
            t.assert_eq(q.front(), i);
            q.pop();
        }
        t.assert_true(q.empty());
    });

    runner.run("Queue: reserve", [](cds::test::TestRunner& t) {
        Queue<int> q;
        q.reserve(100);
        t.assert_true(q.capacity() >= 100);
    });

    runner.run("Queue: shrink_to_fit", [](cds::test::TestRunner& t) {
        Queue<int> q;
        q.reserve(100);
        q.push(1);
        q.shrink_to_fit();
        t.assert_eq(q.capacity(), size_t(1));
    });

    runner.run("Queue: NoDefaultCtor elements", [](cds::test::TestRunner& t) {
        Queue<NoDefaultCtor> q;
        q.emplace(1);
        q.emplace(2);
        t.assert_eq(q.size(), size_t(2));
        t.assert_eq(q.front().value, 1);
        t.assert_eq(q.back().value, 2);
    });

    runner.run("Queue: Exception safety on copy throw", [](cds::test::TestRunner& t) {
        ThrowOnCopy::copy_count = 0;
        ThrowOnCopy::should_throw = true;
        Queue<ThrowOnCopy> q;
        q.emplace(1);
        try {
            Queue<ThrowOnCopy> q2(q);
            t.assert_true(false);
        } catch (...) {
            t.assert_eq(ThrowOnCopy::copy_count, 0);
        }
        ThrowOnCopy::should_throw = false;
    });

    runner.run("Queue: swap", [](cds::test::TestRunner& t) {
        Queue<int> q1 = {1, 2, 3};
        Queue<int> q2 = {4, 5};
        q1.swap(q2);
        t.assert_eq(q1.size(), size_t(2));
        t.assert_eq(q1.front(), 4);
        t.assert_eq(q2.size(), size_t(3));
        t.assert_eq(q2.front(), 1);
    });

    runner.run("Queue: Comparisons", [](cds::test::TestRunner& t) {
        Queue<int> a = {1, 2, 3};
        Queue<int> b = {1, 2, 3};
        Queue<int> c = {1, 2, 4};
        t.assert_true(a == b);
        t.assert_false(a != b);
        t.assert_true(a < c);
        t.assert_true(c > a);
        t.assert_true(a <= b);
        t.assert_true(a >= b);
    });

    runner.run("Queue: STL algorithms", [](cds::test::TestRunner& t) {
        Queue<int> q = {5, 3, 1, 4, 2};
        int count = std::count(q.begin(), q.end(), 3);
        t.assert_eq(count, 1);
        auto it = std::find(q.begin(), q.end(), 4);
        t.assert_true(it != q.end());
        t.assert_eq(*it, 4);
    });

    runner.run("Queue: Large number of operations", [](cds::test::TestRunner& t) {
        Queue<int> q;
        for (int i = 0; i < 10000; ++i) q.push(i);
        t.assert_eq(q.size(), size_t(10000));
        for (int i = 0; i < 5000; ++i) q.pop();
        t.assert_eq(q.size(), size_t(5000));
        t.assert_eq(q.front(), 5000);
    });

    runner.run("Queue: Alternating push/pop", [](cds::test::TestRunner& t) {
        Queue<int> q;
        for (int i = 0; i < 1000; ++i) {
            q.push(i);
            if (i % 2 == 0) q.pop();
        }
        t.assert_eq(q.size(), size_t(500));
    });
}