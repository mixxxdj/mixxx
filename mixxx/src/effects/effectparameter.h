#ifndef EFFECTPARAMETER_H
#define EFFECTPARAMETER_H

#include <QVariant>
#include <QString>

class EffectParameter {
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

    EffectParameter();
    virtual ~EffectParameter();

    ////////////////////////////////////////////////////////////////////////////////
    // Parameter Information
    ////////////////////////////////////////////////////////////////////////////////

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

#endif /* EFFECTPARAMETER_H */
