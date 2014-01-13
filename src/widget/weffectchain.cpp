#include <QtDebug>

#include "widget/weffectchain.h"
#include "effects/effectsmanager.h"

WEffectChain::WEffectChain(QWidget* pParent, EffectsManager* pEffectsManager)
        : WLabel(pParent),
          m_pEffectsManager(pEffectsManager) {
    chainUpdated();
}

WEffectChain::~WEffectChain() {
}

void WEffectChain::setup(QDomNode node, const SkinContext& context) {
    bool rackOk = false;
    int rackNumber = context.selectInt(node, "EffectRack", &rackOk) - 1;
    bool chainOk = false;
    int chainNumber = context.selectInt(node, "EffectChain", &chainOk) - 1;

    // Tolerate no <EffectRack>. Use the default one.
    if (!rackOk) {
        rackNumber = 0;
    }

    if (!chainOk) {
        qDebug() << "EffectChainName node had invalid EffectChain number:" << chainNumber;
    }

    EffectRackPointer pRack = m_pEffectsManager->getEffectRack(rackNumber);
    if (pRack) {
        EffectChainSlotPointer pChainSlot = pRack->getEffectChainSlot(chainNumber);
        if (pChainSlot) {
            setEffectChainSlot(pChainSlot);
        } else {
            qDebug() << "EffectChainName node had invalid EffectChain number:" << chainNumber;
        }
    } else {
        qDebug() << "EffectChainName node had invalid EffectRack number:" << rackNumber;
    }
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
