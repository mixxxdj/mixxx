#pragma once

// TODO: Split this file into separate header files in subdirectory util/compatibility

#include <QCoreApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QWidget>
#include <QtDebug>

#include "util/assert.h"

inline QScreen* getPrimaryScreen() {
    QGuiApplication* app = static_cast<QGuiApplication*>(QCoreApplication::instance());
    VERIFY_OR_DEBUG_ASSERT(app) {
        qWarning() << "Unable to get applications QCoreApplication instance, cannot determine primary screen!";
        // All attempts to find primary screen failed, return nullptr
        return nullptr;
    }
    return app->primaryScreen();
}
