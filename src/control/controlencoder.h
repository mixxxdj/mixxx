#pragma once

#include "preferences/usersettings.h"
#include "control/controlobject.h"

class ControlEncoder : public ControlObject {
    Q_OBJECT
  public:
    ControlEncoder(const ConfigKey& key, bool bIgnoreNops = true);
};
