#include <QtDebug>

#include "widget/weffectchain.h"
#include "effects/effectsmanager.h"
#include "widget/effectwidgetutils.h"

WEffectChain::WEffectChain(QWidget* pParent, EffectsManager* pEffectsManager)
        : WLabel(pParent),
          m_pEffectsManager(pEffectsManager) {
    chainUpdated();
}

WEffectChain::~WEffectChain() {
}

void WEffectChain::setup(QDomNode node, const SkinContext& context) {
    // EffectWidgetUtils propagates NULLs so this is all safe.
    EffectRackPointer pRack = EffectWidgetUtils::getEffectRackFromNode(
            node, context, m_pEffectsManager);
    EffectChainSlotPointer pChainSlot = EffectWidgetUtils::getEffectChainSlotFromNode(
            node, context, pRack);
    if (pChainSlot) {
        setEffectChainSlot(pChainSlot);
    } else {
        SKIN_WARNING(node, context)
                << "EffectChain node could not attach to effect chain slot.";
    }
}

void WEffectChain::setEffectChainSlot(EffectChainSlotPointer pEffectChainSlot) {
    if (pEffectChainSlot) {
        m_pEffectChainSlot = pEffectChainSlot;
        connect(pEffectChainSlot.data(), SIGNAL(updated()),
                this, SLOT(chainUpdated()));
        chainUpdated();
    }
}

void WEffectChain::chainUpdated() {
    QString name = tr("None");
    QString description = tr("No effect chain loaded.");
    if (m_pEffectChainSlot) {
        EffectChainPointer pChain = m_pEffectChainSlot->getEffectChain();
        if (pChain) {
            name = pChain->name();
            description = pChain->description();
        }
    }
    setText(name);
    setBaseTooltip(description);
}
