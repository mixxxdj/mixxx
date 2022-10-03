#include "wequalizerpushbutton.h"

#include "control/controlproxy.h"

WEqualizerPushButton::WEqualizerPushButton(QWidget* pParent, EffectsManager* pEffectsManager)
        : WEffectPushButton(pParent, pEffectsManager),
          m_eqButtonMode(ConfigKey("[Mixer Profile]", "EQButtonMode")) {
}

void WEqualizerPushButton::setup(const QDomNode& node, const SkinContext& context) {
    WEffectPushButton::setup(node, context);
    m_eqButtonMode.connectValueChanged(this, &WEqualizerPushButton::parameterUpdated);
}
