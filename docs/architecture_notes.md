# Architecture Notes

## Memory Model

### Raw Memory Management

All containers use `cds::detail::MemoryTraits<T>` for raw memory operations:

```cpp
// Allocation (uninitialized)
pointer allocate(size_type n);

// Deallocation
void deallocate(pointer p, size_type n) noexcept;

// Placement new construction
template <typename... Args>
void construct_at(pointer p, Args&&... args);

// Explicit destructor call
void destroy_at(pointer p) noexcept;

// Range destruction
template <typename U>
void destroy_range(pointer first, pointer last) noexcept;

// Exception-safe copy
template <typename U>
void uninitialized_copy(pointer first, pointer last, pointer d_first);

// Exception-safe move
template <typename U>
void uninitialized_move(pointer first, pointer last, pointer d_first);
```

### Why Not `new T[n]`?

Using `new T[n]` forces default construction of all `n` elements, which:
1. Requires `T` to be default constructible
2. Wastes cycles constructing elements that will be overwritten
3. Leaks if construction of element `k` throws (elements `0..k-1` already constructed)

Our approach:
- Allocates raw bytes via `::operator new`
- Constructs elements individually via placement `new`
- On exception, destroys only constructed elements before deallocating

## Exception Safety

### Strong Guarantee Implementation

**DynamicArray::push_back** (and similar mutation operations):

```cpp
void push_back(const T& value) {
    if (size_ == capacity_) {
        reserve(calculate_growth(size_ + 1)); // Strong guarantee
    }
    Traits::construct_at(data_ + size_, value); // May throw
    ++size_; // No-throw
}
```

**Reallocation with strong guarantee:**

```cpp
void reallocate(size_type new_cap) {
    pointer new_data = Traits::allocate(new_cap); // May throw (bad_alloc)
    try {
        Traits::uninitialized_move(data_, data_ + size_, new_data); // May throw
    } catch (...) {
        Traits::deallocate(new_data, new_cap); // Cleanup on failure
        throw;
    }
    Traits::destroy_range(data_, data_ + size_); // No-throw
    Traits::deallocate(data_, capacity_); // No-throw
    data_ = new_data;
    capacity_ = new_cap;
}
```

**Copy-and-swap for assignment:**

```cpp
DynamicArray& operator=(const DynamicArray& other) {
    if (this != &other) {
        DynamicArray tmp(other); // Strong guarantee
        swap(tmp); // No-throw
    }
    return *this;
}
```

### No-Throw Guarantees

Operations marked `noexcept`:
- Destructor
- Move constructor/assignment
- `swap()`
- `pop_back()`, `pop_front()`, `pop_back()`, `pop()`
- `clear()`
- Iterator operations
- `empty()`, `size()`, `capacity()`
- `front()`, `back()`, `operator[]`

### Basic Guarantee

Operations that may leave container in valid but unspecified state:
- `insert()`/`erase()` in DynamicArray (on element copy/move throw during shift)

## Iterator Design

### Tag Dispatching

Iterators inherit from facade classes in `iterator_base.hpp`:

```cpp
// Random Access (DynamicArray, Queue)
class Iterator : public RandomAccessIteratorFacade<...> { ... };

// Bidirectional (LinkedList)
class Iterator : public BidirectionalIteratorFacade<...> { ... };
```

Facades provide:
- Operator implementations (`*`, `->`, `++`, `--`, `+`, `-`, `[]`, comparisons)
- CRTP pattern for zero-overhead abstraction
- Standard iterator traits (`iterator_category`, `value_type`, etc.)

### STL Compliance

All iterators satisfy:
- `std::iterator_traits<It>::iterator_category` correct
- `std::iterator_traits<It>::value_type` = `T`
- `std::iterator_traits<It>::reference` = `T&` or `const T&`
- `std::iterator_traits<It>::pointer` = `T*` or `const T*`
- `std::iterator_traits<It>::difference_type` = `ptrdiff_t`

Compatible with:
- Range-based for loops
- `std::algorithm` functions
- `std::ranges` (C++20)

## Sentinel Node Pattern (LinkedList)

```
sentinel <-> node1 <-> node2 <-> ... <-> nodeN <-> sentinel
   ^                                                          |
   +----------------------------------------------------------+
```

Benefits:
- Eliminates null checks for head/tail
- `begin()` = `sentinel.next`, `end()` = `&sentinel`
- Insert at end = insert before sentinel
- Empty list: `sentinel.next == &sentinel && sentinel.prev == &sentinel`
- Single allocation for empty list (sentinel embedded in list object)

## Ring Buffer Design (Queue)

### Layout

```
[empty] [data] [data] [data] [empty] [empty]
  ^                          ^
 head                      tail
```

### Index Arithmetic

```cpp
size_type next_index(size_type idx) const noexcept {
    return (idx + 1) % capacity_;
}

size_type prev_index(size_type idx) const noexcept {
    return (idx == 0) ? capacity_ - 1 : idx - 1;
}
```

### Growth Strategy

When `tail == head` and buffer not empty:
1. Allocate new buffer (2x capacity)
2. Move elements in order: `[head, capacity)` then `[0, tail)`
3. Reset `head = 0`, `tail = size`

This unwrapping preserves element order and enables O(1) random access.

## Rule of 5 Compliance

All containers implement:

```cpp
// Destructor
~Container() { clear(); deallocate(); }

// Copy constructor
Container(const Container& other) { allocate(); uninitialized_copy(); }

// Move constructor
Container(Container&& other) noexcept : data_(other.data_) { other.data_ = nullptr; }

// Copy assignment
Container& operator=(const Container& other) {
    if (this != &other) {
        Container tmp(other);
        swap(tmp);
    }
    return *this;
}

// Move assignment
Container& operator=(Container&& other) noexcept {
    if (this != &other) {
        clear(); deallocate();
        data_ = other.data_; other.data_ = nullptr;
    }
    return *this;
}
```

## Testing Strategy

### Test Categories

1. **Basic Operations**: Construction, size, empty, access
2. **Mutations**: Push/pop, insert/erase, resize
3. **Iterators**: Forward, backward, random access, STL algorithms
4. **Memory**: No leaks (ASan), proper destruction
5. **Exceptions**: Strong guarantee verification with throwing types
6. **Edge Cases**: Empty container, single element, self-assignment
7. **Type Requirements**: Non-default-constructible, move-only types

### Throwing Type Testing

```cpp
struct ThrowOnCopy {
    static bool should_throw;
    ThrowOnCopy(const ThrowOnCopy&) { if (should_throw) throw; }
};
```

Tests verify:
- No memory leaks when copy throws during reallocation
- Container remains in valid state
- No elements leaked

### Sanitizer Integration

CMake enables:
- AddressSanitizer: `-fsanitize=address`
- UndefinedBehaviorSanitizer: `-fsanitize=undefined`
- LeakSanitizer: Part of ASan
- MSVC: `/fsanitize=address`

## Build Configuration

### CMake Minimum Version

3.20 (for `target_compile_features`, `CMAKE_CXX_STANDARD`)

### Compiler Flags

**GCC/Clang:**
```
-Wall -Wextra -Wpedantic -Wconversion -Wshadow
-fsanitize=address,undefined (Debug)
-O3 -march=native (Release)
```

**MSVC:**
```
/W4 /permissive- /std:c++20
/fsanitize=address (Debug)
/O2 (Release)
```

### C++20 Features Used

- Concepts (in Stack comparison operators)
- `std::destroy_at`, `std::construct_at` (C++20)
- `std::is_nothrow_swappable_v`
- Structured bindings (in tests)
- `std::format` (not used, for compatibility)

## Performance Considerations

### Cache Locality

- DynamicArray: Contiguous storage, optimal cache usage
- Queue (Ring): Contiguous storage, better than node-based queue
- LinkedList: Poor cache locality (node allocation scattered)

### Allocation Overhead

- DynamicArray: O(1) amortized per element
- LinkedList: O(1) per element but with allocator overhead
- Queue: O(1) amortized, fewer allocations than node-based queue

### Move Optimization

Uses `std::move_if_noexcept` during reallocation to prefer moves over copies when noexcept.

## Future Improvements

1. **Small Buffer Optimization**: Store small arrays inline
2. **Custom Allocator Support**: Template parameter for allocator
3. **Parallel Algorithms**: Execution policy support
4. **Views**: Non-owning range adapters
5. **Persistent Data Structures**: Immutable variants