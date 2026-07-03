#pragma once

#include <QThread>
#if QT_VERSION < QT_VERSION_CHECK(6, 8, 0)
#include <QCoreApplication>
#endif

inline bool inMainThread() noexcept {
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    return QThread::isMainThread();
#else
    return QCoreApplication::instance()->thread() == QThread::currentThread();
#endif
}
