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

    QVariant getValue() const {
        return m_value.getValue();
    }
    void setValue(const QVariant& value) {
        m_value.setValue(value);
    }

  private:
    EffectManifestParameter m_parameter;
    ControlValueAtomic<QVariant> m_value;
    DISALLOW_COPY_AND_ASSIGN(EngineEffectParameter);
};

#endif /* ENGINEEFFECTPARAMETER_H */
