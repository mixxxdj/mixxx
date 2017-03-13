#include <QtDebug>

#include "widget/weffect.h"

#include "effects/effectsmanager.h"
#include "widget/effectwidgetutils.h"

WEffect::WEffect(QWidget* pParent, EffectsManager* pEffectsManager)
        : WLabel(pParent),
          m_pEffectsManager(pEffectsManager) {
    effectUpdated();
}

WEffect::~WEffect() {
}

void WEffect::setup(QDomNode node, const SkinContext& context) {
    // EffectWidgetUtils propagates NULLs so this is all safe.
    EffectRackPointer pRack = EffectWidgetUtils::getEffectRackFromNode(
            node, context, m_pEffectsManager);
    EffectChainSlotPointer pChainSlot = EffectWidgetUtils::getEffectChainSlotFromNode(
            node, context, pRack);
    EffectSlotPointer pEffectSlot = EffectWidgetUtils::getEffectSlotFromNode(
            node, context, pChainSlot);
    if (pEffectSlot) {
        setEffectSlot(pEffectSlot);
    } else {
        SKIN_WARNING(node, context)
                << "EffectName node could not attach to effect slot.";
    }
}

void WEffect::setEffectSlot(EffectSlotPointer pEffectSlot) {
    if (pEffectSlot) {
        m_pEffectSlot = pEffectSlot;
        connect(pEffectSlot.data(), SIGNAL(updated()),
                this, SLOT(effectUpdated()));
        effectUpdated();
    }
}

void WEffect::effectUpdated() {
    QString name;
    QString description;
    if (m_pEffectSlot) {
        EffectPointer pEffect = m_pEffectSlot->getEffect();
        if (pEffect) {
            const EffectManifest& manifest = pEffect->getManifest();
            name = manifest.name();
            description = manifest.description();
        }
    } else {
        name = tr("None");
        description = tr("No effect loaded.");
    }
    setText(name);
    setBaseTooltip(description);
}
