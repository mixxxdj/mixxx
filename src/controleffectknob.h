#ifndef CONTROLEFFECTKNOB_H
#define CONTROLEFFECTKNOB_H

#include "controlpotmeter.h"
#include "effects/effectmanifestparameter.h"

class ControlEffectKnob : public ControlPotmeter {
    Q_OBJECT
  public:
    ControlEffectKnob(ConfigKey key,
			EffectKnobParameters parameters = EffectKnobParameters());

    void setBehavior(EffectManifestParameter::ControlHint type,
            double dMinValue, double dMaxValue, double dNeutralParameter);
};

#endif // CONTROLLEFFECTKNOB_H
