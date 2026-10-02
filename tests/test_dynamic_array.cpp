#include "test_framework.hpp"
#include <cds/dynamic_array.hpp>
#include <vector>
#include <algorithm>
#include <stdexcept>

struct ThrowOnCopy {
    int value;
    static int copy_count;
    static int move_count;
    static bool should_throw;

    ThrowOnCopy(int v = 0) : value(v) {}
    ThrowOnCopy(const ThrowOnCopy& other) {
        if (should_throw) throw std::runtime_error("Copy throw");
        value = other.value;
        ++copy_count;
    }
    ThrowOnCopy(ThrowOnCopy&& other) noexcept : value(other.value) {
        ++move_count;
    }
    ThrowOnCopy& operator=(const ThrowOnCopy& other) {
        if (should_throw) throw std::runtime_error("Copy assign throw");
        value = other.value;
        ++copy_count;
        return *this;
    }
    ThrowOnCopy& operator=(ThrowOnCopy&& other) noexcept {
        value = other.value;
        ++move_count;
        return *this;
    }
    bool operator==(const ThrowOnCopy& other) const { return value == other.value; }
};

int ThrowOnCopy::copy_count = 0;
int ThrowOnCopy::move_count = 0;
bool ThrowOnCopy::should_throw = false;

struct NoDefaultCtor {
    int value;
    explicit NoDefaultCtor(int v) : value(v) {}
    bool operator==(const NoDefaultCtor& other) const { return value == other.value; }
};

void run_dynamic_array_tests(cds::test::TestRunner& runner) {
    runner.run("DynamicArray: Default constructor", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr;
        t.assert_true(arr.empty());
        t.assert_eq(arr.size(), size_t(0));
        t.assert_eq(arr.capacity(), size_t(0));
    });

    runner.run("DynamicArray: Size constructor", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr(5);
        t.assert_eq(arr.size(), size_t(5));
        t.assert_true(arr.capacity() >= 5);
        for (size_t i = 0; i < 5; ++i) t.assert_eq(arr[i], 0);
    });

    runner.run("DynamicArray: Size + value constructor", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr(5, 42);
        t.assert_eq(arr.size(), size_t(5));
        for (size_t i = 0; i < 5; ++i) t.assert_eq(arr[i], 42);
    });

    runner.run("DynamicArray: Initializer list constructor", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr = {1, 2, 3, 4, 5};
        t.assert_eq(arr.size(), size_t(5));
        for (size_t i = 0; i < 5; ++i) t.assert_eq(arr[i], int(i + 1));
    });

    runner.run("DynamicArray: Copy constructor", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr1 = {1, 2, 3};
        cds::DynamicArray<int> arr2(arr1);
        t.assert_eq(arr2.size(), size_t(3));
        t.assert_eq(arr2[0], 1);
        t.assert_eq(arr2[1], 2);
        t.assert_eq(arr2[2], 3);
        arr1[0] = 100;
        t.assert_eq(arr2[0], 1);
    });

    runner.run("DynamicArray: Move constructor", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr1 = {1, 2, 3};
        cds::DynamicArray<int> arr2(std::move(arr1));
        t.assert_eq(arr2.size(), size_t(3));
        t.assert_eq(arr2[0], 1);
        t.assert_true(arr1.empty());
    });

    runner.run("DynamicArray: Copy assignment", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr1 = {1, 2, 3};
        cds::DynamicArray<int> arr2 = {4, 5};
        arr2 = arr1;
        t.assert_eq(arr2.size(), size_t(3));
        t.assert_eq(arr2[0], 1);
        arr1[0] = 100;
        t.assert_eq(arr2[0], 1);
    });

    runner.run("DynamicArray: Move assignment", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr1 = {1, 2, 3};
        cds::DynamicArray<int> arr2 = {4, 5};
        arr2 = std::move(arr1);
        t.assert_eq(arr2.size(), size_t(3));
        t.assert_eq(arr2[0], 1);
        t.assert_true(arr1.empty());
    });

    runner.run("DynamicArray: Self copy assignment", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr = {1, 2, 3};
        arr = arr;
        t.assert_eq(arr.size(), size_t(3));
        t.assert_eq(arr[0], 1);
    });

    runner.run("DynamicArray: Self move assignment", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr = {1, 2, 3};
        arr = std::move(arr);
        t.assert_eq(arr.size(), size_t(3));
        t.assert_eq(arr[0], 1);
    });

    runner.run("DynamicArray: push_back", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr;
        arr.push_back(1);
        arr.push_back(2);
        arr.push_back(3);
        t.assert_eq(arr.size(), size_t(3));
        t.assert_eq(arr[0], 1);
        t.assert_eq(arr[1], 2);
        t.assert_eq(arr[2], 3);
    });

    runner.run("DynamicArray: push_back move", [](cds::test::TestRunner& t) {
        cds::DynamicArray<std::string> arr;
        std::string s = "hello";
        arr.push_back(std::move(s));
        t.assert_eq(arr.size(), size_t(1));
        t.assert_eq(arr[0], "hello");
    });

    runner.run("DynamicArray: emplace_back", [](cds::test::TestRunner& t) {
        cds::DynamicArray<std::pair<int, int>> arr;
        arr.emplace_back(1, 2);
        arr.emplace_back(3, 4);
        t.assert_eq(arr.size(), size_t(2));
        t.assert_eq(arr[0].first, 1);
        t.assert_eq(arr[0].second, 2);
    });

    runner.run("DynamicArray: pop_back", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr = {1, 2, 3};
        arr.pop_back();
        t.assert_eq(arr.size(), size_t(2));
        t.assert_eq(arr[1], 2);
        arr.pop_back();
        arr.pop_back();
        t.assert_true(arr.empty());
    });

    runner.run("DynamicArray: operator[]", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr = {1, 2, 3};
        t.assert_eq(arr[0], 1);
        arr[0] = 10;
        t.assert_eq(arr[0], 10);
    });

    runner.run("DynamicArray: at with bounds checking", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr = {1, 2, 3};
        t.assert_eq(arr.at(0), 1);
        t.assert_eq(arr.at(2), 3);
        t.assert_throws([&]() { arr.at(3); }, "out_of_range expected");
        t.assert_throws([&]() { arr.at(100); }, "out_of_range expected");
    });

    runner.run("DynamicArray: front/back", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr = {1, 2, 3};
        t.assert_eq(arr.front(), 1);
        t.assert_eq(arr.back(), 3);
        arr.front() = 10;
        arr.back() = 20;
        t.assert_eq(arr[0], 10);
        t.assert_eq(arr[2], 20);
    });

    runner.run("DynamicArray: reserve", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr;
        arr.reserve(100);
        t.assert_true(arr.capacity() >= 100);
        t.assert_eq(arr.size(), size_t(0));
    });

    runner.run("DynamicArray: shrink_to_fit", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr;
        arr.reserve(100);
        arr.push_back(1);
        arr.shrink_to_fit();
        t.assert_eq(arr.capacity(), size_t(1));
    });

    runner.run("DynamicArray: clear", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr = {1, 2, 3};
        arr.clear();
        t.assert_true(arr.empty());
        t.assert_eq(arr.size(), size_t(0));
    });

    runner.run("DynamicArray: resize larger", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr = {1, 2};
        arr.resize(5);
        t.assert_eq(arr.size(), size_t(5));
        t.assert_eq(arr[0], 1);
        t.assert_eq(arr[1], 2);
        t.assert_eq(arr[2], 0);
        t.assert_eq(arr[4], 0);
    });

    runner.run("DynamicArray: resize larger with value", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr = {1, 2};
        arr.resize(5, 42);
        t.assert_eq(arr.size(), size_t(5));
        t.assert_eq(arr[2], 42);
        t.assert_eq(arr[4], 42);
    });

    runner.run("DynamicArray: resize smaller", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr = {1, 2, 3, 4, 5};
        arr.resize(2);
        t.assert_eq(arr.size(), size_t(2));
        t.assert_eq(arr[0], 1);
        t.assert_eq(arr[1], 2);
    });

    runner.run("DynamicArray: Iterator begin/end", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr = {1, 2, 3, 4, 5};
        int sum = 0;
        for (int v : arr) sum += v;
        t.assert_eq(sum, 15);
    });

    runner.run("DynamicArray: Const iterator", [](cds::test::TestRunner& t) {
        const cds::DynamicArray<int> arr = {1, 2, 3};
        int sum = 0;
        for (int v : arr) sum += v;
        t.assert_eq(sum, 6);
    });

    runner.run("DynamicArray: Random access iterator arithmetic", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr = {1, 2, 3, 4, 5};
        auto it = arr.begin();
        t.assert_eq(*it, 1);
        ++it; t.assert_eq(*it, 2);
        it += 2; t.assert_eq(*it, 4);
        --it; t.assert_eq(*it, 3);
        it -= 1; t.assert_eq(*it, 2);
        t.assert_eq(it[2], 4);
        t.assert_eq(arr.end() - arr.begin(), 5);
    });

    runner.run("DynamicArray: STL algorithms", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr = {5, 3, 1, 4, 2};
        std::sort(arr.begin(), arr.end());
        t.assert_eq(arr[0], 1);
        t.assert_eq(arr[4], 5);
        auto it = std::find(arr.begin(), arr.end(), 3);
        t.assert_true(it != arr.end());
        t.assert_eq(*it, 3);
    });

    runner.run("DynamicArray: NoDefaultCtor elements", [](cds::test::TestRunner& t) {
        cds::DynamicArray<NoDefaultCtor> arr;
        arr.emplace_back(1);
        arr.emplace_back(2);
        t.assert_eq(arr.size(), size_t(2));
        t.assert_eq(arr[0].value, 1);
        t.assert_eq(arr[1].value, 2);
    });

    runner.run("DynamicArray: Exception safety on copy throw", [](cds::test::TestRunner& t) {
        ThrowOnCopy::copy_count = 0;
        ThrowOnCopy::should_throw = true;
        cds::DynamicArray<ThrowOnCopy> arr;
        arr.emplace_back(1);
        arr.emplace_back(2);
        try {
            cds::DynamicArray<ThrowOnCopy> arr2(arr);
            t.assert_true(false); // should not reach
        } catch (...) {
            t.assert_eq(ThrowOnCopy::copy_count, 0);
        }
        ThrowOnCopy::should_throw = false;
    });

    runner.run("DynamicArray: Exception safety on reallocation throw", [](cds::test::TestRunner& t) {
        ThrowOnCopy::copy_count = 0;
        ThrowOnCopy::move_count = 0;
        ThrowOnCopy::should_throw = true;
        cds::DynamicArray<ThrowOnCopy> arr;
        arr.emplace_back(1);
        try {
            arr.push_back(ThrowOnCopy(2));
            t.assert_true(false);
        } catch (...) {
        }
        ThrowOnCopy::should_throw = false;
        t.assert_eq(arr.size(), size_t(1));
        t.assert_eq(arr[0].value, 1);
    });

    runner.run("DynamicArray: swap", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr1 = {1, 2, 3};
        cds::DynamicArray<int> arr2 = {4, 5};
        arr1.swap(arr2);
        t.assert_eq(arr1.size(), size_t(2));
        t.assert_eq(arr1[0], 4);
        t.assert_eq(arr2.size(), size_t(3));
        t.assert_eq(arr2[0], 1);
    });

    runner.run("DynamicArray: Comparisons", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> a = {1, 2, 3};
        cds::DynamicArray<int> b = {1, 2, 3};
        cds::DynamicArray<int> c = {1, 2, 4};
        t.assert_true(a == b);
        t.assert_false(a != b);
        t.assert_true(a < c);
        t.assert_true(c > a);
        t.assert_true(a <= b);
        t.assert_true(a >= b);
    });

    runner.run("DynamicArray: data() pointer", [](cds::test::TestRunner& t) {
        cds::DynamicArray<int> arr = {1, 2, 3};
        int* ptr = arr.data();
        t.assert_eq(ptr[0], 1);
        t.assert_eq(ptr[2], 3);
    });
}