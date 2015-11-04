#ifndef ENGINEEFFECTPARAMETER_H
#define ENGINEEFFECTPARAMETER_H

#include <QString>
#include <QVariant>

#include "util.h"
#include "effects/effectmanifestparameter.h"

class EngineEffectParameter {
  public:
    EngineEffectParameter(const EffectManifestParameter& parameter)
            : m_parameter(parameter) {
        // NOTE(rryan): This is just to set the parameter values to sane
        // defaults. When an effect is loaded into the engine it is supposed to
        // immediately send a parameter update. Some effects will go crazy if
        // their parameters are not within the manifest's minimum/maximum bounds
        // so just to be safe we read the min/max/default from the manifest
        // here.
        if (m_parameter.hasMinimum()) {
            m_minimum = m_parameter.getMinimum();
        }
        if (m_parameter.hasMaximum()) {
            m_maximum = m_parameter.getMaximum();
        }
        if (m_parameter.hasDefault()) {
            m_default_value = m_parameter.getDefault();
        }
        m_value = m_default_value;
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
