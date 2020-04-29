#include "widget/effectwidgetutils.h"
#include "widget/weffectparameterknob.h"

void WEffectParameterKnob::setupEffectParameterSlot(const ConfigKey& configKey) {
    EffectParameterSlotPointer pParameterSlot =
            m_pEffectsManager->getEffectParameterSlot(configKey);
    if (!pParameterSlot) {
        qWarning() << "EffectParameterKnob" << configKey <<
                "is not an effect parameter.";
        return;
    }
    setEffectParameterSlot(pParameterSlot);
    setFocusPolicy(Qt::NoFocus);
}

void WEffectParameterKnob::setEffectParameterSlot(
        EffectParameterSlotPointer pParameterSlot) {
    m_pEffectParameterSlot = pParameterSlot;
    if (m_pEffectParameterSlot) {
        connect(m_pEffectParameterSlot.data(), SIGNAL(updated()),
                this, SLOT(parameterUpdated()));
    }
    parameterUpdated();
}

void WEffectParameterKnob::parameterUpdated() {
    if (m_pEffectParameterSlot) {
        setBaseTooltip(QString("%1\n%2").arg(
                       m_pEffectParameterSlot->name(),
                       m_pEffectParameterSlot->description()));
    } else {
        // The knob should be hidden by the skin when the parameterX_loaded ControlObject
        // indicates no parameter is loaded, so this tooltip should never be shown.
        setBaseTooltip("");
    }
}
