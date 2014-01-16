#ifndef BATTERYMAC_H
#define BATTERYMAC_H

#include "util/battery/battery.h"

class BatteryMac : public Battery {
    Q_OBJECT
  public:
    BatteryMac(QObject* pParent=NULL);
    virtual ~BatteryMac();

  protected:
    void read();
};

#endif /* BATTERYMAC_H */
