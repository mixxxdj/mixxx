#pragma once

#include <QString> // IWYU pragma: keep
#include <QThread> // IWYU pragma: keep

#ifndef SET_THREAD_NAME
#define SET_THREAD_NAME(...) \
    QThread::currentThread()->setObjectName(__VA_ARGS__)
#endif

#ifndef SET_THREAD_NAME_P
#define SET_THREAD_NAME_P(base, ...)         \
    QThread::currentThread()->setObjectName( \
            QString::asprintf("%s 0x%x",     \
                    base,                    \
                    static_cast<unsigned int>(reinterpret_cast<intptr_t>(__VA_ARGS__))))
#endif

#ifndef SET_THREAD_NAME_FMT
#define SET_THREAD_NAME_FMT(base, ...) \
    QThread::currentThread()->setObjectName(QString(base " %1").arg(__VA_ARGS__))
#endif
