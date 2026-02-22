#pragma once

#include "control/controlpotmeter.h"
#include "effects/backends/effectmanifestparameter.h"

class ControlEffectKnob : public ControlPotmeter {
    Q_OBJECT
  public:
    ControlEffectKnob(const ConfigKey& key, double dMinValue = 0.0, double dMaxValue = 1.0);

    void setBehaviour(EffectManifestParameter::ValueScaler type,
            double dMinValue,
            double dMaxValue);
};
