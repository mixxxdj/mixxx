#pragma once

#include "util/battery/battery.h"

class BatteryMac : public Battery {
  public:
    BatteryMac(QObject* pParent=nullptr);
    virtual ~BatteryMac();

  protected:
    void read() override;
};
