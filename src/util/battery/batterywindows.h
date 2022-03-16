#pragma once

#include "util/battery/battery.h"

class BatteryWindows : public Battery {
    Q_OBJECT
  public:
    BatteryWindows(QObject* pParent=nullptr);
    ~BatteryWindows() override = default;

  protected:
    void read() override;
};
