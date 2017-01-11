#ifndef EFFECTPARAMETER_H
#define EFFECTPARAMETER_H

#include <QObject>
#include <QVariant>

#include "effects/effectmanifestparameter.h"
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
    EffectParameter(Effect* pEffect, EffectsManager* pEffectsManager,
                    int iParameterNumber, const EffectManifestParameter& parameter);
    virtual ~EffectParameter();

    void addToEngine();
    void removeFromEngine();

    ///////////////////////////////////////////////////////////////////////////
    // Parameter Information
    ///////////////////////////////////////////////////////////////////////////

    const EffectManifestParameter& manifest() const;
    const QString id() const;
    const QString name() const;
    const QString shortName() const;
    const QString description() const;

    ///////////////////////////////////////////////////////////////////////////
    // Value Settings
    ///////////////////////////////////////////////////////////////////////////

    EffectManifestParameter::LinkType getDefaultLinkType() const;
    EffectManifestParameter::LinkInversion getDefaultLinkInversion() const;
    double getNeutralPointOnScale() const;

    double getValue() const;

    void setValue(double value);

    double getDefault() const;
    void setDefault(double defaultValue);

    double getMinimum() const;
    void setMinimum(double minimum);

    double getMaximum() const;
    void setMaximum(double maximum);

    EffectManifestParameter::ControlHint getControlHint() const;

    void updateEngineState();

  signals:
    void valueChanged(double value);

  private:
    QString debugString() const {
        return QString("EffectParameter(%1)").arg(m_parameter.name());
    }

    static bool clampValue(double* pValue,
                           const double& minimum, const double& maximum);
    bool clampValue();
    bool clampDefault();
    bool clampRanges();

    Effect* m_pEffect;
    EffectsManager* m_pEffectsManager;
    int m_iParameterNumber;
    EffectManifestParameter m_parameter;
    double m_minimum;
    double m_maximum;
    double m_default;
    double m_value;
    bool m_bAddedToEngine;

    DISALLOW_COPY_AND_ASSIGN(EffectParameter);
};


#endif /* EFFECTPARAMETER_H */
