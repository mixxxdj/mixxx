#pragma once
#include <QDomElement>

#include "effects/defs.h"
#include "effects/presets/effectparameterpreset.h"

/// EffectPreset is a read-only snapshot of the state of an effect that can be
/// serialized to/deserialized from XML. It is used by EffectChainPreset to
/// save/load chain presets. It is also used by EffectPresetManager to save custom
/// defaults for each effect.
class EffectPreset {
  public:
    EffectPreset();
    EffectPreset(const QDomElement& element);
    EffectPreset(const EffectSlotPointer pEffectSlot);
    EffectPreset(const EffectManifestPointer pManifest);

    const QDomElement toXml(QDomDocument* doc) const;

    const QString& id() const {
        return m_id;
    }

    bool isEmpty() const {
        return m_effectParameterPresets.size() == 0;
    }

    EffectBackendType backendType() const {
        return m_backendType;
    }

    double metaParameter() const {
        return m_dMetaParameter;
    }

    const QList<EffectParameterPreset>& getParameterPresets() const {
        return m_effectParameterPresets;
    }

    /// updates all of the parameters of `this` with the parameters
    /// of `preset`.
    /// The operation is not symmetric:
    /// Parameters which are present on `preset` but not on `this` will
    /// not be added to `this`
    /// Parameters present on `this` but not `preset` will keep their previous
    /// settings
    void updateParametersFrom(const EffectPreset& preset);

  private:
    QString m_id;
    EffectBackendType m_backendType;
    double m_dMetaParameter;

    QList<EffectParameterPreset> m_effectParameterPresets;
};
