#include "battery/battery.h"

#ifdef Q_OS_LINUX
#include "battery/batterylinux.h"
#elif defined(Q_OS_WIN)
#include "battery/batterywindows.h"
#elif defined(Q_OS_MAC)
#include "battery/batterymac.h"
#endif

Battery::Battery(QObject *parent)
        : QObject(parent),
          m_csChargingState(UNKNOWN),
          m_iPercentage(0),
          m_iMinutesLeft(0),
          timer(this) {
    connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
    timer.start(s_iUpdateInterval);
}

Battery::~Battery() {
}

Battery* Battery::getBattery(QObject *parent) {
#ifdef Q_OS_LINUX
		return new BatteryLinux(parent);
#elif defined(Q_OS_WIN)
		return new BatteryWindows(parent);
#elif defined(Q_OS_MAC)
		return new BatteryMac(parent);
#else
		return NULL;
#endif
}

void Battery::update() {
    int lastPercentage = m_iPercentage;
    int lastMinutesLeft = m_iMinutesLeft;
    ChargingState lastChargingState = m_csChargingState;
    read();
    if (lastPercentage != m_iPercentage ||
        lastChargingState != m_csChargingState ||
        lastMinutesLeft != m_iMinutesLeft) {
        emit(stateChanged());
    }
}
