# SPSCQueue.h

[![C/C++ CI](https://github.com/rigtorp/SPSCQueue/workflows/C/C++%20CI/badge.svg)](https://github.com/rigtorp/SPSCQueue/actions)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/rigtorp/SPSCQueue/master/LICENSE)

A single producer single consumer wait-free and lock-free fixed size queue
written in C++11. This implementation is faster than both
[*boost::lockfree::spsc*](https://www.boost.org/doc/libs/1_76_0/doc/html/boost/lockfree/spsc_queue.html)
and [*folly::ProducerConsumerQueue*](https://github.com/facebook/folly/blob/master/folly/docs/ProducerConsumerQueue.md).

## Example

```cpp
SPSCQueue<int> q(1);
auto t = std::thread([&] {
  while (!q.front());
  std::cout << *q.front() << std::endl;
  q.pop();
});
q.push(1);
t.join();
```

See `src/SPSCQueueExample.cpp` for the full example.

## Usage

- `SPSCQueue<T>(size_t capacity);`

  Create a `SPSCqueue` holding items of type `T` with capacity
  `capacity`. Capacity needs to be at least 1.

- `void emplace(Args &&... args);`

  Enqueue an item using inplace construction. Blocks if queue is full.

- `bool try_emplace(Args &&... args);`

  Try to enqueue an item using inplace construction. Returns `true` on
  success and `false` if queue is full.

- `void push(const T &v);`

  Enqueue an item using copy construction. Blocks if queue is full.

- `template <typename P> void push(P &&v);`

  Enqueue an item using move construction. Participates in overload
  resolution only if `std::is_constructible<T, P&&>::value == true`.
  Blocks if queue is full.

- `bool try_push(const T &v);`

  Try to enqueue an item using copy construction. Returns `true` on
  success and `false` if queue is full.

- `template <typename P> bool try_push(P &&v);`

  Try to enqueue an item using move construction. Returns `true` on
  success and `false` if queue is full. Participates in overload
  resolution only if `std::is_constructible<T, P&&>::value == true`.

- `T *front();`

  Return pointer to front of queue. Returns `nullptr` if queue is
  empty.

- `void pop();`

  Dequeue first item of queue. Invalid to call if queue is
  empty. Requires `std::is_nothrow_destructible<T>::value == true`.

- `size_t size();`

  Return the number of items available in the queue.

- `bool empty();`

  Return true if queue is currently empty.

Only a single writer thread can perform enqueue operations and only a
single reader thread can perform dequeue operations. Any other usage
is invalid.

## Huge page support

In addition to supporting custom allocation through the [standard custom
allocator interface](https://en.cppreference.com/w/cpp/named_req/Allocator) this
library also supports standard proposal [P0401R3 Providing size feedback in the
Allocator
interface](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p0401r3.html).
This allows convenient use of [huge
pages](https://www.kernel.org/doc/html/latest/admin-guide/mm/hugetlbpage.html)
without wasting any allocated space. Using size feedback is only supported when
C++17 is enabled.

The library currently doesn't include a huge page allocator since the APIs for
allocating huge pages are platform dependent and handling of huge page size and
NUMA awareness is application specific.

Below is an example huge page allocator for Linux:

```cpp
#include <sys/mman.h>

template <typename T> struct Allocator {
  using value_type = T;

  struct AllocationResult {
    T *ptr;
    size_t count;
  };

  size_t roundup(size_t n) { return (((n - 1) >> 21) + 1) << 21; }

  AllocationResult allocate_at_least(size_t n) {
    size_t count = roundup(sizeof(T) * n);
    auto p = static_cast<T *>(mmap(nullptr, count, PROT_READ | PROT_WRITE,
                                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
                                   -1, 0));
    if (p == MAP_FAILED) {
      throw std::bad_alloc();
    }
    return {p, count / sizeof(T)};
  }

  void deallocate(T *p, size_t n) { munmap(p, roundup(sizeof(T) * n)); }
};
```

See `src/SPSCQueueExampleHugepages.cpp` for the full example on how to use huge
pages on Linux.

## Implementation

![Memory layout](https://github.com/rigtorp/SPSCQueue/blob/master/spsc.svg)

The underlying implementation is based on a [ring
buffer](https://en.wikipedia.org/wiki/Circular_buffer).

Care has been taken to make sure to avoid any issues with [false
sharing](https://en.wikipedia.org/wiki/False_sharing). The head and tail indices
are aligned and padded to the false sharing range (cache line size).
Additionally the slots buffer is padded with the false sharing range at the
beginning and end, this prevents false sharing with any adjacent allocations.

This implementation has higher throughput than a typical concurrent ring buffer
by locally caching the head and tail indices in the writer and reader
respectively. The caching increases throughput by reducing the amount of cache
coherency traffic.

To understand how that works first consider a read operation in absence of
caching: the head index (read index) needs to be updated and thus that cache
line is loaded into the L1 cache in exclusive state. The tail (write index)
needs to be read in order to check that the queue is not empty and is thus
loaded into the L1 cache in shared state. Since a queue write operation needs to
read the head index it's likely that a write operation requires some cache
coherency traffic to bring the head index cache line back into exclusive state.
In the worst case there will be one cache line transition from shared to
exclusive for every read and write operation.

Next consider a queue reader that caches the tail index: if the cached tail
index indicates that the queue is empty, then load the tail index into the
cached tail index. If the queue was non-empty multiple read operations up until
the cached tail index can complete without stealing the writer's tail index
cache line's exclusive state. Cache coherency traffic is therefore reduced. An
analogous argument can be made for the queue write operation.

This implementation allows for arbitrary non-power of two capacities, instead
allocating a extra queue slot to indicate full queue. If you don't want to waste
storage for a extra queue slot you should use a different implementation.

References:

- *Intel*. [Avoiding and Identifying False Sharing Among Threads](https://software.intel.com/en-us/articles/avoiding-and-identifying-false-sharing-among-threads).
- *Wikipedia*. [Ring buffer](https://en.wikipedia.org/wiki/Circular_buffer).
- *Wikipedia*. [False sharing](https://en.wikipedia.org/wiki/False_sharing).

## Testing

Testing lock-free algorithms is hard. I'm using two approaches to test
the implementation:

- A single threaded test that the functionality works as intended,
  including that the item constructor and destructor is invoked
  correctly.
- A multi-threaded fuzz test verifies that all items are enqueued and dequeued
  correctly under heavy contention.

## Benchmarks

Throughput benchmark measures throughput between 2 threads for a queue of `int`
items.

Latency benchmark measures round trip time between 2 threads communicating using
2 queues of `int` items.

Benchmark results for a AMD Ryzen 9 3900X 12-Core Processor, the 2 threads are
running on different cores on the same chiplet:

| Queue                        | Throughput (ops/ms) | Latency RTT (ns) |
| ---------------------------- | ------------------: | ---------------: |
| SPSCQueue                    |              362723 |              133 |
| boost::lockfree::spsc        |              209877 |              222 |
| folly::ProducerConsumerQueue |              148818 |              147 |

## Cited by

SPSCQueue have been cited by the following papers:

- Peizhao Ou and Brian Demsky. 2018. Towards understanding the costs of avoiding
  out-of-thin-air results. Proc. ACM Program. Lang. 2, OOPSLA, Article 136
  (October 2018), 29 pages. DOI: <https://doi.org/10.1145/3276506>

## About

This project was created by [Erik Rigtorp](http://rigtorp.se)
<[erik@rigtorp.se](mailto:erik@rigtorp.se)>.
