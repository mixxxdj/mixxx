#include "qml/qmlscreenmanager.h"

#include <QCoreApplication>
#include <QGuiApplication>

#include "moc_qmlscreenmanager.cpp"

namespace mixxx {
namespace qml {

QString QmlScreenManager::screenId(
        const QString& name, const QString& serialNumber) const {
    if (!serialNumber.isEmpty()) {
        return QStringLiteral("serial:") + serialNumber;
    }
    return QStringLiteral("name:") + name;
}

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
