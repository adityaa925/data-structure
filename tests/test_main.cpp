#include "test_framework.hpp"
#include <cds/dynamic_array.hpp>
#include <cds/linked_list.hpp>
#include <cds/stack.hpp>
#include <cds/queue.hpp>

extern void run_dynamic_array_tests(cds::test::TestRunner&);
extern void run_linked_list_tests(cds::test::TestRunner&);
extern void run_stack_tests(cds::test::TestRunner&);
extern void run_queue_tests(cds::test::TestRunner&);

int main() {
    cds::test::TestRunner runner;

    run_dynamic_array_tests(runner);
    run_linked_list_tests(runner);
    run_stack_tests(runner);
    run_queue_tests(runner);

    return runner.summary();
}