#ifndef UTIL_BATTERY_BATTERYLINUX_H
#define UTIL_BATTERY_BATTERYLINUX_H

#include "util/battery/battery.h"

class BatteryLinux : public Battery {
  public:
    explicit BatteryLinux(QObject* pParent=nullptr);
    ~BatteryLinux() override;

  protected:
    void read() override;

  private:
    void* m_client;
};

#endif /* UTIL_BATTERY_BATTERYLINUX_H */
