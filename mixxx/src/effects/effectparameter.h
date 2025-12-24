#pragma once

#include "effects/backends/effectmanifestparameter.h"
#include "util/class.h"

class EngineEffect;
class EffectParameterPreset;

/// An EffectParameter is a main thread representation of the state of an
/// EngineEffectParameter. EffectParameter tracks a mutable value state and
/// communicates that state to the engine. Separating this from the parameterX
/// ControlObjects in EffectParameterSlotBase allows for decoupling the state of
/// the parameters from the ControlObject states, which is required for
/// EffectSlot to do parameter hiding and rearrangement. EffectParameter is
/// responsible for manipulating the value of knob parameters when the metaknob
/// of the EffectSlot is changed (button parameters cannot be linked to the
/// metaknob). For EffectParameter, there is no difference between knobs and
/// buttons; only EffectSlot and
/// EffectKnobParameterSlot/EffectButtonParameterSlot are responsible for taking
/// care of that difference.
class EffectParameter {
  public:
    EffectParameter(EngineEffect* pEngineEffect,
            EffectsMessengerPointer pEffectsMessenger,
            EffectManifestParameterPointer pParameterManifest,
            const EffectParameterPreset& preset);
    virtual ~EffectParameter();

    EffectManifestParameterPointer manifest() const;

    void setLinkType(EffectManifestParameter::LinkType type) {
        m_linkType = type;
    }
    EffectManifestParameter::LinkType linkType() const {
        return m_linkType;
    }
    void setLinkInversion(EffectManifestParameter::LinkInversion inversion) {
        m_linkInversion = inversion;
    }
    EffectManifestParameter::LinkInversion linkInversion() const {
        return m_linkInversion;
    }

    double getValue() const;
    void setValue(double value);

    void updateEngineState();

  private:
    QString debugString() const {
        return QString("EffectParameter(%1)").arg(m_pParameterManifest->name());
    }

    static bool clampValue(double* pValue,
            const double& minimum,
            const double& maximum);
    bool clampValue();

    EngineEffect* m_pEngineEffect;
    EffectsMessengerPointer m_pMessenger;
    EffectManifestParameterPointer m_pParameterManifest;
    double m_value;
    // Hidden parameters cannot be linked to the metaknob, but EffectParameter
    // needs to maintain their state in case they are loaded into an EffectParameterSlot.
    EffectManifestParameter::LinkType m_linkType;
    EffectManifestParameter::LinkInversion m_linkInversion;

    DISALLOW_COPY_AND_ASSIGN(EffectParameter);
};
