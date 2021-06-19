#include "widget/weffectbuttonparameter.h"

#include <QtDebug>

#include "effects/effectsmanager.h"
#include "moc_weffectbuttonparameter.cpp"
#include "widget/effectwidgetutils.h"

WEffectButtonParameter::WEffectButtonParameter(QWidget* pParent, EffectsManager* pEffectsManager)
        : WEffectParameterBase(pParent, pEffectsManager) {
}

void WEffectButtonParameter::setup(const QDomNode& node, const SkinContext& context) {
    WLabel::setup(node, context);
    // EffectWidgetUtils propagates NULLs so this is all safe.
    EffectRackPointer pRack = EffectWidgetUtils::getEffectRackFromNode(
            node, context, m_pEffectsManager);
    EffectChainSlotPointer pChainSlot = EffectWidgetUtils::getEffectChainSlotFromNode(
            node, context, pRack);
    EffectSlotPointer pEffectSlot = EffectWidgetUtils::getEffectSlotFromNode(
            node, context, pChainSlot);
    EffectParameterSlotBasePointer pParameterSlot =
            EffectWidgetUtils::getButtonParameterSlotFromNode(
                    node, context, pEffectSlot);
    if (pParameterSlot) {
        setEffectParameterSlot(pParameterSlot);
    } else {
        SKIN_WARNING(node, context)
                << "EffectButtonParameter node could not attach to effect parameter";
    }
}
