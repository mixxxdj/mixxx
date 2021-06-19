#include "widget/weffectparameterknobcomposed.h"

#include "moc_weffectparameterknobcomposed.cpp"
#include "widget/effectwidgetutils.h"

namespace {
const QString effectGroupSeparator = "_";
const QString groupClose = "]";
} // anonymous namespace

void WEffectParameterKnobComposed::setupEffectParameterSlot(const ConfigKey& configKey) {
    EffectParameterSlotPointer pParameterSlot =
            m_pEffectsManager->getEffectParameterSlot(configKey);
    if (!pParameterSlot) {
        qWarning() << "EffectParameterKnobComposed" << configKey <<
                "is not an effect parameter.";
        return;
    }
    setEffectParameterSlot(pParameterSlot);
    setFocusPolicy(Qt::NoFocus);
}

void WEffectParameterKnobComposed::setEffectParameterSlot(
        EffectParameterSlotPointer pParameterSlot) {
    m_pEffectParameterSlot = pParameterSlot;
    if (m_pEffectParameterSlot) {
        connect(m_pEffectParameterSlot.data(),
                &EffectParameterSlot::updated,
                this,
                &WEffectParameterKnobComposed::parameterUpdated);
    }
    parameterUpdated();
}

void WEffectParameterKnobComposed::parameterUpdated() {
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
