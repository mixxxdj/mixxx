#ifndef EFFECTMANIFEST_H
#define EFFECTMANIFEST_H

#include "util.h"

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

    // Information
    virtual const QString name() const = 0;
    virtual const QString& description() const = 0;

    // Usage hints
    virtual ControlHint controlHint() const = 0;
    virtual ValueHint valueHint() const = 0;
    virtual SemanticHint semanticHint() const = 0;
    virtual UnitsHint unitsHint() const = 0;

    // Value settings
    virtual bool hasDefault() const = 0;
    virtual QVariant getDefault() const = 0;
    virtual bool hasMinimum() const = 0;
    virtual QVariant getMinimum() const = 0;
    virtual bool hasMaximum() const = 0;
    virtual QVariant getMaximum() const = 0;

  private:
    DISALLOW_COPY_AND_ASSIGN(EffectParameter);
};

class EffectManifest {
  public:
    virtual const QString& name() const = 0;
    virtual const QString& author() const = 0;
    virtual const QString& version() const = 0;
    virtual const QString& description() const = 0;
    virtual const QList<EffectParameter> parameters() const = 0;
  private:
    DISALLOW_COPY_AND_ASSIGN(EffectManifest);
};

#endif /* EFFECTMANIFEST_H */
