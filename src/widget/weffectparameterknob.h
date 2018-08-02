#ifndef WEFFECTKNOB_H
#define WEFFECTKNOB_H

#include "widget/wknob.h"
#include "effects/effectknobparameterslot.h"

class EffectsManager;

// This is used for effect parameter knobs with dynamic
// tooltips, if the knob value is displayed by one of e.g.
// 64 pixmaps.
// If the knob value can be displayed by rotating a single
// SVG, use WEffectParameterKnobComposed.
class WEffectParameterKnob : public WKnob {
  Q_OBJECT
  public:
    WEffectParameterKnob(QWidget* pParent, EffectsManager* pEffectsManager) :
        WKnob(pParent),
        m_pEffectsManager(pEffectsManager) {
    };

    void setupEffectParameterSlot(const ConfigKey& configKey);

  private slots:
    void parameterUpdated();

  private:
    // Set the EffectKnobParameterSlot that should be monitored by this
    // WEffectKnobComposed.
    void setEffectKnobParameterSlot(EffectKnobParameterSlotPointer pParameterSlot);

    EffectsManager* m_pEffectsManager;
    EffectParameterSlotBasePointer m_pEffectParameterSlot;
};

#endif // WEFFECTKNOB_H
