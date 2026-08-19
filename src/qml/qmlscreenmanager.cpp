#include "qml/qmlscreenmanager.h"

#include <QCoreApplication>
#include <QGuiApplication>

#include "moc_qmlscreenmanager.cpp"

namespace mixxx {
namespace qml {

QmlScreenManager::QmlScreenManager(QObject* parent)
        : QObject(parent) {
    auto* pApplication =
            qobject_cast<QGuiApplication*>(QCoreApplication::instance());
    if (!pApplication) {
        return;
    }

    connect(pApplication,
            &QGuiApplication::screenAdded,
            this,
            &QmlScreenManager::screenAdded);
    connect(pApplication,
            &QGuiApplication::screenRemoved,
            this,
            &QmlScreenManager::screenRemoved);
}

} // namespace qml
} // namespace mixxx
