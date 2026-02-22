#include "widget/weffectparameterknob.h"

#include "effects/effectparameterslotbase.h"
#include "moc_weffectparameterknob.cpp"
#include "widget/effectwidgetutils.h"

void WEffectParameterKnob::setup(const QDomNode& node, const SkinContext& context) {
    WKnob::setup(node, context);
    auto pChainSlot = EffectWidgetUtils::getEffectChainFromNode(
            node, context, m_pEffectsManager);
    auto pEffectSlot =
            EffectWidgetUtils::getEffectSlotFromNode(node, context, pChainSlot);
    m_pEffectParameterSlot = EffectWidgetUtils::getParameterSlotFromNode(
            node, context, pEffectSlot);
    VERIFY_OR_DEBUG_ASSERT(m_pEffectParameterSlot) {
        SKIN_WARNING(node, context, QStringLiteral("Could not find effect parameter slot"));
        return;
    }
    connect(m_pEffectParameterSlot.data(),
            &EffectParameterSlotBase::updated,
            this,
            &WEffectParameterKnob::parameterUpdated);
    parameterUpdated();
}

void WEffectParameterKnob::parameterUpdated() {
    if (m_pEffectParameterSlot->isLoaded()) {
        setBaseTooltip(QStringLiteral("%1\n%2").arg(
                m_pEffectParameterSlot->name(),
                m_pEffectParameterSlot->description()));
    } else {
        // The knob should be hidden by the skin when the parameterX_loaded ControlObject
        // indicates no parameter is loaded, so this tooltip should never be shown.
        setBaseTooltip("");
    }
}
