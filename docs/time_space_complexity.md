# Time & Space Complexity Analysis

## DynamicArray<T>

### Time Complexity

| Operation | Complexity | Notes |
|-----------|------------|-------|
| `operator[]` | O(1) | No bounds checking |
| `at()` | O(1) | Bounds checking with exception |
| `front()` / `back()` | O(1) | Direct access |
| `push_back()` | O(1) amortized | Geometric growth (1.5x) |
| `pop_back()` | O(1) | No shrinking |
| `insert()` / `erase()` | O(N) | Element shifting required |
| `resize()` | O(N) | Construction/destruction of elements |
| `reserve()` | O(N) | Only when capacity increases |
| `shrink_to_fit()` | O(N) | Reallocation and move |
| `clear()` | O(N) | Destruction of all elements |
| Iterator operations | O(1) | Random access iterator |

### Space Complexity

| Aspect | Complexity |
|--------|------------|
| Storage | O(N) where N = size |
| Capacity overhead | Up to 50% (growth factor 1.5x) |
| Per-element overhead | 0 bytes (contiguous storage) |

### Amortized Analysis of push_back

With growth factor α = 1.5:
- After k reallocations, capacity = α^k
- Total elements copied = Σ α^i for i=0 to k-1 = (α^k - 1)/(α - 1)
- Total cost = N + (αN - 1)/(α - 1) = O(N)
- Amortized cost per insertion = O(1)

## LinkedList<T>

### Time Complexity

| Operation | Complexity | Notes |
|-----------|------------|-------|
| `front()` / `back()` | O(1) | Sentinel node access |
| `push_front()` / `push_back()` | O(1) | Single allocation + link |
| `pop_front()` / `pop_back()` | O(1) | Unlink + deallocation |
| `insert(pos, val)` | O(1) | Given valid iterator |
| `erase(pos)` | O(1) | Given valid iterator |
| `find()` / `operator[]` | O(N) | Sequential traversal |
| Iterator `++` / `--` | O(1) | Pointer manipulation |
| `splice()` / `merge()` | O(1) | Not implemented |

### Space Complexity

| Aspect | Complexity |
|--------|------------|
| Storage | O(N) |
| Per-node overhead | 2 pointers (16 bytes on 64-bit) |
| Sentinel node | 1 node (fixed overhead) |

## Stack<T>

### Time Complexity

| Operation | Complexity | Notes |
|-----------|------------|-------|
| `push()` | O(1) amortized | Delegates to container |
| `pop()` | O(1) | Delegates to container |
| `top()` | O(1) | Delegates to container |
| `empty()` / `size()` | O(1) | Delegates to container |

### Space Complexity

Same as underlying container (DynamicArray or LinkedList)

## Queue<T> (Ring Buffer)

### Time Complexity

| Operation | Complexity | Notes |
|-----------|------------|-------|
| `push()` / `emplace()` | O(1) amortized | Geometric growth (2x) |
| `pop()` | O(1) | Head index increment |
| `front()` / `back()` | O(1) | Direct index calculation |
| `empty()` / `size()` | O(1) | Maintained counter |
| Iterator operations | O(1) | Random access via modulo |

### Space Complexity

| Aspect | Complexity |
|--------|------------|
| Storage | O(N) where N = capacity |
| Capacity overhead | Up to 100% (growth factor 2x) |
| Per-element overhead | 0 bytes (contiguous storage) |

### Amortized Analysis of push

With growth factor 2:
- Same analysis as DynamicArray, amortized O(1)

## Comparison Summary

| Structure | Access | Insert Head | Insert Tail | Insert Middle | Delete Head | Delete Tail | Space |
|-----------|--------|-------------|-------------|---------------|-------------|-------------|-------|
| DynamicArray | O(1) | O(N) | O(1)* | O(N) | O(N) | O(1) | O(N) |
| LinkedList | O(N) | O(1) | O(1) | O(1)** | O(1) | O(1) | O(N) |
| Stack | O(1)*** | N/A | O(1)* | N/A | N/A | O(1) | O(N) |
| Queue (Ring) | O(1)*** | N/A | O(1)* | N/A | O(1) | N/A | O(N) |

* Amortized
** Given iterator position
*** Top/Front/Back only

## Exception Safety Guarantees

| Operation | Guarantee |
|-----------|-----------|
| Default construction | No-throw |
| Copy construction | Strong (all-or-nothing) |
| Move construction | No-throw |
| Copy assignment | Strong (copy-and-swap) |
| Move assignment | No-throw |
| `push_back` / `push` / `emplace` | Strong |
| `pop_back` / `pop` | No-throw |
| `insert` / `erase` | Strong (LinkedList), Basic (DynamicArray) |
| `reserve` / `reallocate` | Strong |
| `resize` | Strong |
| `clear` | No-throw |
| `swap` | No-throw |

## Iterator Invalidation

### DynamicArray
- Reallocation: All iterators invalidated
- Insert/Erase: Iterators at/after position invalidated
- Reserve/Shrink: If reallocation occurs, all invalidated

### LinkedList
- Insert: No invalidation
- Erase: Only erased iterator invalidated
- Clear: All iterators invalidated

### Queue (Ring Buffer)
- Reallocation: All iterators invalidated
- Push/Pop: No invalidation (indices remain valid)