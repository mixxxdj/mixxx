#pragma once

#include "effects/defs.h"
#include "widget/wknob.h"

class EffectsManager;

// This is used for effect parameter knobs with dynamic
// tooltips, if the knob value is displayed by one of e.g.
// 64 pixmaps.
// If the knob value can be displayed by rotating a single
// SVG, use WEffectParameterKnobComposed.
class WEffectParameterKnob : public WKnob {
    Q_OBJECT
  public:
    WEffectParameterKnob(QWidget* pParent, EffectsManager* pEffectsManager)
            : WKnob(pParent),
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
