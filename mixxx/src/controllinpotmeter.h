#ifndef CONTROLLINPOTMETER_H
#define CONTROLLINPOTMETER_H

#include "controlpotmeter.h"

class ControlLinPotmeter : public ControlPotmeter {
    Q_OBJECT
  public:
    ControlLinPotmeter(ConfigKey key, double dMinValue=0.0, double dMaxValue=1.0);
};

#endif // CONTROLLINPOTMETER_H
