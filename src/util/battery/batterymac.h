#pragma once

#include "util/battery/battery.h"

class BatteryMac : public Battery {
    Q_OBJECT
  public:
    BatteryMac(QObject* pParent=nullptr);
    virtual ~BatteryMac();

  protected:
    void read() override;
};
