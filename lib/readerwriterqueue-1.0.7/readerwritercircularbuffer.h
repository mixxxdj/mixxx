// ©2020 Cameron Desrochers.
// Distributed under the simplified BSD license (see the license file that
// should have come with this header).

// Provides a C++11 implementation of a single-producer, single-consumer wait-free concurrent
// circular buffer (fixed-size queue).

#pragma once

#include <utility>
#include <chrono>
#include <memory>
#include <cstdlib>
#include <cstdint>
#include <cassert>

// Note that this implementation is fully modern C++11 (not compatible with old MSVC versions)
// but we still include atomicops.h for its LightweightSemaphore implementation.
#include "atomicops.h"

#ifndef MOODYCAMEL_CACHE_LINE_SIZE
#define MOODYCAMEL_CACHE_LINE_SIZE 64
#endif

namespace moodycamel {

template<typename T>
class BlockingReaderWriterCircularBuffer
{
public:
	typedef T value_type;

public:
	explicit BlockingReaderWriterCircularBuffer(std::size_t capacity)
		: maxcap(capacity), mask(), rawData(), data(),
		slots_(new spsc_sema::LightweightSemaphore(static_cast<spsc_sema::LightweightSemaphore::ssize_t>(capacity))),
		items(new spsc_sema::LightweightSemaphore(0)),
		nextSlot(0), nextItem(0)
	{
		// Round capacity up to power of two to compute modulo mask.
		// Adapted from http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
		--capacity;
		capacity |= capacity >> 1;
		capacity |= capacity >> 2;
		capacity |= capacity >> 4;
		for (std::size_t i = 1; i < sizeof(std::size_t); i <<= 1)
			capacity |= capacity >> (i << 3);
		mask = capacity++;
		rawData = static_cast<char*>(std::malloc(capacity * sizeof(T) + std::alignment_of<T>::value - 1));
		data = align_for<T>(rawData);
	}

	BlockingReaderWriterCircularBuffer(BlockingReaderWriterCircularBuffer&& other)
		: maxcap(0), mask(0), rawData(nullptr), data(nullptr),
		slots_(new spsc_sema::LightweightSemaphore(0)),
		items(new spsc_sema::LightweightSemaphore(0)),
		nextSlot(), nextItem()
	{
		swap(other);
	}

	BlockingReaderWriterCircularBuffer(BlockingReaderWriterCircularBuffer const&) = delete;

	// Note: The queue should not be accessed concurrently while it's
	// being deleted. It's up to the user to synchronize this.
	~BlockingReaderWriterCircularBuffer()
	{
		for (std::size_t i = 0, n = items->availableApprox(); i != n; ++i)
			reinterpret_cast<T*>(data)[(nextItem + i) & mask].~T();
		std::free(rawData);
	}

	BlockingReaderWriterCircularBuffer& operator=(BlockingReaderWriterCircularBuffer&& other) noexcept
	{
		swap(other);
		return *this;
	}

	BlockingReaderWriterCircularBuffer& operator=(BlockingReaderWriterCircularBuffer const&) = delete;

	// Swaps the contents of this buffer with the contents of another.
	// Not thread-safe.
	void swap(BlockingReaderWriterCircularBuffer& other) noexcept
	{
		std::swap(maxcap, other.maxcap);
		std::swap(mask, other.mask);
		std::swap(rawData, other.rawData);
		std::swap(data, other.data);
		std::swap(slots_, other.slots_);
		std::swap(items, other.items);
		std::swap(nextSlot, other.nextSlot);
		std::swap(nextItem, other.nextItem);
	}

	// Enqueues a single item (by copying it).
	// Fails if not enough room to enqueue.
	// Thread-safe when called by producer thread.
	// No exception guarantee (state will be corrupted) if constructor of T throws.
	bool try_enqueue(T const& item)
	{
		if (!slots_->tryWait())
			return false;
		inner_enqueue(item);
		return true;
	}

	// Enqueues a single item (by moving it, if possible).
	// Fails if not enough room to enqueue.
	// Thread-safe when called by producer thread.
	// No exception guarantee (state will be corrupted) if constructor of T throws.
	bool try_enqueue(T&& item)
	{
		if (!slots_->tryWait())
			return false;
		inner_enqueue(std::move(item));
		return true;
	}

	// Blocks the current thread until there's enough space to enqueue the given item,
	// then enqueues it (via copy).
	// Thread-safe when called by producer thread.
	// No exception guarantee (state will be corrupted) if constructor of T throws.
	void wait_enqueue(T const& item)
	{
		while (!slots_->wait());
		inner_enqueue(item);
	}

	// Blocks the current thread until there's enough space to enqueue the given item,
	// then enqueues it (via move, if possible).
	// Thread-safe when called by producer thread.
	// No exception guarantee (state will be corrupted) if constructor of T throws.
	void wait_enqueue(T&& item)
	{
		while (!slots_->wait());
		inner_enqueue(std::move(item));
	}

	// Blocks the current thread until there's enough space to enqueue the given item,
	// or the timeout expires. Returns false without enqueueing the item if the timeout
	// expires, otherwise enqueues the item (via copy) and returns true.
	// Thread-safe when called by producer thread.
	// No exception guarantee (state will be corrupted) if constructor of T throws.
	bool wait_enqueue_timed(T const& item, std::int64_t timeout_usecs)
	{
		if (!slots_->wait(timeout_usecs))
			return false;
		inner_enqueue(item);
		return true;
	}

	// Blocks the current thread until there's enough space to enqueue the given item,
	// or the timeout expires. Returns false without enqueueing the item if the timeout
	// expires, otherwise enqueues the item (via move, if possible) and returns true.
	// Thread-safe when called by producer thread.
	// No exception guarantee (state will be corrupted) if constructor of T throws.
	bool wait_enqueue_timed(T&& item, std::int64_t timeout_usecs)
	{
		if (!slots_->wait(timeout_usecs))
			return false;
		inner_enqueue(std::move(item));
		return true;
	}

	// Blocks the current thread until there's enough space to enqueue the given item,
	// or the timeout expires. Returns false without enqueueing the item if the timeout
	// expires, otherwise enqueues the item (via copy) and returns true.
	// Thread-safe when called by producer thread.
	// No exception guarantee (state will be corrupted) if constructor of T throws.
	template<typename Rep, typename Period>
	inline bool wait_enqueue_timed(T const& item, std::chrono::duration<Rep, Period> const& timeout)
	{
		return wait_enqueue_timed(item, std::chrono::duration_cast<std::chrono::microseconds>(timeout).count());
	}

	// Blocks the current thread until there's enough space to enqueue the given item,
	// or the timeout expires. Returns false without enqueueing the item if the timeout
	// expires, otherwise enqueues the item (via move, if possible) and returns true.
	// Thread-safe when called by producer thread.
	// No exception guarantee (state will be corrupted) if constructor of T throws.
	template<typename Rep, typename Period>
	inline bool wait_enqueue_timed(T&& item, std::chrono::duration<Rep, Period> const& timeout)
	{
		return wait_enqueue_timed(std::move(item), std::chrono::duration_cast<std::chrono::microseconds>(timeout).count());
	}

	// Attempts to dequeue a single item.
	// Returns false if the buffer is empty.
	// Thread-safe when called by consumer thread.
	// No exception guarantee (state will be corrupted) if assignment operator of U throws.
	template<typename U>
	bool try_dequeue(U& item)
	{
		if (!items->tryWait())
			return false;
		inner_dequeue(item);
		return true;
	}

	// Blocks the current thread until there's something to dequeue, then dequeues it.
	// Thread-safe when called by consumer thread.
	// No exception guarantee (state will be corrupted) if assignment operator of U throws.
	template<typename U>
	void wait_dequeue(U& item)
	{
		while (!items->wait());
		inner_dequeue(item);
	}

	// Blocks the current thread until either there's something to dequeue
	// or the timeout expires. Returns false without setting `item` if the
	// timeout expires, otherwise assigns to `item` and returns true.
	// Thread-safe when called by consumer thread.
	// No exception guarantee (state will be corrupted) if assignment operator of U throws.
	template<typename U>
	bool wait_dequeue_timed(U& item, std::int64_t timeout_usecs)
	{
		if (!items->wait(timeout_usecs))
			return false;
		inner_dequeue(item);
		return true;
	}

	// Blocks the current thread until either there's something to dequeue
	// or the timeout expires. Returns false without setting `item` if the
	// timeout expires, otherwise assigns to `item` and returns true.
	// Thread-safe when called by consumer thread.
	// No exception guarantee (state will be corrupted) if assignment operator of U throws.
	template<typename U, typename Rep, typename Period>
	inline bool wait_dequeue_timed(U& item, std::chrono::duration<Rep, Period> const& timeout)
	{
		return wait_dequeue_timed(item, std::chrono::duration_cast<std::chrono::microseconds>(timeout).count());
	}

	// Returns a pointer to the next element in the queue (the one that would
	// be removed next by a call to `try_dequeue` or `try_pop`). If the queue
	// appears empty at the time the method is called, returns nullptr instead.
	// Thread-safe when called by consumer thread.
	inline T* peek()
	{
		if (!items->availableApprox())
			return nullptr;
		return inner_peek();
	}

	// Pops the next element from the queue, if there is one.
	// Thread-safe when called by consumer thread.
	inline bool try_pop()
	{
		if (!items->tryWait())
			return false;
		inner_pop();
		return true;
	}

	// Returns a (possibly outdated) snapshot of the total number of elements currently in the buffer.
	// Thread-safe.
	inline std::size_t size_approx() const
	{
		return items->availableApprox();
	}

	// Returns the maximum number of elements that this circular buffer can hold at once.
	// Thread-safe.
	inline std::size_t max_capacity() const
	{
		return maxcap;
	}

private:
	template<typename U>
	void inner_enqueue(U&& item)
	{
		std::size_t i = nextSlot++;
		new (reinterpret_cast<T*>(data) + (i & mask)) T(std::forward<U>(item));
		items->signal();
	}

	template<typename U>
	void inner_dequeue(U& item)
	{
		std::size_t i = nextItem++;
		T& element = reinterpret_cast<T*>(data)[i & mask];
		item = std::move(element);
		element.~T();
		slots_->signal();
	}

	T* inner_peek()
	{
		return reinterpret_cast<T*>(data) + (nextItem & mask);
	}

	void inner_pop()
	{
		std::size_t i = nextItem++;
		reinterpret_cast<T*>(data)[i & mask].~T();
		slots_->signal();
	}

	template<typename U>
	static inline char* align_for(char* ptr)
	{
		const std::size_t alignment = std::alignment_of<U>::value;
		return ptr + (alignment - (reinterpret_cast<std::uintptr_t>(ptr) % alignment)) % alignment;
	}

private:
	std::size_t maxcap;                           // actual (non-power-of-two) capacity
	std::size_t mask;                             // circular buffer capacity mask (for cheap modulo)
	char* rawData;                                // raw circular buffer memory
	char* data;                                   // circular buffer memory aligned to element alignment
	std::unique_ptr<spsc_sema::LightweightSemaphore> slots_;  // number of slots currently free (named with underscore to accommodate Qt's 'slots' macro)
	std::unique_ptr<spsc_sema::LightweightSemaphore> items;   // number of elements currently enqueued
	char cachelineFiller0[MOODYCAMEL_CACHE_LINE_SIZE - sizeof(char*) * 2 - sizeof(std::size_t) * 2 - sizeof(std::unique_ptr<spsc_sema::LightweightSemaphore>) * 2];
	std::size_t nextSlot;                         // index of next free slot to enqueue into
	char cachelineFiller1[MOODYCAMEL_CACHE_LINE_SIZE - sizeof(std::size_t)];
	std::size_t nextItem;                         // index of next element to dequeue from
};

}
