#ifndef ENGINEEFFECTPARAMETER_H
#define ENGINEEFFECTPARAMETER_H

#include <QString>
#include <QVariant>

#include "control/controlvalue.h"
#include "util.h"
#include "effects/effectmanifestparameter.h"

class EngineEffectParameter {
  public:
    EngineEffectParameter(const EffectManifestParameter& parameter)
            : m_parameter(parameter) {
    }
    virtual ~EngineEffectParameter() { }

    ///////////////////////////////////////////////////////////////////////////
    // Parameter Information
    ///////////////////////////////////////////////////////////////////////////

    const QString& id() const {
        return m_parameter.id();
    }
    const QString& name() const {
        return m_parameter.name();
    }
    const QString& description() const {
        return m_parameter.description();
    }

    ///////////////////////////////////////////////////////////////////////////
    // Value Settings
    ///////////////////////////////////////////////////////////////////////////

    const QVariant& value() const {
        return m_value;
    }
    void setValue(const QVariant& value) {
        m_value = value;
    }

    const QVariant& defaultValue() const {
        return m_default_value;
    }
    void setDefaultValue(const QVariant& default_value) {
        m_default_value = default_value;
    }

    const QVariant& minimum() const {
        return m_minimum;
    }
    void setMinimum(const QVariant& minimum) {
        m_minimum = minimum;
    }

    const QVariant& maximum() const {
        return m_maximum;
    }
    void setMaximum(const QVariant& maximum) {
        m_maximum = maximum;
    }

  private:
    EffectManifestParameter m_parameter;
    QVariant m_value;
    QVariant m_default_value;
    QVariant m_minimum;
    QVariant m_maximum;

    DISALLOW_COPY_AND_ASSIGN(EngineEffectParameter);
};

#endif /* ENGINEEFFECTPARAMETER_H */
