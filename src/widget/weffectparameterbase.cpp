#include <QtDebug>

#include "widget/weffectparameterbase.h"
#include "effects/effectsmanager.h"

WEffectParameterBase::WEffectParameterBase(QWidget* pParent, EffectsManager* pEffectsManager)
        : WLabel(pParent),
          m_pEffectsManager(pEffectsManager) {
    parameterUpdated();
}

WEffectParameterBase::~WEffectParameterBase() {
}

void WEffectParameterBase::setEffectParameterSlot(EffectParameterSlotBasePointer pEffectParameterSlot) {
	m_pEffectParameterSlot = pEffectParameterSlot;
  if (m_pEffectParameterSlot) {
      connect(m_pEffectParameterSlot.data(), SIGNAL(updated()),
              this, SLOT(parameterUpdated()));
  }
  parameterUpdated();
}

void WEffectParameterBase::parameterUpdated() {
    if (m_pEffectParameterSlot) {
        setText(m_pEffectParameterSlot->name());
        setBaseTooltip(m_pEffectParameterSlot->description());
    } else {
        setText(tr("None"));
        setBaseTooltip(tr("No effect loaded."));
    }
}
