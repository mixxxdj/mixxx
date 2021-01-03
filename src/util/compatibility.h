#pragma once

#include <QCoreApplication>
#include <QGuiApplication>
#include <QList>
#include <QScreen>
#include <QUuid>
#include <QWidget>

#include "util/assert.h"

inline qreal getDevicePixelRatioF(const QWidget* widget) {
    return widget->devicePixelRatioF();
}

inline QScreen* getPrimaryScreen() {
    QGuiApplication* app = static_cast<QGuiApplication*>(QCoreApplication::instance());
    VERIFY_OR_DEBUG_ASSERT(app) {
        qWarning() << "Unable to get applications QCoreApplication instance, cannot determine primary screen!";
        // All attempts to find primary screen failed, return nullptr
        return nullptr;
    }
    return app->primaryScreen();
}

inline
QString uuidToStringWithoutBraces(const QUuid& uuid) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    return uuid.toString(QUuid::WithoutBraces);
#else
    QString withBraces = uuid.toString();
    DEBUG_ASSERT(withBraces.size() == 38);
    DEBUG_ASSERT(withBraces.startsWith('{'));
    DEBUG_ASSERT(withBraces.endsWith('}'));
    // We need to strip off the heading/trailing curly braces after formatting
    return withBraces.mid(1, 36);
#endif
}

inline
QString uuidToNullableStringWithoutBraces(const QUuid& uuid) {
    if (uuid.isNull()) {
        return QString();
    } else {
        return uuidToStringWithoutBraces(uuid);
    }
}

template <typename T>
inline T atomicLoadAcquire(const QAtomicInteger<T>& atomicInt) {
    // TODO: QBasicAtomicInteger<T>::load() is deprecated and should be
    // replaced with QBasicAtomicInteger<T>::loadRelaxed() However, the
    // proposed alternative has just been introduced in Qt 5.14. Until the
    // minimum required Qt version of Mixxx is increased, we need a version
    // check here
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return atomicInt.loadAcquire();
#else
    return atomicInt.load();
#endif
}

template <typename T>
inline T* atomicLoadAcquire(const QAtomicPointer<T>& atomicPtr) {
    // TODO: QBasicAtomicPointer<T>::load() is deprecated and should be
    // replaced with QBasicAtomicPointer<T>::loadRelaxed() However, the
    // proposed alternative has just been introduced in Qt 5.14. Until the
    // minimum required Qt version of Mixxx is increased, we need a version
    // check here
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return atomicPtr.loadAcquire();
#else
    return atomicPtr.load();
#endif
}

template <typename T>
inline T atomicLoadRelaxed(const QAtomicInteger<T>& atomicInt) {
    // TODO: QBasicAtomicInteger<T>::load() is deprecated and should be
    // replaced with QBasicAtomicInteger<T>::loadRelaxed() However, the
    // proposed alternative has just been introduced in Qt 5.14. Until the
    // minimum required Qt version of Mixxx is increased, we need a version
    // check here
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return atomicInt.loadRelaxed();
#else
    return atomicInt.load();
#endif
}

template <typename T>
inline T* atomicLoadRelaxed(const QAtomicPointer<T>& atomicPtr) {
    // TODO: QBasicAtomicPointer<T>::load() is deprecated and should be
    // replaced with QBasicAtomicPointer<T>::loadRelaxed() However, the
    // proposed alternative has just been introduced in Qt 5.14. Until the
    // minimum required Qt version of Mixxx is increased, we need a version
    // check here
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return atomicPtr.loadRelaxed();
#else
    return atomicPtr.load();
#endif
}

template <typename T>
inline void atomicStoreRelaxed(QAtomicInteger<T>& atomicInt, T newValue) {
    // TODO: QBasicAtomicInteger<T>::store(T newValue) is deprecated and should
    // be replaced with QBasicAtomicInteger<T>::storeRelaxed(T newValue)
    // However, the proposed alternative has just been introduced in Qt 5.14.
    // Until the minimum required Qt version of Mixxx is increased, we need a
    // version check here
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    atomicInt.storeRelaxed(newValue);
#else
    atomicInt.store(newValue);
#endif
}

template <typename T>
inline void atomicStoreRelaxed(QAtomicPointer<T>& atomicPtr, T* newValue) {
    // TODO: QBasicAtomicPointer<T>::store(T* newValue) is deprecated and
    // should be replaced with QBasicAtomicPointer<T>::storeRelaxed(T*
    // newValue) However, the proposed alternative has just been introduced in
    // Qt 5.14. Until the minimum required Qt version of Mixxx is increased, we
    // need a version check here
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    atomicPtr.storeRelaxed(newValue);
#else
    atomicPtr.store(newValue);
#endif
}

/// Helper to insert values into a QList with specific indices.
///
/// *For legacy code only - Do not use for new code!*
template<typename T>
inline void listAppendOrReplaceAt(QList<T>* pList, int index, const T& value) {
    VERIFY_OR_DEBUG_ASSERT(index <= pList->size()) {
        qWarning() << "listAppendOrReplaceAt: Padding list with"
                   << (index - pList->size()) << "default elements";
        while (index > pList->size()) {
            pList->append(T());
        }
    }
    VERIFY_OR_DEBUG_ASSERT(index == pList->size()) {
        pList->replace(index, value);
        return;
    }
    pList->append(value);
}
