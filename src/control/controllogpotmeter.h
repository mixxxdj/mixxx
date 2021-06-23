#pragma once

#include "control/controlpotmeter.h"
#include "preferences/usersettings.h"

class ControlLogpotmeter : public ControlPotmeter {
    Q_OBJECT
  public:
    ControlLogpotmeter(const ConfigKey& key, double dMaxValue, double minDB);
};
