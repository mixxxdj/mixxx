#include "battery/battery.h"
#include "battery/batterylinux.h"
//#include "battery/batterymac.h"
//#include "battery/batterywin.h"

Battery::Battery(QObject *parent) :
  QObject(parent),
  m_csChargingState(UNKNOWN),
  timer(this) {
	connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
	timer.start(s_iUpdateInterval);
}

Battery::~Battery() {

}

Battery* Battery::getBattery(QObject *parent) {
	#ifdef Q_OS_LINUX
		return new BatteryLinux(parent);
	#elif Q_OS_WIN
		//return new BatteryWindows(parent);
		// TODO(XXX) implement BatteryWindows
		return NULL;
	#elif Q_OS_MAC
		//return new BatteryMac(parent);
		// TODO(XXX) implement BatteryMac
		return NULL;
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
