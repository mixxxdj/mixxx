#pragma once
#include <QDomElement>

#include "effects/defs.h"
#include "effects/presets/effectpreset.h"

class EffectChainSlot;

class EffectChainPreset {
  public:
    EffectChainPreset();
    EffectChainPreset(const QDomElement& chainElement);
    EffectChainPreset(const EffectChainSlot* pChain);
    ~EffectChainPreset();

    const QDomElement toXml(QDomDocument* doc) const;

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
