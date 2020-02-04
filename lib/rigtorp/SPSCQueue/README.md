# SPSCQueue.h

[![Build Status](https://travis-ci.org/rigtorp/SPSCQueue.svg?branch=master)](https://travis-ci.org/rigtorp/SPSCQueue)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/rigtorp/SPSCQueue/master/LICENSE)

A single producer single consumer wait-free and lock-free fixed size
queue written in C++11.

## Example

```cpp
SPSCQueue<int> q(2);
auto t = std::thread([&] {
  while (!q.front());
  std::cout << *q.front() << std::endl;
  q.pop();
});
q.push(1);
t.join();
```

## Usage

- `SPSCQueue<T>(size_t capacity);`

  Create a `SPSCqueue` holding items of type `T` with capacity
  `capacity`. Capacity need to be greater than 2.

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
  
- `template <typename P> void try_push(P &&v);`

  Try to enqueue an item using move construction. Returns `true` on
  success and `false` if queue is full. Participates in overload
  resolution only if `std::is_constructible<T, P&&>::value == true`.

- `T *front();`

  Return pointer to front of queue. Returns `nullptr` if queue is
  empty.

- `pop();`

  Dequeue first elment of queue. Invalid to call if queue is
  empty. Requires `std::is_nothrow_destructible<T>::value == true`.

Only a single writer thread can perform enqueue operations and only a
single reader thread can perform dequeue operations. Any other usage
is invalid.

## Implementation

![Memory layout](https://github.com/rigtorp/SPSCQueue/blob/master/spsc.png)

The underlying implementation is a
[ring buffer](https://en.wikipedia.org/wiki/Circular_buffer). 

Care has been taken to make sure to avoid any issues with
[false sharing](https://en.wikipedia.org/wiki/False_sharing). The head
and tail pointers are aligned and padded to the false sharing range
(currently hard coded to 128 bytes). The slots buffer is padded with
the false sharing range at the beginning and end.

References:

- *Intel*. [Avoiding and Identifying False Sharing Among Threads](https://software.intel.com/en-us/articles/avoiding-and-identifying-false-sharing-among-threads).
- *Wikipedia*. [Ring buffer](https://en.wikipedia.org/wiki/Circular_buffer).
- *Wikipedia*. [False sharing](https://en.wikipedia.org/wiki/False_sharing).

## Testing

Testing lock-free algorithms is hard. I'm using two approaches to test
the implementation:

- A single threaded test that the functionality works as intended,
  including that the element constructor and destructor is invoked
  correctly.
- A multithreaded fuzz test that all elements are enqueued and
  dequeued correctly under heavy contention.

## Benchmarks

Throughput benchmark measures throughput between 2 threads for a
`SPSCQueue<int>` of size 256.

Latency benchmark measures round trip time between 2 threads
communicating using 2 queues of type `SPSCQueue<int>`.

The following numbers are for a 2 socket machine with 2 x Intel(R)
Xeon(R) CPU E5-2620 0 @ 2.00GHz.
 
| NUMA Node / Core / Hyper-Thread | Throughput (ops/ms) | Latency RTT (ns) |
| ------------------------------- | -------------------:| ----------------:|
| #0,#0,#0 & #0,#0,#1             |               63942 |               60 |
| #0,#0,#0 & #0,#1,#0             |               37739 |              238 |
| #0,#0,#0 & #1,#0,#0             |               25744 |              768 |

## Cited by

SPSCQueue have been cited by the following papers:
- Peizhao Ou and Brian Demsky. 2018. Towards understanding the costs of avoiding
  out-of-thin-air results. Proc. ACM Program. Lang. 2, OOPSLA, Article 136
  (October 2018), 29 pages. DOI: https://doi.org/10.1145/3276506 

## About

This project was created by [Erik Rigtorp](http://rigtorp.se)
<[erik@rigtorp.se](mailto:erik@rigtorp.se)>.
