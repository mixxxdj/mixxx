#pragma once

#include <QJSEngine>
#include <QObject>
#include <QQmlEngine>
#include <QScreen>
#include <QString>

namespace mixxx {
namespace qml {

class QmlScreenManager : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(ScreenManager)
    QML_SINGLETON

  public:
    explicit QmlScreenManager(QObject* parent = nullptr);
    ~QmlScreenManager() override = default;

    static QmlScreenManager* create(
            QQmlEngine* pQmlEngine,
            [[maybe_unused]] QJSEngine* pJsEngine) {
        return new QmlScreenManager(pQmlEngine);
    }

    Q_INVOKABLE QString screenId(
            const QString& name, const QString& serialNumber) const;

  signals:
    void screenAdded(QScreen* screen);
    void screenRemoved(QScreen* screen);
};

} // namespace qml
} // namespace mixxx
