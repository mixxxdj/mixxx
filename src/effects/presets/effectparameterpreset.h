#pragma once

#include <QDomElement>

#include "effects/defs.h"
#include "effects/effectmanifestparameter.h"

class EffectParameterPreset {
  public:
    EffectParameterPreset();
    EffectParameterPreset(const QDomElement& parameterElement);
    EffectParameterPreset(const EffectManifestParameterPointer pManifestParameter);
    ~EffectParameterPreset();

    bool isNull() const {
        return m_id.isEmpty();
    }

    double value() const {
        return m_dValue;
    }
    const QString& id() const {
        return m_id;
    }
    EffectManifestParameter::LinkType linkType() const {
        return m_linkType;
    }
    EffectManifestParameter::LinkInversion linkInverted() const {
        return m_linkInversion;
    }
    bool hidden() const {
        return m_bHidden;
    }

  private:
    double m_dValue;
    QString m_id;
    EffectManifestParameter::LinkType m_linkType;
    EffectManifestParameter::LinkInversion m_linkInversion;
    bool m_bHidden;
};
