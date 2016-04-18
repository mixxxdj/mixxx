#ifndef UTIL_BATTERY_BATTERYLINUX_H
#define UTIL_BATTERY_BATTERYLINUX_H

#include "util/battery/battery.h"

class BatteryLinux : public Battery {
  public:
    BatteryLinux(QObject* pParent=nullptr);
    virtual ~BatteryLinux();

  protected:
    void read() override;
};

#endif /* UTIL_BATTERY_BATTERYLINUX_H */
