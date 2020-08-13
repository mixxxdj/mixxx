#pragma once

#include "preferences/usersettings.h"
#include "control/controlobject.h"

class ControlTTRotary : public ControlObject {
    Q_OBJECT
  public:
    ControlTTRotary(ConfigKey key);
};
