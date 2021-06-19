#pragma once

#include <QWidget>
#include <QLabel>
#include <QDomNode>

#include "effects/effectchainslot.h"
#include "widget/wlabel.h"
#include "skin/legacy/skincontext.h"

class EffectsManager;

class WEffectChain : public WLabel {
    Q_OBJECT
  public:
    WEffectChain(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context) override;

  private slots:
    void chainUpdated();

  private:
    // Set the EffectChain that should be monitored by this WEffectChain
    void setEffectChainSlot(EffectChainSlotPointer pEffectChainSlot);

    EffectsManager* m_pEffectsManager;
    EffectChainSlotPointer m_pEffectChainSlot;
};
