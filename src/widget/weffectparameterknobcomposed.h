#ifndef WEFFECTKNOBCOMPOSED_H
#define WEFFECTKNOBCOMPOSED_H

#include "widget/wknobcomposed.h"
#include "effects/effectparameterslotbase.h"

class WEffectParameterKnobComposed : public WKnobComposed {
  Q_OBJECT
  public:
    WEffectParameterKnobComposed(QWidget* pParent, EffectsManager* pEffectsManager) :
        WKnobComposed(pParent),
        m_pEffectsManager(pEffectsManager) {
    };

    void setup(const QDomNode& node, const SkinContext& context);

  private slots:
    void parameterUpdated();

  private:
    // Set the EffectParameterSlot that should be monitored by this
    // WEffectKnobComposed.
    void setEffectParameterSlot(EffectParameterSlotBasePointer pEffectParameterSlot);

    EffectsManager* m_pEffectsManager;
    EffectParameterSlotBasePointer m_pEffectParameterSlot;
};

#endif // WEFFECTKNOBCOMPOSED_H
