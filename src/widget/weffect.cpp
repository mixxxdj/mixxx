#include "widget/weffect.h"

WEffect::WEffect(QWidget* pParent)
        : WLabel(pParent) {
}

WEffect::~WEffect() {
}

void WEffect::setEffectSlot(EffectSlotPointer pEffectSlot) {
    if (pEffectSlot) {
        m_pEffectSlot = pEffectSlot;
        connect(pEffectSlot.data(), SIGNAL(updated()),
                this, SLOT(effectUpdated()));
        effectUpdated();
    }
}

void WEffect::effectUpdated() {
    QString name = tr("None");
    if (m_pEffectSlot) {
        EffectPointer pEffect = m_pEffectSlot->getEffect();
        if (pEffect) {
            name = pEffect->getManifest().name();
        }
    }
    setText(name);
}
