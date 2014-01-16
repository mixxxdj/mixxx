#ifndef BATTERYLINUX_H
#define BATTERYLINUX_H

#include "util/battery/battery.h"

class BatteryLinux : public Battery {
    Q_OBJECT
  public:
    BatteryLinux(QObject* pParent=NULL,
                 const QString& infoFile="/proc/acpi/battery/BAT0/info",
                 const QString& stateFile="/proc/acpi/battery/BAT0/state");
    virtual ~BatteryLinux();

  protected:
    void read();

  private:
    static const QString s_sMaximumCapacityKeyword;
    static const QString s_sCurrentCapacityKeyword;
    static const QString s_sChargingStateKeyword;
    static const QString s_sCurrentRateKeyword;

    // readValue looks for sKeyword in sFile and returns its value. Used to read
    // battery information from the /proc filesystem.
    int readValue(const QString& sFile, const QString& sKeyword) const;

    ChargingState readChargingState() const;
    int readCurrentCapacity() const {
        return readValue(m_sStateFile, s_sCurrentCapacityKeyword);
    }
    int readCurrentRate() const {
        return readValue(m_sStateFile, s_sCurrentRateKeyword);
    }
    int readMaximumCapacity() const {
        return readValue(m_sInfoFile, s_sMaximumCapacityKeyword);
    }

    int getMinutesLeft(ChargingState chargingState, int currentCapacity,
                       int maximumCapacity, int currentRate) const;

    // general information about the battery used to read the maximum capacity
    const QString m_sInfoFile;

    // gives current informations about battery (status, capacity, rate)
    const QString m_sStateFile;
};

#endif /* BATTERYLINUX_H */
