# Custom Data Structures Library (C++20)

A production-grade, header-only C++20 library implementing fundamental data structures from scratch with custom memory management, strong exception safety, full iterator support, comprehensive unit testing, and comparative benchmarks against the C++ Standard Template Library (STL).

## Features

- **Zero-overhead abstractions**: Header-only, no runtime dependencies
- **Custom memory management**: Placement `new`, explicit destruction, no default construction of unused elements
- **Rule of 5 compliant**: Proper copy/move constructors, assignments, and destructors with `noexcept` specifications
- **Strong exception safety**: All mutations provide strong or basic guarantees; no leaks on exceptions
- **STL-compliant iterators**: Compatible with range-based `for`, `<algorithm>`, and `<ranges>`
- **Sanitizer verified**: AddressSanitizer, LeakSanitizer, UndefinedBehaviorSanitizer clean
- **Benchmark suite**: Nanosecond-precision comparisons vs `std::vector`, `std::list`, `std::stack`, `std::queue`

## Data Structures

| Container | Backing | Iterator Category | Key Characteristics |
|-----------|---------|-------------------|---------------------|
| `cds::DynamicArray<T>` | Contiguous raw buffer | Random Access | 1.5x growth, `emplace_back`, strong guarantee |
| `cds::LinkedList<T>` | Doubly-linked with sentinel | Bidirectional | O(1) insert/erase at iterator, no null checks |
| `cds::Stack<T, Container>` | Adapter (default: DynamicArray) | N/A (adapter) | LIFO, `emplace`, configurable container |
| `cds::Queue<T>` | Circular ring buffer | Random Access | Dynamic growth, cache-friendly, O(1) amortized |

## Quick Start

### Requirements

- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 19.28+)
- CMake 3.20+

### Building

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build

# Run tests
cd build && ctest --output-on-failure

# Run benchmarks
./build/benchmarks/cds_benchmarks
```

### Sanitizer Build (Debug)

```bash
cmake -B build_debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build_debug
cd build_debug && ctest --output-on-failure
```

## Usage Examples

### DynamicArray (Vector Replacement)

```cpp
#include <cds/dynamic_array.hpp>

cds::DynamicArray<int> arr;
arr.push_back(1);
arr.emplace_back(2, 3); // Not valid for int, but works for aggregates

// Random access
arr[0] = 42;
arr.at(1); // bounds checked

// STL algorithms
std::sort(arr.begin(), arr.end());
for (int v : arr) { /* ... */ }

// Exception safety: strong guarantee on reallocation
```

### LinkedList

```cpp
#include <cds/linked_list.hpp>

cds::LinkedList<std::string> list;
list.push_front("first");
list.push_back("last");
list.emplace(list.begin(), "middle");

// O(1) insertion/erasure at iterator
auto it = list.insert(list.begin(), "new_front");
list.erase(it);

// Bidirectional iteration
for (auto& s : list) { /* ... */ }
```

### Stack

```cpp
#include <cds/stack.hpp>

// Default: backed by DynamicArray
cds::Stack<int> stack;
stack.push(1);
stack.emplace(2);
int top = stack.top();
stack.pop();

// Alternative: NodeStack (LinkedList-backed)
cds::NodeStack<int> node_stack;
```

### Queue (Ring Buffer)

```cpp
#include <cds/queue.hpp>

cds::Queue<int> q;
q.push(1);
q.emplace(2);
int front = q.front();
q.pop();

// Iteration in FIFO order
for (int v : q) { /* ... */ }

// Random access iterators
auto it = q.begin();
it += 5;
```

## Complexity

| Structure | Access | Insert Head | Insert Tail | Insert Middle | Delete Head | Delete Tail | Space |
|-----------|--------|-------------|-------------|---------------|-------------|-------------|-------|
| DynamicArray | O(1) | O(N) | O(1)* | O(N) | O(N) | O(1) | O(N) |
| LinkedList | O(N) | O(1) | O(1) | O(1)** | O(1) | O(1) | O(N) |
| Stack | O(1)*** | N/A | O(1)* | N/A | N/A | O(1) | O(N) |
| Queue (Ring) | O(1)*** | N/A | O(1)* | N/A | O(1) | N/A | O(N) |

* Amortized
** Given iterator
*** Top/Front/Back only

## Documentation

- [Time & Space Complexity](docs/time_space_complexity.md) - Formal Big-O proofs & tables
- [Architecture Notes](docs/architecture_notes.md) - Memory model, exception guarantees, iterator design

## Testing

The test suite covers:
- Basic operations (construction, access, mutation)
- Iterator compliance (forward, backward, random access, STL algorithms)
- Exception safety (strong guarantee with throwing types)
- Edge cases (empty, single element, self-assignment, move-only types)
- Memory safety (ASan/Valgrind verified)

```bash
# Run all tests
ctest --output-on-failure

# Run specific test
./build/tests/cds_tests
```

## Benchmarks

Comparative benchmarks against STL containers:

```bash
./build/benchmarks/cds_benchmarks
```

Typical results (Release, x86-64):

| Operation | CDS vs STL |
|-----------|------------|
| DynamicArray push_back | ~1.0-1.2x |
| DynamicArray iteration | ~1.0x |
| DynamicArray sort | ~1.0x |
| LinkedList push_back | ~0.8-1.0x |
| Queue push/pop (ring vs deque) | **1.5-3.0x faster** |
| Stack push/pop | ~1.0x |

The ring buffer queue significantly outperforms `std::queue<std::deque>` due to contiguous storage and zero per-element allocations.

## Project Structure

```
custom-ds-cpp/
├── CMakeLists.txt
├── include/
│   └── cds/
│       ├── dynamic_array.hpp
│       ├── linked_list.hpp
│       ├── stack.hpp
│       ├── queue.hpp
│       └── utils/
│           ├── memory_traits.hpp
│           └── iterator_base.hpp
├── tests/
│   ├── CMakeLists.txt
│   ├── test_main.cpp
│   ├── test_framework.hpp
│   ├── test_dynamic_array.cpp
│   ├── test_linked_list.cpp
│   ├── test_stack.cpp
│   └── test_queue.cpp
├── benchmarks/
│   ├── CMakeLists.txt
│   └── benchmark_comparison.cpp
├── docs/
│   ├── time_space_complexity.md
│   └── architecture_notes.md
└── README.md
```

## Design Principles

1. **No default construction of unused memory** - Uses raw allocation + placement new
2. **Strong exception safety** - Copy-and-swap, commit-or-rollback patterns
3. **Rule of 5** - All special member functions implemented correctly
4. **STL compatibility** - Standard iterator traits, algorithms, range-based for
5. **Zero dependencies** - Header-only, standard library only
6. **Modern C++** - C++20 concepts, `noexcept`, `std::move_if_noexcept`

## License

MIT License - See LICENSE file for details.