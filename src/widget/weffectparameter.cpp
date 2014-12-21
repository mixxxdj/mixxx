#include <QtDebug>

#include "widget/weffectparameter.h"
#include "effects/effectsmanager.h"
#include "widget/effectwidgetutils.h"

WEffectParameter::WEffectParameter(QWidget* pParent, EffectsManager* pEffectsManager)
        : WEffectParameterBase(pParent, pEffectsManager) {
}

WEffectParameter::~WEffectParameter() {
}

void WEffectParameter::setup(QDomNode node, const SkinContext& context) {
    // EffectWidgetUtils propagates NULLs so this is all safe.
    EffectRackPointer pRack = EffectWidgetUtils::getEffectRackFromNode(
            node, context, m_pEffectsManager);
    EffectChainSlotPointer pChainSlot = EffectWidgetUtils::getEffectChainSlotFromNode(
            node, context, pRack);
    EffectSlotPointer pEffectSlot = EffectWidgetUtils::getEffectSlotFromNode(
            node, context, pChainSlot);
    EffectParameterSlotBasePointer pParameterSlot =
            EffectWidgetUtils::getParameterSlotFromNode(
                    node, context, pEffectSlot);
    if (pParameterSlot) {
        setEffectParameterSlot(pParameterSlot);
    } else {
        SKIN_WARNING(node, context)
                << "EffectParameter node could not attach to effect parameter";
    }
}
