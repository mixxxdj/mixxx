#include "widget/weffectchain.h"

WEffectChain::WEffectChain(QWidget* pParent)
        : QLabel(pParent) {
}

WEffectChain::~WEffectChain() {
}

void WEffectChain::setEffectChainSlot(EffectChainSlotPointer effectChainSlot) {
    if (effectChainSlot) {
        m_pEffectChainSlot = effectChainSlot;
        connect(effectChainSlot.data(), SIGNAL(updated()),
                this, SLOT(chainUpdated()));
        chainUpdated();
    }
}

void WEffectChain::chainUpdated() {
    if (m_pEffectChainSlot) {
        setText(m_pEffectChainSlot->name());
    }
}
