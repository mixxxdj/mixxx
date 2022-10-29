#pragma once

#include <QWidget>

#include "widget/weffectpushbutton.h"

// A specialization of WEffectPushButton that updates itself when the EQ Button Mode settings changes.
// This is needed to update the tooltips of the button with information on the new mode.
class WEqualizerPushButton : public WEffectPushButton {
  public:
    WEqualizerPushButton(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context) override;

  private:
    ControlProxy m_eqButtonMode;
};
