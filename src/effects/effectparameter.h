#ifndef EFFECTPARAMETER_H
#define EFFECTPARAMETER_H

#include <QObject>
#include <QVariant>

#include "util.h"
#include "effects/effectmanifestparameter.h"

class Effect;

// An EffectParameter is an instance of an EffectManifestParameter, which is in
// charge of keeping track of the instance values for the default, minimum,
// maximum and value for each Effect's parameter, and validating that they are
// always within acceptable ranges. This class is NOT thread-safe and must only
// be used from the main thread.
class EffectParameter : public QObject {
    Q_OBJECT
  public:
    EffectParameter(Effect* pEffect, const EffectManifestParameter& parameter);
    virtual ~EffectParameter();

    ///////////////////////////////////////////////////////////////////////////
    // Parameter Information
    ///////////////////////////////////////////////////////////////////////////

    const QString name() const;
    const QString description() const;

    ///////////////////////////////////////////////////////////////////////////
    // Value Settings
    ///////////////////////////////////////////////////////////////////////////

    QVariant getValue() const;
    void setValue(QVariant value);

    QVariant getDefault() const;
    void setDefault(QVariant defaultValue);

    QVariant getMinimum() const;
    void setMinimum(QVariant minimum);

    QVariant getMaximum() const;
    void setMaximum(QVariant maximum);

  private:
    QString debugString() const {
        return QString("EffectParameter(%1)").arg(m_parameter.name());
    }

    static bool clampValue(EffectManifestParameter::ValueHint valueHint, QVariant& value,
                           const QVariant& minimum, const QVariant& maximum);
    bool clampValue();
    bool clampDefault();
    bool clampRanges();
    bool checkType(const QVariant& value) const;

    Effect* m_pEffect;
    EffectManifestParameter m_parameter;
    QVariant m_minimum;
    QVariant m_maximum;
    QVariant m_default;
    QVariant m_value;

    DISALLOW_COPY_AND_ASSIGN(EffectParameter);
};


#endif /* EFFECTPARAMETER_H */
