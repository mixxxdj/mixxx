#include "widget/weffect.h"

#include <QtDebug>

#include "effects/effectsmanager.h"
#include "moc_weffect.cpp"
#include "widget/effectwidgetutils.h"

WEffect::WEffect(QWidget* pParent, EffectsManager* pEffectsManager)
        : WLabel(pParent),
          m_pEffectsManager(pEffectsManager) {
    effectUpdated();
}

void WEffect::setup(const QDomNode& node, const SkinContext& context) {
    WLabel::setup(node, context);
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
        connect(pEffectSlot.data(), &EffectSlot::updated, this, &WEffect::effectUpdated);
        effectUpdated();
    }
}

void WEffect::effectUpdated() {
    QString name;
    QString description;
    if (m_pEffectSlot) {
        EffectPointer pEffect = m_pEffectSlot->getEffect();
        if (pEffect) {
            EffectManifestPointer pManifest = pEffect->getManifest();
            name = pManifest->displayName();
            //: %1 = effect name; %2 = effect description
            description = tr("%1: %2").arg(pManifest->name(), pManifest->description());
        }
    } else {
        name = tr("None");
        description = tr("No effect loaded.");
    }
    setText(name);
    setBaseTooltip(description);
}
