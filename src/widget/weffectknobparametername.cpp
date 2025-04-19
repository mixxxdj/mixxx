#include "widget/weffectknobparametername.h"

#include "moc_weffectknobparametername.cpp"
#include "widget/effectwidgetutils.h"
#include "widget/weffectparameterknobcomposed.h" // Included the knob widget header to allow linking with the label.

// Helper function to get a sibling widget of a specific type (e.g., knob <-> label)
// within the same parent container. This allows the label and knob to find each other.
template<typename T>
T* getSiblingOfType(QWidget* pWidget) {
    if (!pWidget || !pWidget->parentWidget()) {
        return nullptr;
    }

    const auto children = pWidget->parentWidget()->findChildren<T*>();
    for (T* pChild : children) {
        if (pChild != pWidget) {
            return pChild;
        }
    }
    return nullptr;
}

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

    // Our new connection to the knob.
    // Attempt to find the sibling knob associated with this label, and connect them.
    // This allows the knob to access the label for tooltip fallback text.
    WEffectParameterKnobComposed* pKnob = getSiblingOfType<WEffectParameterKnobComposed>(this);
    if (pKnob) {
        pKnob->setLabelPointer(this);
    }
}
