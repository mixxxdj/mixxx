#pragma once

#include "effects/defs.h"
#include "widget/weffectknobparametername.h" // Forward declaration for label widget to avoid circular dependency.
#include "widget/wknobcomposed.h"

class EffectsManager;
class WEffectKnobParameterName; // Forward declare WEffectKnobParameterName to
                                // allow usage in setLabelPointer().

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

    // Connecting the knob with its label.
    // Allow linking a label widget to this knob so it can be used as a fallback
    // in the tooltip when parameter info is unavailable.
    void setLabelPointer(WEffectKnobParameterName* pLabel) {
        m_pLabel = pLabel;
    }
    void mousePressEvent(QMouseEvent* e) override;
  private slots:
    void parameterUpdated();

  private:
    EffectsManager* m_pEffectsManager;
    EffectParameterSlotBasePointer m_pEffectParameterSlot;
    WEffectKnobParameterName* m_pLabel =
            nullptr; // Pointer to associated label. Pointer to the associated
                     // label widget (if available). Used for fallback tooltip
                     // when parameter data is missing.
};
