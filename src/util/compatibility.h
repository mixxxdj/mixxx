#pragma once

// TODO: Split this file into separate header files in subdirectory util/compatibility

#include <QCoreApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QUuid>
#include <QWidget>
#include <QtDebug>

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
    return uuid.toString(QUuid::WithoutBraces);
}

inline
QString uuidToNullableStringWithoutBraces(const QUuid& uuid) {
    if (uuid.isNull()) {
        return QString();
    } else {
        return uuidToStringWithoutBraces(uuid);
    }
}
