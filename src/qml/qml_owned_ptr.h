#pragma once

#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <memory>

#include "util/assert.h"

// Use this wrapper class to clearly represent a raw pointer that is owned by a
// QML Engine. Objects which derive from QObject, have their lifetime governed
// by the QML (or JavaScript) Engine, and thus such pointers do not require a
// manual delete to free the heap memory when they go out of scope, as they will
// be handled by the engine garbage collector.
template<typename T>
    requires(std::is_base_of_v<QObject, T>)
class qml_owned_ptr final {
  public:
    explicit qml_owned_ptr(T* t = nullptr) noexcept
            : m_ptr{t} {
        if (m_ptr) {
            QQmlEngine::setObjectOwnership(m_ptr, QQmlEngine::JavaScriptOwnership);
        }
    }

    // explicitly generate trivial destructor (since decltype(m_ptr) is not a class type)
    ~qml_owned_ptr() noexcept = default;

    // Rule of 5
    qml_owned_ptr(const qml_owned_ptr<T>& other)
            : m_ptr{other.m_ptr} {
        DEBUG_ASSERT(!m_ptr ||
                QQmlEngine::objectOwnership(m_ptr) ==
                        QQmlEngine::JavaScriptOwnership);
    }
    qml_owned_ptr& operator=(const qml_owned_ptr<T>&) = delete;
    qml_owned_ptr(const qml_owned_ptr<T>&& other)
            : m_ptr{other.m_ptr} {
        DEBUG_ASSERT(!m_ptr ||
                QQmlEngine::objectOwnership(m_ptr) ==
                        QQmlEngine::JavaScriptOwnership);
    }
    qml_owned_ptr& operator=(const qml_owned_ptr<T>&& other) = delete;

    // If U* is convertible to T* then qml_owned_ptr<U> is convertible to qml_owned_ptr<T>
    template<
            typename U,
            typename = typename std::enable_if_t<std::is_convertible_v<U*, T*>, U>>
    qml_owned_ptr(qml_owned_ptr<U>&& u) noexcept
            : m_ptr{u.m_ptr} {
        u.m_ptr = nullptr;
    }

    // If U* is convertible to T* then qml_owned_ptr<U> is assignable to qml_owned_ptr<T>
    template<typename U>
        requires std::is_convertible_v<U*, T*>
    qml_owned_ptr& operator=(qml_owned_ptr<U>&& u) noexcept {
        qml_owned_ptr temp{std::move(u)};
        std::swap(temp.m_ptr, m_ptr);
        DEBUG_ASSERT(!m_ptr ||
                QQmlEngine::objectOwnership(m_ptr) ==
                        QQmlEngine::JavaScriptOwnership);
        return *this;
    }

    qml_owned_ptr& operator=(std::nullptr_t) noexcept {
        qml_owned_ptr{std::move(*this)}; // move *this into a temporary that gets destructed
        return *this;
    }

    // Prevent unintended invocation of delete on qml_owned_ptr
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
};

template<typename T, typename... Args>
qml_owned_ptr<T> make_qml_owned(Args&&... args) {
    return qml_owned_ptr<T>(new T(std::forward<Args>(args)...));
}

// A use case for this function is when giving an object owned by `std::unique_ptr` to a Qt
// function, that will make the object owned by the Qt object tree. Example:
// ```
// parent->someFunctionThatAddsAChild(to_qml_owned(child))
// ```
// where `child` is a `std::unique_ptr`. After the call, the created `qml_owned_ptr` will
// automatically be destructed such that the DEBUG_ASSERT that checks whether a parent exists is
// triggered.
template<typename T>
qml_owned_ptr<T> to_qml_owned(std::unique_ptr<T>& u) noexcept {
    // the DEBUG_ASSERT in the qml_owned_ptr constructor will catch cases where
    // the unique_ptr should not have been released
    return qml_owned_ptr<T>{u.release()};
}
