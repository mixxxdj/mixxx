#pragma once

#include <QCoreApplication>
#include <QGuiApplication>
#include <QList>
#include <QScreen>
#include <QUuid>
#include <QWindow>
#include <QWidget>

#include "util/assert.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)

// this adds const to non-const objects (like std::as_const)
template <typename T>
struct QAddConst { typedef const T Type; };
template <typename T>
constexpr typename QAddConst<T>::Type &qAsConst(T &t) { return t; }
// prevent rvalue arguments:
template <typename T>
void qAsConst(const T &&) = delete;

template <typename... Args>
struct QNonConstOverload
{
    template <typename R, typename T>
    Q_DECL_CONSTEXPR auto operator()(R (T::*ptr)(Args...)) const Q_DECL_NOTHROW -> decltype(ptr)
    { return ptr; }

    template <typename R, typename T>
    static Q_DECL_CONSTEXPR auto of(R (T::*ptr)(Args...)) Q_DECL_NOTHROW -> decltype(ptr)
    { return ptr; }
};

template <typename... Args>
struct QConstOverload
{
    template <typename R, typename T>
    Q_DECL_CONSTEXPR auto operator()(R (T::*ptr)(Args...) const) const Q_DECL_NOTHROW -> decltype(ptr)
    { return ptr; }

    template <typename R, typename T>
    static Q_DECL_CONSTEXPR auto of(R (T::*ptr)(Args...) const) Q_DECL_NOTHROW -> decltype(ptr)
    { return ptr; }
};

template <typename... Args>
struct QOverload : QConstOverload<Args...>, QNonConstOverload<Args...>
{
    using QConstOverload<Args...>::of;
    using QConstOverload<Args...>::operator();
    using QNonConstOverload<Args...>::of;
    using QNonConstOverload<Args...>::operator();

    template <typename R>
    Q_DECL_CONSTEXPR auto operator()(R (*ptr)(Args...)) const Q_DECL_NOTHROW -> decltype(ptr)
    { return ptr; }

    template <typename R>
    static Q_DECL_CONSTEXPR auto of(R (*ptr)(Args...)) Q_DECL_NOTHROW -> decltype(ptr)
    { return ptr; }
};

#endif


inline qreal getDevicePixelRatioF(const QWidget* widget) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    return widget->devicePixelRatioF();
#endif

    // Crawl up to the window and return qreal value
    QWindow* window = widget->window()->windowHandle();
    if (window) {
        return window->devicePixelRatio();
    }

    // return integer value as last resort
    return widget->devicePixelRatio();
}

inline QScreen* getPrimaryScreen() {
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QGuiApplication* app = static_cast<QGuiApplication*>(QCoreApplication::instance());
    VERIFY_OR_DEBUG_ASSERT(app) {
        qWarning() << "Unable to get applications QCoreApplication instance, cannot determine primary screen!";
    } else {
        return app->primaryScreen();
    }
#endif
    const QList<QScreen*> screens = QGuiApplication::screens();
    VERIFY_OR_DEBUG_ASSERT(!screens.isEmpty()) {
        qWarning() << "No screens found, cannot determine primary screen!";
    } else {
        return screens.first();
    }

    // All attempts to find primary screen failed, return nullptr
    return nullptr;
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
    // minimum required Qt version of Mixx is increased, we need a version
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
    // minimum required Qt version of Mixx is increased, we need a version
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
    // minimum required Qt version of Mixx is increased, we need a version
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
    // minimum required Qt version of Mixx is increased, we need a version
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
    // Until the minimum required Qt version of Mixx is increased, we need a
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
    // Qt 5.14. Until the minimum required Qt version of Mixx is increased, we
    // need a version check here
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    atomicPtr.storeRelaxed(newValue);
#else
    atomicPtr.store(newValue);
#endif
}

#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
template<typename T>
inline uint qHash(const QList<T>& key, uint seed = 0) {
    uint hash = 0;
    for (const auto& elem : key) {
        hash ^= qHash(elem, seed);
    }
    return hash;
}
#endif
