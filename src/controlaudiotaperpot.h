
#ifndef CONTROLAUDIOTAPERPOT_H
#define CONTROLAUDIOTAPERPOT_H

#include "controlpotmeter.h"
#include "configobject.h"

class ControlAudioTaperPot : public ControlPotmeter {
    Q_OBJECT
  public:
    ControlAudioTaperPot(ConfigKey key, double minDB, double maxDB, double neutralParameter);
};

#endif // CONTROLAUDIOTAPERPOT_H
