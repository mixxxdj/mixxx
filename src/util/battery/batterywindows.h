#ifndef BATTERYWINDOWS_H
#define BATTERYWINDOWS_H

#include "util/battery/battery.h"

class BatteryWindows : public Battery {
    Q_OBJECT
  public:
    BatteryWindows(QObject* pParent=NULL);
    virtual ~BatteryWindows();

  protected:
    void read();
};

#endif /* BATTERYWINDOWS_H */
