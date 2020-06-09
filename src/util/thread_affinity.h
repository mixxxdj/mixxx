#pragma once

/// Debug assertion macros for detecting thread affinity violations.

#include <QCoreApplication>
#include <QThread>

#include "util/assert.h"

/// Assert that the current thread is the same as the host
/// thread of the given QObject pointer. That thread runs
/// the event loop for this object.
#define DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(pObject) \
    DEBUG_ASSERT(pObject);                            \
    DEBUG_ASSERT(pObject->thread() == QThread::currentThread())

/// Assert that the current thread is the same as the main
/// thread of the application.
#define DEBUG_ASSERT_MAIN_THREAD_AFFINITY() \
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(QCoreApplication::instance())
