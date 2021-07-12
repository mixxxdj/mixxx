#pragma once

#include "widget/wknobcomposed.h"
#include "effects/effectparameterslot.h"

// This is used for effect parameter knobs with dynamic
// tooltips, if the knob value is displayed by rotating a
// single SVG image.
// For more complex transitions you may consider to use
// WEffectParameterKnob, which displays one of e.g. 64
// pixmaps
class WEffectParameterKnobComposed : public WKnobComposed {
  Q_OBJECT
  public:
    WEffectParameterKnobComposed(QWidget* pParent, EffectsManager* pEffectsManager) :
        WKnobComposed(pParent),
        m_pEffectsManager(pEffectsManager) {
    };

    void setupEffectParameterSlot(const ConfigKey& configKey);

  private slots:
    void parameterUpdated();

  private:
    // Set the EffectParameterSlot that should be monitored by this
    // WEffectKnobComposed.
    void setEffectParameterSlot(EffectParameterSlotPointer pParameterSlot);

    EffectsManager* m_pEffectsManager;
    EffectParameterSlotBasePointer m_pEffectParameterSlot;
};
