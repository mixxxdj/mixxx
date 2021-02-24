#include "widget/weffectchain.h"

#include <QtDebug>

#include "effects/effectsmanager.h"
#include "moc_weffectchain.cpp"
#include "widget/effectwidgetutils.h"

WEffectChain::WEffectChain(QWidget* pParent, EffectsManager* pEffectsManager)
        : WLabel(pParent),
          m_pEffectsManager(pEffectsManager) {
    chainUpdated();
}

void WEffectChain::setup(const QDomNode& node, const SkinContext& context) {
    WLabel::setup(node, context);
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
        connect(pEffectChainSlot.data(),
                &EffectChainSlot::updated,
                this,
                &WEffectChain::chainUpdated);
        chainUpdated();
    }
}

void WEffectChain::chainUpdated() {
    QString name = EffectsManager::kNoEffectString;
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
