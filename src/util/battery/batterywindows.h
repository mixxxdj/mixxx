#ifndef UTIL_BATTERY_BATTERYWINDOWS_H
#define UTIL_BATTERY_BATTERYWINDOWS_H

#include "util/battery/battery.h"

class BatteryWindows : public Battery {
  public:
    BatteryWindows(QObject* pParent=nullptr);
    virtual ~BatteryWindows();

  protected:
    void read() override;
};

#endif /* UTIL_BATTERY_BATTERYWINDOWS_H */
