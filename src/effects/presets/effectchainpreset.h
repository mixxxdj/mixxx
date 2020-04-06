#pragma once
#include <QDomElement>

#include "effects/defs.h"
#include "effects/presets/effectpreset.h"

class EffectChainSlot;

// EffectChainPreset is a read-only snapshot of the state of an effect chain
// that can be serialized to/deserialized from XML. It is used by EffectsManager
// to easily save/load user-defined chain presets as well as save the state of
// loaded effects when Mixxx shuts down and restarts.
class EffectChainPreset {
  public:
    EffectChainPreset();
    EffectChainPreset(const QDomElement& chainElement);
    EffectChainPreset(const EffectChainSlot* pChain);
    ~EffectChainPreset();

    const QDomElement toXml(QDomDocument* doc) const;

    bool isEmpty() const {
        return m_effectPresets.size() == 0;
    }

    // This is the only exception to EffectChainPreset being read-only.
    void setName(const QString& newName) {
        m_name = newName;
    }
    const QString& name() const {
        return m_name;
    }
    EffectChainMixMode mixMode() const {
        return m_mixMode;
    }
    double superKnob() const {
        return m_dSuper;
    }

    const QList<EffectPresetPointer>& effectPresets() const {
        return m_effectPresets;
    }

  private:
    QString m_id;
    QString m_name;
    QString m_description;
    double m_dSuper;
    EffectChainMixMode m_mixMode;
    QList<EffectPresetPointer> m_effectPresets;
};
