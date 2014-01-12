#include "widget/weffectchain.h"

WEffectChain::WEffectChain(QWidget* pParent)
        : WLabel(pParent) {
}

WEffectChain::~WEffectChain() {
}

void WEffectChain::setEffectChainSlot(EffectChainSlotPointer pEffectChainSlot) {
    if (pEffectChainSlot) {
        m_pEffectChainSlot = pEffectChainSlot;
        connect(pEffectChainSlot.data(), SIGNAL(updated()),
                this, SLOT(chainUpdated()));
        chainUpdated();
    }
}

void WEffectChain::chainUpdated() {
    QString name = tr("None");
    if (m_pEffectChainSlot) {
        EffectChainPointer pChain = m_pEffectChainSlot->getEffectChain();
        if (pChain) {
            name = pChain->name();
        }
    }
    setText(name);
}
