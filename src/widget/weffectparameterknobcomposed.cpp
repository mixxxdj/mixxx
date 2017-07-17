#include "widget/effectwidgetutils.h"
#include "widget/weffectparameterknobcomposed.h"

void WEffectParameterKnobComposed::setup(const QDomNode& node,
                                         const SkinContext& context) {
    WKnobComposed::setup(node, context);

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
                << "EffectParameterKnobComposed node could not attach to effect parameter";
    }
};

void WEffectParameterKnobComposed::parameterUpdated() {
    if (m_pEffectParameterSlot) {
        setBaseTooltip(QString("%1\n%2").arg(
                       m_pEffectParameterSlot->name(),
                       m_pEffectParameterSlot->description()));
    } else {
        setBaseTooltip(tr("No effect loaded."));
    }
}

void WEffectParameterKnobComposed::setEffectParameterSlot(
        EffectParameterSlotBasePointer pParameterSlot) {
    m_pEffectParameterSlot = pParameterSlot;
    if (m_pEffectParameterSlot) {
        connect(m_pEffectParameterSlot.data(), SIGNAL(updated()),
                this, SLOT(parameterUpdated()));
    }
    parameterUpdated();
}
