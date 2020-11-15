/*
Copyright (c) 2020 Erik Rigtorp <erik@rigtorp.se>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#pragma once

#include <atomic>
#include <cassert>
#include <cstddef>
#include <memory> // std::allocator
#include <new>    // std::hardware_destructive_interference_size
#include <stdexcept>
#include <type_traits> // std::enable_if, std::is_*_constructible

namespace rigtorp {

template <typename T, typename Allocator = std::allocator<T>> class SPSCQueue {

#if defined(__cpp_if_constexpr) && defined(__cpp_lib_void_t)
  template <typename Alloc2, typename = void>
  struct has_allocate_at_least : std::false_type {};

  template <typename Alloc2>
  struct has_allocate_at_least<
      Alloc2, std::void_t<typename Alloc2::value_type,
                          decltype(std::declval<Alloc2 &>().allocate_at_least(
                              size_t{}))>> : std::true_type {};
#endif

public:
  explicit SPSCQueue(const size_t capacity,
                     const Allocator &allocator = Allocator())
      : capacity_(capacity), allocator_(allocator), head_(0), tail_(0) {
    // The queue needs at least one element
    if (capacity_ < 1) {
      capacity_ = 1;
    }
    capacity_++; // Needs one slack element
    // Prevent overflowing size_t
    if (capacity_ > SIZE_MAX - 2 * kPadding) {
      capacity_ = SIZE_MAX - 2 * kPadding;
    }

#if defined(__cpp_if_constexpr) && defined(__cpp_lib_void_t)
    if constexpr (has_allocate_at_least<Allocator>::value) {
      auto res = allocator_.allocate_at_least(capacity_ + 2 * kPadding);
      slots_ = res.ptr;
      capacity_ = res.count - 2 * kPadding;
    } else {
      slots_ = std::allocator_traits<Allocator>::allocate(
          allocator_, capacity_ + 2 * kPadding);
    }
#else
    slots_ = std::allocator_traits<Allocator>::allocate(
        allocator_, capacity_ + 2 * kPadding);
#endif

    static_assert(alignof(SPSCQueue<T>) == kCacheLineSize, "");
    static_assert(sizeof(SPSCQueue<T>) >= 3 * kCacheLineSize, "");
    assert(reinterpret_cast<char *>(&tail_) -
               reinterpret_cast<char *>(&head_) >=
           static_cast<std::ptrdiff_t>(kCacheLineSize));
  }

  ~SPSCQueue() {
    while (front()) {
      pop();
    }
    std::allocator_traits<Allocator>::deallocate(allocator_, slots_,
                                                 capacity_ + 2 * kPadding);
  }

  // non-copyable and non-movable
  SPSCQueue(const SPSCQueue &) = delete;
  SPSCQueue &operator=(const SPSCQueue &) = delete;

  template <typename... Args>
  void emplace(Args &&... args) noexcept(
      std::is_nothrow_constructible<T, Args &&...>::value) {
    static_assert(std::is_constructible<T, Args &&...>::value,
                  "T must be constructible with Args&&...");
    auto const head = head_.load(std::memory_order_relaxed);
    auto nextHead = head + 1;
    if (nextHead == capacity_) {
      nextHead = 0;
    }
    while (nextHead == tail_.load(std::memory_order_acquire))
      ;
    new (&slots_[head + kPadding]) T(std::forward<Args>(args)...);
    head_.store(nextHead, std::memory_order_release);
  }

  template <typename... Args>
  bool try_emplace(Args &&... args) noexcept(
      std::is_nothrow_constructible<T, Args &&...>::value) {
    static_assert(std::is_constructible<T, Args &&...>::value,
                  "T must be constructible with Args&&...");
    auto const head = head_.load(std::memory_order_relaxed);
    auto nextHead = head + 1;
    if (nextHead == capacity_) {
      nextHead = 0;
    }
    if (nextHead == tail_.load(std::memory_order_acquire)) {
      return false;
    }
    new (&slots_[head + kPadding]) T(std::forward<Args>(args)...);
    head_.store(nextHead, std::memory_order_release);
    return true;
  }

  void push(const T &v) noexcept(std::is_nothrow_copy_constructible<T>::value) {
    static_assert(std::is_copy_constructible<T>::value,
                  "T must be copy constructible");
    emplace(v);
  }

  template <typename P, typename = typename std::enable_if<
                            std::is_constructible<T, P &&>::value>::type>
  void push(P &&v) noexcept(std::is_nothrow_constructible<T, P &&>::value) {
    emplace(std::forward<P>(v));
  }

  bool
  try_push(const T &v) noexcept(std::is_nothrow_copy_constructible<T>::value) {
    static_assert(std::is_copy_constructible<T>::value,
                  "T must be copy constructible");
    return try_emplace(v);
  }

  template <typename P, typename = typename std::enable_if<
                            std::is_constructible<T, P &&>::value>::type>
  bool try_push(P &&v) noexcept(std::is_nothrow_constructible<T, P &&>::value) {
    return try_emplace(std::forward<P>(v));
  }

  T *front() noexcept {
    auto const tail = tail_.load(std::memory_order_relaxed);
    if (head_.load(std::memory_order_acquire) == tail) {
      return nullptr;
    }
    return &slots_[tail + kPadding];
  }

  void pop() noexcept {
    static_assert(std::is_nothrow_destructible<T>::value,
                  "T must be nothrow destructible");
    auto const tail = tail_.load(std::memory_order_relaxed);
    assert(head_.load(std::memory_order_acquire) != tail);
    slots_[tail + kPadding].~T();
    auto nextTail = tail + 1;
    if (nextTail == capacity_) {
      nextTail = 0;
    }
    tail_.store(nextTail, std::memory_order_release);
  }

  size_t size() const noexcept {
    std::ptrdiff_t diff = head_.load(std::memory_order_acquire) -
                          tail_.load(std::memory_order_acquire);
    if (diff < 0) {
      diff += capacity_;
    }
    return static_cast<size_t>(diff);
  }

  bool empty() const noexcept { return size() == 0; }

  size_t capacity() const noexcept { return capacity_ - 1; }

private:
// on macOS there is a bug in libc++ where __cpp_lib_hardware_interference_size
// is defined but std::hardware_destructive_interference_size is not actually implemented
// https://bugs.llvm.org/show_bug.cgi?id=41423
#if defined(__cpp_lib_hardware_interference_size) && ! defined(__APPLE__)
  static constexpr size_t kCacheLineSize =
      std::hardware_destructive_interference_size;
#else
  static constexpr size_t kCacheLineSize = 64;
#endif

  // Padding to avoid false sharing between slots_ and adjacent allocations
  static constexpr size_t kPadding = (kCacheLineSize - 1) / sizeof(T) + 1;

private:
  size_t capacity_;
  T *slots_;
#if defined(__has_cpp_attribute) && __has_cpp_attribute(no_unique_address)
  Allocator allocator_ [[no_unique_address]];
#else
  Allocator allocator_;
#endif

  // Align to avoid false sharing between head_ and tail_
  alignas(kCacheLineSize) std::atomic<size_t> head_;
  alignas(kCacheLineSize) std::atomic<size_t> tail_;

  // Padding to avoid adjacent allocations to share cache line with tail_
  char padding_[kCacheLineSize - sizeof(tail_)];
};
} // namespace rigtorp
