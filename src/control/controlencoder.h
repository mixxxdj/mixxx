#ifndef CONTROLENCODER_H
#define CONTROLENCODER_H

#include "preferences/usersettings.h"
#include "control/controlobject.h"

class ControlEncoder : public ControlObject {
    Q_OBJECT
  public:
    ControlEncoder(ConfigKey key, bool bIgnoreNops=true);
};

#endif
