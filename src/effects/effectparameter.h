#ifndef EFFECTPARAMETER_H
#define EFFECTPARAMETER_H

#include <QObject>
#include <QVariant>

#include "effects/effectmanifestparameter.h"
#include "effects/effectslot.h"
#include "util/class.h"

class Effect;
class EffectsManager;
class EngineEffect;

// An EffectParameter is a wrapper around EffectManifestParameter that tracks a
// mutable value state and communicates that state to the engine. This class is
// NOT thread-safe and must only be used from the main thread. Separating this
// from the parameterX ControlObjects in EffectParameterSlot allows for decoupling
// the state of the parameters from the ControlObject states, which is required for
// parameter hiding and rearrangement.
class EffectParameter : public QObject {
    Q_OBJECT
  public:
    EffectParameter(EngineEffect* pEngineEffect, EffectsManager* pEffectsManager,
                    int iParameterNumber, EffectManifestParameterPointer pParameter);
    virtual ~EffectParameter();

    EffectManifestParameterPointer manifest() const;

    double getValue() const;
    void setValue(double value);

    void updateEngineState();

  signals:
    void valueChanged(double value);

  private:
    QString debugString() const {
        return QString("EffectParameter(%1)").arg(m_pParameter->name());
    }

    static bool clampValue(double* pValue,
                           const double& minimum, const double& maximum);
    bool clampValue();

    EngineEffect* m_pEngineEffect;
    EffectsManager* m_pEffectsManager;
    int m_iParameterNumber;
    EffectManifestParameterPointer m_pParameter;
    double m_value;

    DISALLOW_COPY_AND_ASSIGN(EffectParameter);
};


#endif /* EFFECTPARAMETER_H */
