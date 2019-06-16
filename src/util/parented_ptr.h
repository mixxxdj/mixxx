#ifndef UTIL_PARENTED_PTR_H
#define UTIL_PARENTED_PTR_H

#include <QPointer>

#include "util/assert.h"
#include "util/memory.h"

/**
 * Use this wrapper class to clearly represent a raw pointer that is owned by the QT object tree.
 * Objects which both derive from QObject AND have a parent object, have their lifetime governed by the QT object tree, 
 * and thus such pointers do not require a manual delete to free the heap memory when they go out of scope.
**/
template<typename T>
class parented_ptr {
  public:
    parented_ptr() noexcept = default;

    parented_ptr(nullptr_t) noexcept
            : m_ptr{nullptr} {
    }

    explicit parented_ptr(T* t) noexcept
            : m_ptr{t} {
    }

    ~parented_ptr() noexcept {
        if (m_ptr != nullptr) {
            DEBUG_ASSERT(m_ptr->parent() != nullptr);
        }
    }

    // Delete copy constructor and copy assignment operator
    parented_ptr(const parented_ptr<T>&) = delete;
    parented_ptr& operator=(const parented_ptr<T>&) = delete;

    /*
     * If U* is convertible to T* then we also want parented_ptr<U> convertible to parented_ptr<T>
     */
    template<
            typename U,
            typename = typename std::enable_if<std::is_convertible<U*, T*>::value, U>::type>
    parented_ptr(parented_ptr<U>&& u) noexcept
            : m_ptr{u.m_ptr} {
        u.m_ptr = nullptr;
    }

    /*
     * If U* is convertible to T* then we also want parented_ptr<U> assignable to parented_ptr<T>
     * E.g. parented_ptr<Base> base = make_parented<Derived>(); should work as expected.
     */
    template<
            typename U,
            typename = typename std::enable_if<std::is_convertible<U*, T*>::value, U>::type>
    parented_ptr& operator=(parented_ptr<U>&& u) noexcept {
        parented_ptr temp{std::move(u)};
        std::swap(temp.m_ptr, m_ptr);
        return *this;
    }

    parented_ptr& operator=(nullptr_t) noexcept {
        m_ptr = nullptr;
        return *this;
    }

    operator T*() const noexcept {
        return m_ptr;
    }

    T* get() const noexcept {
        return m_ptr;
    }

    T& operator*() const noexcept {
        return *m_ptr;
    }

    T* operator->() const noexcept {
        return m_ptr;
    }

    operator bool() const noexcept {
        return m_ptr != nullptr;
    }

    QPointer<T> toWeakRef() {
        return m_ptr;
    }

  private:
    T* m_ptr;
};

namespace {

template<typename T, typename... Args>
inline parented_ptr<T> make_parented(Args&&... args) {
    return parented_ptr<T>(new T(std::forward<Args>(args)...));
}

/// A use case for this function is when giving an object owned by `std::unique_ptr` to a Qt
/// function, that will make the object owned by the Qt object tree. Example:
/// ```
/// parent->someFunctionThatAddsAChild(to_parented(child))
/// ```
/// where `child` is a `std::unique_ptr`. After the call, the created `parented_ptr` will
/// automatically be destructed such that the DEBUG_ASSERT for an existent parent is triggered.
template<typename T>
inline parented_ptr<T> to_parented(std::unique_ptr<T>& u) noexcept {
    // the DEBUG_ASSERT in the parented_ptr constructor will catch cases where the unique_ptr should
    // not have been released
    return parented_ptr<T>{u.release()};
}

// Comparison operator definitions
template<typename T, typename U>
inline bool operator==(const T* lhs, const parented_ptr<U>& rhs) noexcept {
    return lhs == rhs.get();
}

template<typename T, typename U>
inline bool operator==(const parented_ptr<T>& lhs, const U* rhs) noexcept {
    return lhs.get() == rhs;
}

template<typename T, typename U>
inline bool operator==(const parented_ptr<T>& lhs, const parented_ptr<U>& rhs) noexcept {
    return lhs.get() == rhs.get();
}

template<typename T, typename U>
inline bool operator!=(const T* lhs, const parented_ptr<U>& rhs) noexcept {
    return !(lhs == rhs.get());
}

template<typename T, typename U>
inline bool operator!=(const parented_ptr<T>& lhs, const U* rhs) noexcept {
    return !(lhs.get() == rhs);
}

template<typename T, typename U>
inline bool operator!=(const parented_ptr<T>& lhs, const parented_ptr<U>& rhs) noexcept {
    return !(lhs.get() == rhs.get());
}

} // namespace

#endif // UTIL_PARENTED_PTR_H
