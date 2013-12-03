#ifndef EFFECTMANIFESTPARAMETER_H
#define EFFECTMANIFESTPARAMETER_H

#include <QVariant>
#include <QString>
#include <QtDebug>

class EffectManifestParameter {
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
        CONTROL_KNOB_LOGARITHMIC,
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

    EffectManifestParameter() { }
    virtual ~EffectManifestParameter() {
        qDebug() << debugString() << "destroyed";
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Parameter Information
    ////////////////////////////////////////////////////////////////////////////////

    virtual const QString& id() const {
        return m_id;
    }
    virtual void setId(const QString& id) {
        m_id = id;
    }

    virtual const QString& name() const {
        return m_name;
    }
    virtual void setName(const QString& name) {
        m_name = name;
    }

    virtual const QString& description() const {
        return m_description;
    }
    virtual void setDescription(const QString& description) {
        m_description = description;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Usage hints
    ////////////////////////////////////////////////////////////////////////////////

    virtual ControlHint controlHint() const {
        return m_controlHint;
    }
    virtual void setControlHint(ControlHint controlHint) {
        m_controlHint = controlHint;
    }

    virtual ValueHint valueHint() const {
        return m_valueHint;
    }
    virtual void setValueHint(ValueHint valueHint) {
        m_valueHint = valueHint;
    }

    virtual SemanticHint semanticHint() const {
        return m_semanticHint;
    }
    virtual void setSemanticHint(SemanticHint semanticHint) {
        m_semanticHint = semanticHint;
    }

    virtual UnitsHint unitsHint() const {
        return m_unitsHint;
    }
    virtual void setUnitsHint(UnitsHint unitsHint) {
        m_unitsHint = unitsHint;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Value Settings
    ////////////////////////////////////////////////////////////////////////////////

    virtual bool hasDefault() const {
        return m_default.isValid();
    }
    virtual const QVariant& getDefault() const {
        return m_default;
    }
    virtual void setDefault(const QVariant& defaultValue) {
        m_default = defaultValue;
    }

    virtual bool hasMinimum() const {
        return m_minimum.isValid();
    }
    virtual const QVariant& getMinimum() const {
        return m_minimum;
    }
    virtual void setMinimum(const QVariant& minimum) {
        m_minimum = minimum;
    }

    virtual bool hasMaximum() const {
        return m_maximum.isValid();
    }
    virtual const QVariant& getMaximum() const {
        return m_maximum;
    }
    virtual void setMaximum(const QVariant& maximum) {
        m_maximum = maximum;
    }

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

QDebug operator<<(QDebug dbg, const EffectManifestParameter& parameter);

#endif /* EFFECTMANIFESTPARAMETER_H */
