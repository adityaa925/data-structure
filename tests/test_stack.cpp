#include "test_framework.hpp"
#include <cds/stack.hpp>
#include <cds/dynamic_array.hpp>
#include <cds/linked_list.hpp>
#include <stdexcept>

void run_stack_tests(cds::test::TestRunner& runner) {
    using cds::Stack;
    using cds::DynamicArray;
    using cds::LinkedList;
    using cds::NodeStack;

    runner.run("Stack: Default constructor (DynamicArray)", [](cds::test::TestRunner& t) {
        Stack<int> s;
        t.assert_true(s.empty());
        t.assert_eq(s.size(), size_t(0));
    });

    runner.run("Stack: Default constructor (LinkedList)", [](cds::test::TestRunner& t) {
        NodeStack<int> s;
        t.assert_true(s.empty());
        t.assert_eq(s.size(), size_t(0));
    });

    runner.run("Stack: push/pop", [](cds::test::TestRunner& t) {
        Stack<int> s;
        s.push(1);
        s.push(2);
        s.push(3);
        t.assert_eq(s.size(), size_t(3));
        t.assert_eq(s.top(), 3);
        s.pop();
        t.assert_eq(s.top(), 2);
        s.pop();
        t.assert_eq(s.top(), 1);
        s.pop();
        t.assert_true(s.empty());
    });

    runner.run("Stack: push move", [](cds::test::TestRunner& t) {
        Stack<std::string> s;
        std::string str = "hello";
        s.push(std::move(str));
        t.assert_eq(s.size(), size_t(1));
        t.assert_eq(s.top(), "hello");
    });

    runner.run("Stack: emplace", [](cds::test::TestRunner& t) {
        Stack<std::pair<int, int>> s;
        s.emplace(1, 2);
        s.emplace(3, 4);
        t.assert_eq(s.size(), size_t(2));
        t.assert_eq(s.top().first, 3);
        t.assert_eq(s.top().second, 4);
    });

    runner.run("Stack: top() const", [](cds::test::TestRunner& t) {
        const Stack<int> s = []() { Stack<int> tmp; tmp.push(42); return tmp; }();
        t.assert_eq(s.top(), 42);
    });

    runner.run("Stack: Constructor from container", [](cds::test::TestRunner& t) {
        DynamicArray<int> arr = {1, 2, 3};
        Stack<int> s(arr);
        t.assert_eq(s.size(), size_t(3));
        t.assert_eq(s.top(), 3);
    });

    runner.run("Stack: Constructor from initializer_list", [](cds::test::TestRunner& t) {
        Stack<int> s = {1, 2, 3};
        t.assert_eq(s.size(), size_t(3));
        t.assert_eq(s.top(), 3);
    });

    runner.run("Stack: Copy constructor", [](cds::test::TestRunner& t) {
        Stack<int> s1;
        s1.push(1);
        s1.push(2);
        Stack<int> s2(s1);
        t.assert_eq(s2.size(), size_t(2));
        t.assert_eq(s2.top(), 2);
        s1.pop();
        t.assert_eq(s2.top(), 2);
    });

    runner.run("Stack: Move constructor", [](cds::test::TestRunner& t) {
        Stack<int> s1;
        s1.push(1);
        s1.push(2);
        Stack<int> s2(std::move(s1));
        t.assert_eq(s2.size(), size_t(2));
        t.assert_eq(s2.top(), 2);
        t.assert_true(s1.empty());
    });

    runner.run("Stack: Copy assignment", [](cds::test::TestRunner& t) {
        Stack<int> s1;
        s1.push(1);
        s1.push(2);
        Stack<int> s2;
        s2 = s1;
        t.assert_eq(s2.size(), size_t(2));
        t.assert_eq(s2.top(), 2);
    });

    runner.run("Stack: Move assignment", [](cds::test::TestRunner& t) {
        Stack<int> s1;
        s1.push(1);
        s1.push(2);
        Stack<int> s2;
        s2 = std::move(s1);
        t.assert_eq(s2.size(), size_t(2));
        t.assert_true(s1.empty());
    });

    runner.run("Stack: Self copy assignment", [](cds::test::TestRunner& t) {
        Stack<int> s;
        s.push(1);
        s = s;
        t.assert_eq(s.size(), size_t(1));
        t.assert_eq(s.top(), 1);
    });

    runner.run("Stack: Self move assignment", [](cds::test::TestRunner& t) {
        Stack<int> s;
        s.push(1);
        s = std::move(s);
        t.assert_eq(s.size(), size_t(1));
        t.assert_eq(s.top(), 1);
    });

    runner.run("Stack: swap", [](cds::test::TestRunner& t) {
        Stack<int> s1;
        s1.push(1);
        s1.push(2);
        Stack<int> s2;
        s2.push(3);
        s1.swap(s2);
        t.assert_eq(s1.size(), size_t(1));
        t.assert_eq(s1.top(), 3);
        t.assert_eq(s2.size(), size_t(2));
        t.assert_eq(s2.top(), 2);
    });

    runner.run("Stack: Comparisons", [](cds::test::TestRunner& t) {
        Stack<int> a;
        a.push(1); a.push(2); a.push(3);
        Stack<int> b;
        b.push(1); b.push(2); b.push(3);
        Stack<int> c;
        c.push(1); c.push(2); c.push(4);
        t.assert_true(a == b);
        t.assert_false(a != b);
        t.assert_true(a < c);
        t.assert_true(c > a);
        t.assert_true(a <= b);
        t.assert_true(a >= b);
    });

    runner.run("Stack: NodeStack (LinkedList backed) operations", [](cds::test::TestRunner& t) {
        NodeStack<int> s;
        s.push(1);
        s.push(2);
        s.push(3);
        t.assert_eq(s.size(), size_t(3));
        t.assert_eq(s.top(), 3);
        s.pop();
        t.assert_eq(s.top(), 2);
        s.pop();
        t.assert_eq(s.top(), 1);
        s.pop();
        t.assert_true(s.empty());
    });

    runner.run("Stack: NodeStack emplace", [](cds::test::TestRunner& t) {
        NodeStack<std::pair<int, int>> s;
        s.emplace(1, 2);
        s.emplace(3, 4);
        t.assert_eq(s.size(), size_t(2));
        t.assert_eq(s.top().first, 3);
    });

    runner.run("Stack: NodeStack copy/move", [](cds::test::TestRunner& t) {
        NodeStack<int> s1;
        s1.push(1);
        s1.push(2);
        NodeStack<int> s2(s1);
        t.assert_eq(s2.size(), size_t(2));
        NodeStack<int> s3(std::move(s1));
        t.assert_eq(s3.size(), size_t(2));
        t.assert_true(s1.empty());
    });

    runner.run("Stack: reserve on underlying DynamicArray", [](cds::test::TestRunner& t) {
        Stack<int, DynamicArray<int>> s;
        s.c_.reserve(100);
        t.assert_true(s.c_.capacity() >= 100);
        for (int i = 0; i < 50; ++i) s.push(i);
        t.assert_eq(s.size(), size_t(50));
    });
}