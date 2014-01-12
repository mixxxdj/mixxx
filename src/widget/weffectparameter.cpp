#include "widget/weffectparameter.h"

WEffectParameter::WEffectParameter(QWidget* pParent)
        : WLabel(pParent) {

}

WEffectParameter::~WEffectParameter() {
}

void WEffectParameter::setEffectParameterSlot(EffectParameterSlotPointer pEffectParameterSlot) {
    if (pEffectParameterSlot) {
        m_pEffectParameterSlot = pEffectParameterSlot;
        connect(pEffectParameterSlot.data(), SIGNAL(updated()),
                this, SLOT(parameterUpdated()));
        parameterUpdated();
    }
}

void WEffectParameter::parameterUpdated() {
    if (m_pEffectParameterSlot) {
        setText(m_pEffectParameterSlot->name());
    } else {
        setText(tr("None"));
    }
}
