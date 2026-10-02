#include <cds/dynamic_array.hpp>
#include <cds/linked_list.hpp>
#include <cds/stack.hpp>
#include <cds/queue.hpp>
#include <vector>
#include <list>
#include <deque>
#include <stack>
#include <queue>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <random>
#include <memory>

using namespace std::chrono;

template <typename Fn>
double benchmark_ms(Fn&& fn, int iterations = 10) {
    double total = 0;
    for (int i = 0; i < iterations; ++i) {
        auto start = high_resolution_clock::now();
        fn();
        auto end = high_resolution_clock::now();
        total += duration<double, std::milli>(end - start).count();
    }
    return total / iterations;
}

void print_result(const std::string& name, double cds_time, double stl_time) {
    double speedup = stl_time / cds_time;
    std::cout << std::left << std::setw(40) << name
              << " | CDS: " << std::right << std::setw(10) << std::fixed << std::setprecision(3) << cds_time << " ms"
              << " | STL: " << std::setw(10) << stl_time << " ms"
              << " | Speedup: " << std::setw(6) << std::setprecision(2) << speedup << "x" << std::endl;
}

int main() {
    std::cout << "=== Custom Data Structures vs STL Benchmarks ===\n\n";
    std::cout << std::left << std::setw(40) << "Test"
              << " | CDS (ms)     | STL (ms)     | Speedup" << std::endl;
    std::cout << std::string(85, '-') << std::endl;

    const size_t N = 100000;
    const size_t SMALL_N = 10000;

    std::vector<int> rand_data(N);
    std::iota(rand_data.begin(), rand_data.end(), 0);
    std::mt19937 rng(42);
    std::shuffle(rand_data.begin(), rand_data.end(), rng);

    // DynamicArray vs vector
    {
        print_result("DynamicArray/vector: push_back (100k)", 
            benchmark_ms([&]() {
                cds::DynamicArray<int> arr;
                arr.reserve(N);
                for (int v : rand_data) arr.push_back(v);
            }),
            benchmark_ms([&]() {
                std::vector<int> vec;
                vec.reserve(N);
                for (int v : rand_data) vec.push_back(v);
            })
        );
    }

    {
        print_result("DynamicArray/vector: emplace_back (100k)", 
            benchmark_ms([&]() {
                cds::DynamicArray<std::pair<int,int>> arr;
                arr.reserve(N);
                for (int v : rand_data) arr.emplace_back(v, v*2);
            }),
            benchmark_ms([&]() {
                std::vector<std::pair<int,int>> vec;
                vec.reserve(N);
                for (int v : rand_data) vec.emplace_back(v, v*2);
            })
        );
    }

    {
        cds::DynamicArray<int> cds_arr;
        cds_arr.reserve(N);
        std::vector<int> std_vec;
        std_vec.reserve(N);
        for (int v : rand_data) { cds_arr.push_back(v); std_vec.push_back(v); }

        print_result("DynamicArray/vector: random access (100k)", 
            benchmark_ms([&]() {
                volatile long sum = 0;
                for (size_t i = 0; i < N; ++i) sum += cds_arr[i];
            }, 100),
            benchmark_ms([&]() {
                volatile long sum = 0;
                for (size_t i = 0; i < N; ++i) sum += std_vec[i];
            }, 100)
        );
    }

    {
        cds::DynamicArray<int> cds_arr;
        cds_arr.reserve(N);
        std::vector<int> std_vec;
        std_vec.reserve(N);
        for (int v : rand_data) { cds_arr.push_back(v); std_vec.push_back(v); }

        print_result("DynamicArray/vector: iteration (100k)", 
            benchmark_ms([&]() {
                volatile long sum = 0;
                for (int v : cds_arr) sum += v;
            }, 100),
            benchmark_ms([&]() {
                volatile long sum = 0;
                for (int v : std_vec) sum += v;
            }, 100)
        );
    }

    {
        cds::DynamicArray<int> cds_arr;
        cds_arr.reserve(N);
        std::vector<int> std_vec;
        std_vec.reserve(N);
        for (int v : rand_data) { cds_arr.push_back(v); std_vec.push_back(v); }

        print_result("DynamicArray/vector: sort (100k)", 
            benchmark_ms([&]() {
                cds::DynamicArray<int> arr = cds_arr;
                std::sort(arr.begin(), arr.end());
            }, 10),
            benchmark_ms([&]() {
                std::vector<int> vec = std_vec;
                std::sort(vec.begin(), vec.end());
            }, 10)
        );
    }

    // LinkedList vs list
    {
        print_result("LinkedList/list: push_back (10k)", 
            benchmark_ms([&]() {
                cds::LinkedList<int> list;
                for (int i = 0; i < SMALL_N; ++i) list.push_back(i);
            }, 50),
            benchmark_ms([&]() {
                std::list<int> list;
                for (int i = 0; i < SMALL_N; ++i) list.push_back(i);
            }, 50)
        );
    }

    {
        cds::LinkedList<int> cds_list;
        std::list<int> std_list;
        for (int i = 0; i < SMALL_N; ++i) { cds_list.push_back(i); std_list.push_back(i); }

        print_result("LinkedList/list: iteration (10k)", 
            benchmark_ms([&]() {
                volatile long sum = 0;
                for (int v : cds_list) sum += v;
            }, 100),
            benchmark_ms([&]() {
                volatile long sum = 0;
                for (int v : std_list) sum += v;
            }, 100)
        );
    }

    {
        cds::LinkedList<int> cds_list;
        std::list<int> std_list;
        for (int i = 0; i < SMALL_N; ++i) { cds_list.push_back(i); std_list.push_back(i); }

        print_result("LinkedList/list: insert at middle (10k)", 
            benchmark_ms([&]() {
                cds::LinkedList<int> list = cds_list;
                auto it = list.begin();
                std::advance(it, SMALL_N / 2);
                for (int i = 0; i < 100; ++i) list.insert(it, i);
            }, 50),
            benchmark_ms([&]() {
                std::list<int> list = std_list;
                auto it = list.begin();
                std::advance(it, SMALL_N / 2);
                for (int i = 0; i < 100; ++i) list.insert(it, i);
            }, 50)
        );
    }

    {
        cds::LinkedList<int> cds_list;
        std::list<int> std_list;
        for (int i = 0; i < SMALL_N; ++i) { cds_list.push_back(i); std_list.push_back(i); }

        print_result("LinkedList/list: erase at middle (10k)", 
            benchmark_ms([&]() {
                cds::LinkedList<int> list = cds_list;
                auto it = list.begin();
                std::advance(it, SMALL_N / 2);
                for (int i = 0; i < 100; ++i) { it = list.erase(it); if (it == list.end()) it = list.begin(); }
            }, 50),
            benchmark_ms([&]() {
                std::list<int> list = std_list;
                auto it = list.begin();
                std::advance(it, SMALL_N / 2);
                for (int i = 0; i < 100; ++i) { it = list.erase(it); if (it == list.end()) it = list.begin(); }
            }, 50)
        );
    }

    // Stack vs std::stack
    {
        print_result("Stack (DynamicArray)/std::stack: push/pop (100k)", 
            benchmark_ms([&]() {
                cds::Stack<int> s;
                for (int i = 0; i < N; ++i) s.push(i);
                while (!s.empty()) s.pop();
            }, 20),
            benchmark_ms([&]() {
                std::stack<int> s;
                for (int i = 0; i < N; ++i) s.push(i);
                while (!s.empty()) s.pop();
            }, 20)
        );
    }

    {
        cds::Stack<int> cds_stack;
        std::stack<int> std_stack;
        for (int i = 0; i < N; ++i) { cds_stack.push(i); std_stack.push(i); }

        print_result("Stack: top() access (100k)", 
            benchmark_ms([&]() {
                volatile int sum = 0;
                for (int i = 0; i < N; ++i) sum += cds_stack.top();
            }, 1000),
            benchmark_ms([&]() {
                volatile int sum = 0;
                for (int i = 0; i < N; ++i) sum += std_stack.top();
            }, 1000)
        );
    }

    // Queue vs std::queue
    {
        print_result("Queue (Ring)/std::queue: push/pop (100k)", 
            benchmark_ms([&]() {
                cds::Queue<int> q;
                for (int i = 0; i < N; ++i) q.push(i);
                while (!q.empty()) q.pop();
            }, 20),
            benchmark_ms([&]() {
                std::queue<int> q;
                for (int i = 0; i < N; ++i) q.push(i);
                while (!q.empty()) q.pop();
            }, 20)
        );
    }

    {
        print_result("Queue (Ring)/std::queue<deque>: push/pop (100k)", 
            benchmark_ms([&]() {
                cds::Queue<int> q;
                for (int i = 0; i < N; ++i) q.push(i);
                while (!q.empty()) q.pop();
            }, 20),
            benchmark_ms([&]() {
                std::queue<int, std::deque<int>> q;
                for (int i = 0; i < N; ++i) q.push(i);
                while (!q.empty()) q.pop();
            }, 20)
        );
    }

    {
        cds::Queue<int> cds_q;
        std::queue<int> std_q;
        for (int i = 0; i < N; ++i) { cds_q.push(i); std_q.push(i); }

        print_result("Queue: front() access (100k)", 
            benchmark_ms([&]() {
                volatile int sum = 0;
                for (int i = 0; i < N; ++i) sum += cds_q.front();
            }, 1000),
            benchmark_ms([&]() {
                volatile int sum = 0;
                for (int i = 0; i < N; ++i) sum += std_q.front();
            }, 1000)
        );
    }

    // Mixed workload
    {
        print_result("Mixed: push/pop/access pattern (50k)", 
            benchmark_ms([&]() {
                cds::Queue<int> q;
                for (int i = 0; i < 50000; ++i) {
                    q.push(i);
                    if (i % 3 == 0 && !q.empty()) q.pop();
                }
            }, 50),
            benchmark_ms([&]() {
                std::queue<int> q;
                for (int i = 0; i < 50000; ++i) {
                    q.push(i);
                    if (i % 3 == 0 && !q.empty()) q.pop();
                }
            }, 50)
        );
    }

    std::cout << "\n=== Memory Overhead Test ===\n";
    {
        cds::DynamicArray<int> cds_arr;
        std::vector<int> std_vec;
        for (int i = 0; i < 1000; ++i) { cds_arr.push_back(i); std_vec.push_back(i); }
        std::cout << "DynamicArray capacity: " << cds_arr.capacity() << ", size: " << cds_arr.size() << std::endl;
        std::cout << "std::vector capacity: " << std_vec.capacity() << ", size: " << std_vec.size() << std::endl;
    }

    {
        cds::Queue<int> cds_q;
        std::queue<int> std_q;
        for (int i = 0; i < 1000; ++i) { cds_q.push(i); std_q.push(i); }
        std::cout << "Queue capacity: " << cds_q.capacity() << ", size: " << cds_q.size() << std::endl;
        std::cout << "std::queue<deque> size: " << std_q.size() << std::endl;
    }

    return 0;
}