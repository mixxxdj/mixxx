#pragma once
#include <QDomElement>

#include "effects/defs.h"
#include "effects/effectchainmixmode.h"

class EffectChain;

/// EffectChainPreset is a read-only snapshot of the state of an EffectChain
/// that can be serialized to/deserialized from XML. It is used by
/// EffectChainPresetManager to easily save/load user-defined chain presets
/// as well as save the state of StandardEffectChains when Mixxx shuts down and restarts.
class EffectChainPreset {
  public:
    EffectChainPreset();
    EffectChainPreset(const QDomElement& chainElement);
    EffectChainPreset(const EffectChain* pChain);
    /// make a chain preset with just one effect
    EffectChainPreset(EffectManifestPointer pManifest);
    EffectChainPreset(EffectPresetPointer pEffectPreset);
    ~EffectChainPreset();

    const QDomElement toXml(QDomDocument* doc) const;

    bool isEmpty() const {
        return m_effectPresets.size() == 0;
    }

    /// This is the only exception to EffectChainPreset being read-only.
    void setName(const QString& newName) {
        m_name = newName;
    }
    const QString& name() const {
        return m_name;
    }
    EffectChainMixMode::Type mixMode() const {
        return m_mixMode;
    }
    double superKnob() const {
        return m_dSuper;
    }
    void setReadOnly() {
        m_readOnly = true;
    }
    bool isReadOnly() const {
        return m_readOnly;
    }

    const QList<EffectPresetPointer>& effectPresets() const {
        return m_effectPresets;
    }

  private:
    QString m_name;
    EffectChainMixMode::Type m_mixMode;
    double m_dSuper;
    QList<EffectPresetPointer> m_effectPresets;
    bool m_readOnly;
};
