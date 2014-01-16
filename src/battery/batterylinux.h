#ifndef BATTERYLINUX_H
#define BATTERYLINUX_H

#include "battery/battery.h"

class BatteryLinux : public Battery {
	Q_OBJECT

  public:
	BatteryLinux(QObject *parent=0,
				 QString infoFile = "/proc/acpi/battery/BAT0/info",
				 QString stateFile = "/proc/acpi/battery/BAT0/state");
	virtual ~BatteryLinux();

  protected:
	void read();
	ChargingState readChargingState();
	int readMinutesLeft();
	int readCurrentCapacity() {
		return readValue(m_sStateFile, s_sCurrentCapacityKeyword);
	}
	int readCurrentRate() {
		return readValue(m_sStateFile, s_sCurrentRateKeyword);
	}
	int readMaximumCapacity() {
		return readValue(m_sInfoFile, s_sMaximumCapacityKeyword);
	}
	int readPercentage() {
		if (!m_iMaximumCapacity) return 0; // Prevent division by 0
		return m_iCurrentCapacity * 100 / m_iMaximumCapacity;
	}

  private:
	static const QString s_sMaximumCapacityKeyword;
	static const QString s_sCurrentCapacityKeyword;
	static const QString s_sChargingStateKeyword;
	static const QString s_sCurrentRateKeyword;

	// readValue looks for sKeyword in sFile and returns it's value
	// used by readCurrentCapacity() readMaximumCapacity() and readCurrentRate()
	int readValue(QString sFile, QString sKeyword);

	// general Information about the battery
	// used to read the maximum capacity
	const QString m_sInfoFile;

	// gives current informations about battery (status, capacity, rate)
	const QString m_sStateFile;

	int m_iMaximumCapacity;
	int m_iCurrentCapacity; // Battery capacity in mAh
	int m_iCurrentRate; // charging/discharging rate in mA
};

#endif /* BATTERYLINUX_H */
