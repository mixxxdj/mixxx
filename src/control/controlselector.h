#ifndef CONTROLSELECTOR_H
#define CONTROLSELECTOR_H

#include "preferences/usersettings.h"
#include "control/controlobject.h"

class ControlSelector : public ControlObject {
    Q_OBJECT
  public:
    ControlSelector(ConfigKey key, bool bIgnoreNops=true);
};

#endif
