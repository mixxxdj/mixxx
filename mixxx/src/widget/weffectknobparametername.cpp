#include "widget/weffectknobparametername.h"

#include "moc_weffectknobparametername.cpp"
#include "widget/effectwidgetutils.h"

WEffectKnobParameterName::WEffectKnobParameterName(
        QWidget* pParent, EffectsManager* pEffectsManager)
        : WEffectParameterNameBase(pParent, pEffectsManager) {
}

void WEffectKnobParameterName::setup(const QDomNode& node, const SkinContext& context) {
    WLabel::setup(node, context);
    // EffectWidgetUtils propagates NULLs so this is all safe.
    EffectChainPointer pChainSlot = EffectWidgetUtils::getEffectChainFromNode(
            node, context, m_pEffectsManager);
    m_pEffectSlot = EffectWidgetUtils::getEffectSlotFromNode(
            node, context, pChainSlot);
    m_pParameterSlot =
            EffectWidgetUtils::getParameterSlotFromNode(
                    node, context, m_pEffectSlot);
    VERIFY_OR_DEBUG_ASSERT(m_pParameterSlot) {
        SKIN_WARNING(node,
                context,
                QStringLiteral("EffectParameter node could not attach to "
                               "effect parameter"));
    }
    setEffectParameterSlot(m_pParameterSlot);
}
