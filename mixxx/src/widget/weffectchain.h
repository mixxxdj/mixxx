#pragma once

#include "effects/defs.h"
#include "widget/wlabel.h"

class EffectsManager;

class WEffectChain : public WLabel {
    Q_OBJECT
  public:
    WEffectChain(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context) override;

  private slots:
    void chainPresetChanged(const QString& newName);

  private:
    // Set the EffectChain that should be monitored by this WEffectChain
    void setEffectChain(EffectChainPointer pEffectChain);

    EffectsManager* m_pEffectsManager;
    EffectChainPointer m_pEffectChain;
};
