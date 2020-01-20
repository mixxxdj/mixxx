#pragma once

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
