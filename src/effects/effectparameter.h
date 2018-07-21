#ifndef EFFECTPARAMETER_H
#define EFFECTPARAMETER_H

#include <QObject>
#include <QVariant>

#include "effects/effectmanifestparameter.h"
#include "effects/effectslot.h"
#include "util/class.h"

class Effect;
class EffectsManager;

// An EffectParameter is an instance of an EffectManifestParameter, which is in
// charge of keeping track of the instance values for the default, minimum,
// maximum and value for each Effect's parameter, and validating that they are
// always within acceptable ranges. This class is NOT thread-safe and must only
// be used from the main thread.
class EffectParameter : public QObject {
    Q_OBJECT
  public:
    EffectParameter(EffectSlot* pEffectSlot, EffectsManager* pEffectsManager,
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

    EffectSlot* m_pEffectSlot;
    EffectsManager* m_pEffectsManager;
    int m_iParameterNumber;
    EffectManifestParameterPointer m_pParameter;
    double m_value;

    DISALLOW_COPY_AND_ASSIGN(EffectParameter);
};


#endif /* EFFECTPARAMETER_H */
