#ifndef WEFFECTCHAIN_H
#define WEFFECTCHAIN_H

#include <QWidget>
#include <QLabel>
#include <QDomNode>

#include "effects/effectchainslot.h"
#include "widget/wlabel.h"
#include "skin/skincontext.h"

class EffectsManager;

class WEffectChain : public WLabel {
    Q_OBJECT
  public:
    WEffectChain(QWidget* pParent, EffectsManager* pEffectsManager);
    virtual ~WEffectChain();

    void setup(QDomNode node, const SkinContext& context);

  private slots:
    void chainUpdated();

  private:
    // Set the EffectChain that should be monitored by this WEffectChain
    void setEffectChainSlot(EffectChainSlotPointer pEffectChainSlot);

    EffectsManager* m_pEffectsManager;
    EffectChainSlotPointer m_pEffectChainSlot;
};

#endif /* WEFFECTCHAIN_H */
