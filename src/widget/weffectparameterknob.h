#ifndef WEFFECTKNOB_H
#define WEFFECTKNOB_H

#include "widget/wknob.h"
#include "effects/effectparameterslot.h"

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
    // Set the EffectParameterSlot that should be monitored by this
    // WEffectKnobComposed.
    void setEffectParameterSlot(EffectParameterSlotPointer pParameterSlot);

    EffectsManager* m_pEffectsManager;
    EffectParameterSlotBasePointer m_pEffectParameterSlot;
};

#endif // WEFFECTKNOB_H
