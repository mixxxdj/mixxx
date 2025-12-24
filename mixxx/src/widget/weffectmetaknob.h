#pragma once

#include "effects/defs.h"
#include "widget/wknobcomposed.h"

class EffectsManager;

// This is used for effect parameter knobs with dynamic
// tooltips, if the knob value is displayed by rotating a single SVG..
class WEffectMetaKnob : public WKnobComposed {
    Q_OBJECT
  public:
    WEffectMetaKnob(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context) override;

  private slots:
    void effectChanged();

  private:
    EffectsManager* m_pEffectsManager;
    EffectSlotPointer m_pEffectSlot;
};
