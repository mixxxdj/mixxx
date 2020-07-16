#include <QtDebug>

#include "widget/weffectparameternamebase.h"
#include "effects/effectsmanager.h"

WEffectParameterNameBase::WEffectParameterNameBase(QWidget* pParent, EffectsManager* pEffectsManager)
        : WLabel(pParent),
          m_pEffectsManager(pEffectsManager) {
    parameterUpdated();
}

void WEffectParameterNameBase::setEffectParameterSlot(
        EffectParameterSlotBasePointer pEffectKnobParameterSlot) {
    m_pEffectParameterSlot = pEffectKnobParameterSlot;
    if (m_pEffectParameterSlot) {
        connect(m_pEffectParameterSlot.data(),
                &EffectParameterSlotBase::updated,
                this,
                &WEffectParameterNameBase::parameterUpdated);
    }
    parameterUpdated();
}

void WEffectParameterNameBase::parameterUpdated() {
    if (m_pEffectParameterSlot) {
        if (!m_pEffectParameterSlot->shortName().isEmpty()) {
            setText(m_pEffectParameterSlot->shortName());
        } else {
            setText(m_pEffectParameterSlot->name());
        }
        setBaseTooltip(QString("%1\n%2").arg(
                       m_pEffectParameterSlot->name(),
                       m_pEffectParameterSlot->description()));
    } else {
        setText(tr("None"));
        setBaseTooltip(tr("No effect loaded."));
    }
}
