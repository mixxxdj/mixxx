#pragma once

/// Common utility functions and helpers that are needed for
/// all Qt versions.

#include <QPointer>
#include <QString>

namespace mixxx {

/// Allow to emit non-const signals from a const member function.
///
/// https://github.com/KDE/clazy/blob/master/docs/checks/README-const-signal-or-slot.md
///
/// This is needed as a workaround for many DAO classes. Most member
/// functions should be const and those classes should not emit any
/// signals.
///
/// Usage: emit thisAsNonConst(this)->signalName(<signal args>);
template<typename T>
inline T* thisAsNonConst(const T* constThisPointer) {
    return const_cast<T*>(constThisPointer);
}

/// Escape special characters in text properties to prevent
/// the implicit creation of shortcuts for widgets.
///
/// The given text is supposed to be displayed literally on
/// the screen and no shortcuts shall be created.
///
/// Needed for: QAction, QAbstractButton
inline QString escapeTextPropertyWithoutShortcuts(QString text) {
    text.replace(QChar('&'), QStringLiteral("&&"));
    return text;
}

/// A safe wrapper for QPointer.
///
/// Prevents directly dereferencing the internal pointer. The internal
/// pointer in QPointer might be reset and become a nullptr at any time
/// when the referenced QObject lives in a different thread. This
/// behavior could cause spurious crashes due to race conditions.
template<typename T>
class SafeQPointer final {
  public:
    SafeQPointer() = default;
    SafeQPointer(std::nullptr_t) {
    }
    template<typename U>
    explicit SafeQPointer(U* ptr)
            : m_ptr(ptr) {
    }

    void clear() {
        m_ptr.clear();
    }

    T* data() const {
        return m_ptr.data();
    }

    operator bool() const {
        return m_ptr;
    }

    SafeQPointer<T>& operator=(std::nullptr_t) {
        m_ptr.clear();
        return *this;
    }
    template<typename U>
    SafeQPointer<T>& operator=(U* ptr) {
        m_ptr = ptr;
        return *this;
    }

  private:
    QPointer<T> m_ptr;
};

class ScopedDeleteLater final {
  public:
    explicit ScopedDeleteLater(QObject* pObject)
            : m_pObject(pObject) {
    }
    ~ScopedDeleteLater() {
        if (m_pObject) {
            m_pObject->deleteLater();
        }
    }
    ScopedDeleteLater(ScopedDeleteLater&&) = delete;
    ScopedDeleteLater(const ScopedDeleteLater&) = delete;

  private:
    QObject* const m_pObject;
};

} // namespace mixxx
