#ifndef EFFECTMANIFESTPARAMETER_H
#define EFFECTMANIFESTPARAMETER_H

#include <QObject>
#include <QVariant>
#include <QString>
#include <QSharedPointer>
#include <QtDebug>

class EffectManifestParameter;
typedef QSharedPointer<const EffectManifestParameter> EffectManifestParameterPointer;

class EffectManifestParameter : public QObject {
  public:
    enum ValueHint {
        VALUE_UNKNOWN = 0,
        VALUE_BOOLEAN,
        VALUE_INTEGRAL,
        VALUE_FLOAT
    };

    enum ControlHint {
        CONTROL_UNKNOWN = 0,
        CONTROL_KNOB_LINEAR,
        CONTROL_KNOB_LOGARHYTHMIC,
        CONTROL_TOGGLE
    };

    enum SemanticHint {
        SEMANTIC_UNKNOWN = 0,
        SEMANTIC_SAMPLES,
        SEMANTIC_NOTE,
    };

    enum UnitsHint {
        UNITS_UNKNOWN = 0,
        UNITS_TIME,
        UNITS_HERTZ,
        UNITS_SAMPLERATE, // fraction of the samplerate
    };

    EffectManifestParameter(QObject* pParent = NULL);
    virtual ~EffectManifestParameter();

    ////////////////////////////////////////////////////////////////////////////////
    // Parameter Information
    ////////////////////////////////////////////////////////////////////////////////

    virtual const QString id() const;
    virtual void setId(QString id);

    virtual const QString name() const;
    virtual void setName(QString name);

    virtual const QString description() const;
    virtual void setDescription(QString description);

    ////////////////////////////////////////////////////////////////////////////////
    // Usage hints
    ////////////////////////////////////////////////////////////////////////////////

    virtual ControlHint controlHint() const;
    virtual void setControlHint(ControlHint controlHint);

    virtual ValueHint valueHint() const;
    virtual void setValueHint(ValueHint valueHint);

    virtual SemanticHint semanticHint() const;
    virtual void setSemanticHint(SemanticHint semanticHint);

    virtual UnitsHint unitsHint() const;
    virtual void setUnitsHint(UnitsHint unitsHint);

    ////////////////////////////////////////////////////////////////////////////////
    // Value Settings
    ////////////////////////////////////////////////////////////////////////////////

    virtual bool hasDefault() const;
    virtual QVariant getDefault() const;
    virtual void setDefault(QVariant defaultValue);

    virtual bool hasMinimum() const;
    virtual QVariant getMinimum() const;
    virtual void setMinimum(QVariant minimum);

    virtual bool hasMaximum() const;
    virtual QVariant getMaximum() const;
    virtual void setMaximum(QVariant maximum);

  private:
    QString debugString() const {
        return QString("EffectManifestParameter(%1)").arg(m_id);
    }
    QString m_id;
    QString m_name;
    QString m_description;

    ControlHint m_controlHint;
    ValueHint m_valueHint;
    SemanticHint m_semanticHint;
    UnitsHint m_unitsHint;

    QVariant m_default;
    QVariant m_minimum;
    QVariant m_maximum;
};

QDebug operator<<(QDebug dbg, const EffectManifestParameter &parameter);

#endif /* EFFECTMANIFESTPARAMETER_H */
