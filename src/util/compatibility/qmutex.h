#pragma once

#include <QMutex>
#include <QMutexLocker>
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#include <QRecursiveMutex>
#endif

/// Transitional utility macros and functions to migrate from
/// non-templated QMutexLocker in Qt5 to templated
/// QMutexLocker<MutexType> in Qt6. Also includes some helpers
/// to avoid conditional compilation for QRecursiveMutex that
/// has been introduced in Qt 5.14.0.

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#define QT_RECURSIVE_MUTEX QRecursiveMutex
#define QT_RECURSIVE_MUTEX_INIT
#else
#define QT_RECURSIVE_MUTEX QMutex
#define QT_RECURSIVE_MUTEX_INIT QMutex::Recursive
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#define QT_MUTEX_LOCKER_TYPE(MUTEX_TYPE) QMutexLocker<MUTEX_TYPE>
#else
#define QT_MUTEX_LOCKER_TYPE(MUTEX_TYPE) QMutexLocker
#endif

#define QT_MUTEX_LOCKER QT_MUTEX_LOCKER_TYPE(QMutex)
#define QT_RECURSIVE_MUTEX_LOCKER QT_MUTEX_LOCKER_TYPE(QT_RECURSIVE_MUTEX)

[[nodiscard]] inline QT_MUTEX_LOCKER lockMutex(QMutex* pMutex) {
    return QT_MUTEX_LOCKER(pMutex);
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
[[nodiscard]] inline QT_RECURSIVE_MUTEX_LOCKER lockMutex(QRecursiveMutex* pMutex) {
    return QT_RECURSIVE_MUTEX_LOCKER(pMutex);
}
#endif
