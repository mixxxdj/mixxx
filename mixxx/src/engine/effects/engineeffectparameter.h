#ifndef ENGINEEFFECTPARAMETER_H
#define ENGINEEFFECTPARAMETER_H

#include <QString>
#include <QVariant>

#include "util.h"
#include "effects/effectmanifestparameter.h"

class EngineEffectParameter {
  public:
    EngineEffectParameter(const EffectManifestParameter& parameter) : m_parameter(parameter) {
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

    const QVariant& getValue() const {
        return m_value;
    }
    void setValue(const QVariant& value) {
        m_value = value;
    }
    void setValue(const double& value) {
        m_value = value;
    }

  private:
    EffectManifestParameter m_parameter;
    QVariant m_value;
    DISALLOW_COPY_AND_ASSIGN(EngineEffectParameter);
};

#endif /* ENGINEEFFECTPARAMETER_H */
