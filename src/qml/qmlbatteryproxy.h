#pragma once

#include <QJSEngine>
#include <QObject>
#include <QQmlEngine>
#include <QSharedPointer>

#include "util/battery/battery.h"

namespace mixxx {
namespace qml {

class QmlBatteryProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(double percentage READ percentage NOTIFY stateChanged)
    Q_PROPERTY(bool isCharging READ isCharging NOTIFY stateChanged)
    Q_PROPERTY(int minutesLeft READ minutesLeft NOTIFY stateChanged)
    Q_PROPERTY(bool isBatteryAvailable READ isBatteryAvailable CONSTANT)
    QML_NAMED_ELEMENT(Battery)
    QML_SINGLETON
  public:
    explicit QmlBatteryProxy(QObject* parent = nullptr);
    ~QmlBatteryProxy() override = default;

    static QmlBatteryProxy* create(
            QQmlEngine* pQmlEngine,
            [[maybe_unused]] QJSEngine* pJsEngine) {
        return new QmlBatteryProxy(pQmlEngine);
    }

    double percentage() const;
    bool isCharging() const;
    int minutesLeft() const;
    bool isBatteryAvailable() const;

  signals:
    void stateChanged();

  private slots:
    void slotStateChanged();

  private:
    QSharedPointer<Battery> m_pBattery;
};

} // namespace qml
} // namespace mixxx
