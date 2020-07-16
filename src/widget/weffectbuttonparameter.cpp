#include <QtDebug>

#include "widget/weffectbuttonparameter.h"
#include "effects/effectsmanager.h"
#include "widget/effectwidgetutils.h"

WEffectButtonParameter::WEffectButtonParameter(QWidget* pParent, EffectsManager* pEffectsManager)
        : WEffectParameterBase(pParent, pEffectsManager) {
}

void WEffectButtonParameter::setup(const QDomNode& node, const SkinContext& context) {
    WLabel::setup(node, context);
    // EffectWidgetUtils propagates NULLs so this is all safe.
    EffectChainSlotPointer pChainSlot = EffectWidgetUtils::getEffectChainSlotFromNode(
            node, context, m_pEffectsManager);
    EffectSlotPointer pEffectSlot = EffectWidgetUtils::getEffectSlotFromNode(
            node, context, pChainSlot);
    EffectParameterSlotBasePointer pParameterSlot =
            EffectWidgetUtils::getButtonParameterSlotFromNode(
                    node, context, pEffectSlot);
    VERIFY_OR_DEBUG_ASSERT(pParameterSlot) {
        SKIN_WARNING(node, context)
                << "EffectButtonParameter node could not attach to effect parameter";
    }
    setEffectParameterSlot(pParameterSlot);
}
