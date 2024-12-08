#pragma once

#include <QMutex>
#include <memory>

#include "util/assert.h"

// @brief A wrapper class to protect pointers owned by another entity, allowing
// safe access to the referenced object across threads. This prevents the
// object from being deleted by its owner while other threads are executing
// functions on the referenced object, implemented mostly lock-free, except during
// reset(), if the object is still in use.
//
// Use cases are:
// - Implementing callbacks to objects with shorter lifetimes.
// - A safe replacement for Qt's direct connections, which must not be used across
//   threads in such cases.
//
// @note Unlike `std::shared_ptr`, the object is not deleted when the last
// `borrowed_ptr` falls out of scope. Ownership and deletion are always managed
// by the original owner (via `borrowable_ptr`).
//
// If the `borrowable_ptr` falls out of scope while there are active `borrowed_ptr`
// instances, the thread is suspended until all `borrowed_ptr` instances are deleted
// and the reference count reaches zero.
//
// Thread safety:
// - It is thread-safe to replace the object in `borrowable_ptr`.
// - The transition is not atomic: `borrowed_ptr` instances become null before pointing
//   to the new object.

// Usage:
//	int i = 5;
//	{
//	    // Make i borrowable, owned by the stack
//      auto borrowable = borrowable_ptr(&i);
//	    {
//	        // Borrow i, this can be borrowd across threads
//          // borrowed_ptr is a strong reference that guranties the borrowed object is
//          // valid unit the end of the scope
//          borrowed_ptr borrowed1 = borrowable.borrow();
//	        borrowed_ptr borrowed2 = borrowable.borrow();
//	    } // borrowed objects are returned
//	} // borrowable_ptr falls out of scope, suspended if a borrowed pointer is not returned

template<typename Tp>
class borrowable_ptr;

template<typename Tp>
class borrowed_ptr : public std::shared_ptr<Tp> {
  public:
    constexpr borrowed_ptr() noexcept
            : std::shared_ptr<Tp>() {
    }

    borrowed_ptr(const borrowed_ptr<Tp>& other) noexcept
            : std::shared_ptr<Tp>(other) {
    }

    borrowed_ptr(borrowed_ptr<Tp>&& other) noexcept
            : std::shared_ptr<Tp>(std::move(other)) {
    }

    borrowed_ptr& operator=(const borrowed_ptr& other) noexcept = default;

  private:
    // @brief Internal constructor used by `borrowable_ptr` to create a borrowed pointer.
    // @param other A non owning shared pointer to the managed object.
    borrowed_ptr(const std::shared_ptr<Tp>& other) noexcept
            : std::shared_ptr<Tp>(other) {
    }

    friend class borrowable_ptr<Tp>;
};

template<typename Tp>
class borrowable_ptr {
  private:
    // @brief Custom deleter to manage the mutex during object destruction.
    class borrowable_deleter {
      public:
        explicit borrowable_deleter(QMutex* pMutex)
                : m_pMutex(pMutex) {
            // Lock the mutex to indicate the object is in use.
            m_pMutex->lock();
        }

        template<typename T>
        void operator()(T*) {
            // Last reference is gone; release the mutex to allow `borrowable_ptr` to clean up.
            m_pMutex->unlock();
        }

      private:
        QMutex* m_pMutex;
    };

  public:
    borrowable_ptr() {
    }

    // @brief Construct a `borrowable_ptr` managing a raw pointer but not owning
    // @param p Raw pointer to the managed object.
    explicit borrowable_ptr(Tp* p) {
        if (p) {
            m_sharedPtr = std::shared_ptr<Tp>(p, borrowable_deleter(&m_mutex));
        }
    }

    ~borrowable_ptr() {
        reset();
        m_mutex.unlock();
    }

    // @brief Assign a new raw pointer to the `borrowable_ptr` but not owning
    // @param p Raw pointer to the new object.
    // @return Reference to this instance.
    borrowable_ptr& operator=(Tp* p) {
        reset();
        m_mutex.unlock();
        m_sharedPtr = std::shared_ptr<Tp>(p, borrowable_deleter(&m_mutex));
        return *this;
    }

    // @brief Borrow a strong reference to the managed object.
    // @return A `borrowed_ptr` instance pointing to the managed object.
    borrowed_ptr<Tp> borrow() {
        return borrowed_ptr<Tp>(m_sharedPtr);
    }

    borrowable_ptr& operator=(const borrowable_ptr& other) {
        this->operator=(other.get());
        return *this;
    }

    void reset() {
        m_sharedPtr.reset();
        // Wait until all borrowed references are released.
        m_mutex.lock();
    }

    // @brief Get the raw pointer to the managed object.
    // @return Raw pointer to the managed object.
    Tp* get() const noexcept {
        return m_sharedPtr.get();
    }

  private:
    QMutex m_mutex;
    std::shared_ptr<Tp> m_sharedPtr; ///< Non-owning shared pointer to the managed object.
};
