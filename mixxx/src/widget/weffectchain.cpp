#include "widget/weffectchain.h"

WEffectChain::WEffectChain(QWidget* pParent)
        : QLabel(pParent) {
}

WEffectChain::~WEffectChain() {
}

void WEffectChain::setEffectChain(EffectChainPointer effectChain) {
    if (effectChain) {
        m_pEffectChain = effectChain;
        connect(effectChain.data(), SIGNAL(updated()),
                this, SLOT(chainUpdated()));
        chainUpdated();
    }
}

void WEffectChain::chainUpdated() {
    if (m_pEffectChain) {
        setText(m_pEffectChain->name());
    }
}
