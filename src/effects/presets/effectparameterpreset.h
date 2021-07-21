#pragma once

#include <QDomElement>

#include "effects/backends/effectmanifestparameter.h"
#include "effects/defs.h"

/// EffectParameterPreset is a read-only snapshot of the state of an effect
/// parameter that can be serialized to/deserialized from XML. It is only used
/// as a component of an EffectPreset; never on its own.
class EffectParameterPreset {
  public:
    EffectParameterPreset();
    EffectParameterPreset(const QDomElement& parameterElement);
    /// The hidden state is stored in the EffectSlot, not the EffectParameter
    EffectParameterPreset(const EffectParameterPointer pParameter, bool hidden);
    /// read defaults from manifest
    EffectParameterPreset(const EffectManifestParameterPointer pManifestParameter);
    ~EffectParameterPreset();

    const QDomElement toXml(QDomDocument* doc) const;

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
