#pragma once

#include <QAtomicInteger>
#include <QAtomicPointer>

// TODO: QBasicAtomicInteger/Pointer<T>::load/store() are deprecated and
// should be replaced with their explicit relaxed/acquire/release counterparts.
// However, the proposed alternatives have just been introduced in Qt 5.14.
// Until the minimum required Qt version of Mixxx is increased some utility
// functions that work independently of the Qt version are needed.

template<typename T>
inline T atomicLoadRelaxed(const QAtomicInteger<T>& atomicInt) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return atomicInt.loadRelaxed();
#else
    return atomicInt.load();
#endif
}

template<typename T>
inline T* atomicLoadRelaxed(const QAtomicPointer<T>& atomicPtr) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return atomicPtr.loadRelaxed();
#else
    return atomicPtr.load();
#endif
}

template<typename T>
inline void atomicStoreRelaxed(QAtomicPointer<T>& atomicPtr, T* newValue) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    atomicPtr.storeRelaxed(newValue);
#else
    atomicPtr.store(newValue);
#endif
}
