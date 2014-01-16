#ifndef BATTERYLINUX_H
#define BATTERYLINUX_H

#include "battery/battery.h"

class BatteryLinux : public Battery {
    Q_OBJECT
  public:
    BatteryLinux(QObject* pParent=NULL,
                 const QString& infoFile="/proc/acpi/battery/BAT0/info",
                 const QString& stateFile="/proc/acpi/battery/BAT0/state");
    virtual ~BatteryLinux();

  protected:
    void read();

    ChargingState readChargingState() const;
    int readMinutesLeft() const;
    int readCurrentCapacity() const {
        return readValue(m_sStateFile, s_sCurrentCapacityKeyword);
    }
    int readCurrentRate() const {
        return readValue(m_sStateFile, s_sCurrentRateKeyword);
    }

    int readMaximumCapacity() const {
        return readValue(m_sInfoFile, s_sMaximumCapacityKeyword);
    }

    int readPercentage() const {
        // Prevent division by 0.
        if (!m_iMaximumCapacity)
            return 0;
        return m_iCurrentCapacity * 100 / m_iMaximumCapacity;
    }

  private:
    static const QString s_sMaximumCapacityKeyword;
    static const QString s_sCurrentCapacityKeyword;
    static const QString s_sChargingStateKeyword;
    static const QString s_sCurrentRateKeyword;

    // readValue looks for sKeyword in sFile and returns its value. Used to read
    // battery information from the /proc filesystem.
    int readValue(const QString& sFile, const QString& sKeyword) const;

    // general information about the battery used to read the maximum capacity
    const QString m_sInfoFile;

    // gives current informations about battery (status, capacity, rate)
    const QString m_sStateFile;

    // Battery maximum capacity in mAh.
    int m_iMaximumCapacity;

    // Battery current capacity in mAh.
    int m_iCurrentCapacity;

    // Charge/discharge rate in mA.
    int m_iCurrentRate;
};

#endif /* BATTERYLINUX_H */
