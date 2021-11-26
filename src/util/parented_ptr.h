#pragma once

#include <QPointer>
#include <memory>

#include "util/assert.h"

// Use this wrapper class to clearly represent a raw pointer that is owned by the QT object tree.
// Objects which both derive from QObject AND have a parent object, have their lifetime governed by
// the QT object tree, and thus such pointers do not require a manual delete to free the heap memory
// when they go out of scope.
//
// NOTE: A parented_ptr must not dangle! Therefore, the lifetime of the parent must exceed the
// lifetime of the parented_ptr.
template<typename T>
class parented_ptr final {
  public:
    parented_ptr() noexcept
            : m_ptr{nullptr} {
    }

    parented_ptr(std::nullptr_t) noexcept
            : m_ptr{nullptr} {
    }

    explicit parented_ptr(T* t) noexcept
            : m_ptr{t} {
    }

    ~parented_ptr() noexcept {
        DEBUG_ASSERT(!m_ptr || static_cast<const QObject*>(m_ptr)->parent());
    }

    // Delete copy constructor and copy assignment operator
    parented_ptr(const parented_ptr<T>&) = delete;
    parented_ptr& operator=(const parented_ptr<T>&) = delete;

    // If U* is convertible to T* then parented_ptr<U> is convertible to parented_ptr<T>
    template<
            typename U,
            typename = typename std::enable_if<std::is_convertible<U*, T*>::value, U>::type>
    parented_ptr(parented_ptr<U>&& u) noexcept
            : m_ptr{u.m_ptr} {
        u.m_ptr = nullptr;
    }

    // If U* is convertible to T* then parented_ptr<U> is assignable to parented_ptr<T>
    template<
            typename U,
            typename = typename std::enable_if<std::is_convertible<U*, T*>::value, U>::type>
    parented_ptr& operator=(parented_ptr<U>&& u) noexcept {
        parented_ptr temp{std::move(u)};
        std::swap(temp.m_ptr, m_ptr);
        return *this;
    }

    parented_ptr& operator=(std::nullptr_t) noexcept {
        parented_ptr{std::move(*this)}; // move *this into a temporary that gets destructed
        return *this;
    }

    // Prevent unintended invocation of delete on parented_ptr
    operator void*() const = delete;

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

    template<typename>
    friend class parented_ptr;
};

template<typename T, typename... Args>
inline parented_ptr<T> make_parented(Args&&... args) {
    return parented_ptr<T>(new T(std::forward<Args>(args)...));
}

// A use case for this function is when giving an object owned by `std::unique_ptr` to a Qt
// function, that will make the object owned by the Qt object tree. Example:
// ```
// parent->someFunctionThatAddsAChild(to_parented(child))
// ```
// where `child` is a `std::unique_ptr`. After the call, the created `parented_ptr` will
// automatically be destructed such that the DEBUG_ASSERT that checks whether a parent exists is
// triggered.
template<typename T>
inline parented_ptr<T> to_parented(std::unique_ptr<T>& u) noexcept {
    // the DEBUG_ASSERT in the parented_ptr constructor will catch cases where the unique_ptr should
    // not have been released
    return parented_ptr<T>{u.release()};
}

// Comparison operator definitions:

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
