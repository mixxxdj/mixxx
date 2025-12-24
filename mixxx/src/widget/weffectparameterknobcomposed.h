#pragma once

#include "effects/defs.h"
#include "widget/wknobcomposed.h"

class EffectsManager;

// This is used for effect parameter knobs with dynamic
// tooltips, if the knob value is displayed by rotating a
// single SVG image.
// For more complex transitions you may consider to use
// WEffectParameterKnob, which displays one of e.g. 64
// pixmaps
class WEffectParameterKnobComposed : public WKnobComposed {
    Q_OBJECT
  public:
    WEffectParameterKnobComposed(QWidget* pParent, EffectsManager* pEffectsManager)
            : WKnobComposed(pParent),
              m_pEffectsManager(pEffectsManager) {
        setFocusPolicy(Qt::NoFocus);
    };

    void setup(const QDomNode& node, const SkinContext& context) override;

  private slots:
    void parameterUpdated();

  private:
    EffectsManager* m_pEffectsManager;
    EffectParameterSlotBasePointer m_pEffectParameterSlot;
};
