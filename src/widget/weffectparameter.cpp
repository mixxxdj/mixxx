#include <QtDebug>

#include "widget/weffectparameter.h"
#include "effects/effectsmanager.h"
#include "widget/effectwidgetutils.h"

WEffectParameter::WEffectParameter(QWidget* pParent, EffectsManager* pEffectsManager)
        : WEffectParameterBase(pParent, pEffectsManager) {
}

void WEffectParameter::setup(const QDomNode& node, const SkinContext& context) {
    WLabel::setup(node, context);
    // EffectWidgetUtils propagates NULLs so this is all safe.
    EffectChainSlotPointer pChainSlot = EffectWidgetUtils::getEffectChainSlotFromNode(
            node, context, m_pEffectsManager);
    EffectSlotPointer pEffectSlot = EffectWidgetUtils::getEffectSlotFromNode(
            node, context, pChainSlot);
    EffectParameterSlotBasePointer pParameterSlot =
            EffectWidgetUtils::getParameterSlotFromNode(
                    node, context, pEffectSlot);
    VERIFY_OR_DEBUG_ASSERT(pParameterSlot) {
        SKIN_WARNING(node, context)
                << "EffectParameter node could not attach to effect parameter";
    }
    setEffectParameterSlot(pParameterSlot);
}
