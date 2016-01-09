#ifndef UTIL_BATTERY_BATTERYMAC_H
#define UTIL_BATTERY_BATTERYMAC_H

#include "util/battery/battery.h"

class BatteryMac : public Battery {
  public:
    BatteryMac(QObject* pParent=nullptr);
    virtual ~BatteryMac();

  protected:
    void read() override;
};

#endif /* UTIL_BATTERY_BATTERYMAC_H */
