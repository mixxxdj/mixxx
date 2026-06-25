#include "qml/qmlbatteryproxy.h"

#include "moc_qmlbatteryproxy.cpp"

namespace mixxx {
namespace qml {

QmlBatteryProxy::QmlBatteryProxy(QObject* parent)
        : QObject(parent),
          m_pBattery(Battery::getBattery(this)) {
    if (m_pBattery) {
        connect(m_pBattery.data(),
                &Battery::stateChanged,
                this,
                &QmlBatteryProxy::slotStateChanged);
        m_pBattery->update();
    }
}

double QmlBatteryProxy::percentage() const {
    return m_pBattery ? m_pBattery->getPercentage() : 0.0;
}

bool QmlBatteryProxy::isCharging() const {
    if (!m_pBattery) {
        return false;
    }
    const auto state = m_pBattery->getChargingState();
    return state == Battery::CHARGING || state == Battery::CHARGED;
}

int QmlBatteryProxy::minutesLeft() const {
    return m_pBattery ? m_pBattery->getMinutesLeft() : 0;
}

bool QmlBatteryProxy::isBatteryAvailable() const {
    return m_pBattery != nullptr;
}

void QmlBatteryProxy::slotStateChanged() {
    emit stateChanged();
}

} // namespace qml
} // namespace mixxx
