#include <QtDebug>

#include "widget/weffectparameter.h"
#include "effects/effectsmanager.h"

WEffectParameter::WEffectParameter(QWidget* pParent, EffectsManager* pEffectsManager)
        : WLabel(pParent),
          m_pEffectsManager(pEffectsManager) {
    parameterUpdated();
}

WEffectParameter::~WEffectParameter() {
}

void WEffectParameter::setup(QDomNode node, const SkinContext& context) {
    bool rackOk = false;
    int rackNumber = context.selectInt(node, "EffectRack", &rackOk) - 1;
    bool chainOk = false;
    int chainNumber = context.selectInt(node, "EffectChain", &chainOk) - 1;
    bool effectOk = false;
    int effectNumber = context.selectInt(node, "Effect", &effectOk) - 1;
    bool parameterOk = false;
    int parameterNumber = context.selectInt(node, "EffectParameter", &parameterOk) - 1;

    // Tolerate no <EffectRack>. Use the default one.
    if (!rackOk) {
        rackNumber = 0;
    }

    if (!chainOk) {
        qDebug() << "EffectParameterName node had invalid EffectChain number:" << chainNumber;
    }

    if (!effectOk) {
        qDebug() << "EffectParameterName node had invalid Effect number:" << effectNumber;
    }

    if (!parameterOk) {
        qDebug() << "EffectParameterName node had invalid Parameter number:" << parameterNumber;
    }

    EffectRackPointer pRack = m_pEffectsManager->getEffectRack(rackNumber);
    if (pRack) {
        EffectChainSlotPointer pChainSlot = pRack->getEffectChainSlot(chainNumber);
        if (pChainSlot) {
            EffectSlotPointer pEffectSlot = pChainSlot->getEffectSlot(effectNumber);
            if (pEffectSlot) {
                EffectParameterSlotPointer pParameterSlot =
                        pEffectSlot->getEffectParameterSlot(parameterNumber);
                if (pParameterSlot) {
                    setEffectParameterSlot(pParameterSlot);
                } else {
                    qDebug() << "EffectParameterName node had invalid Parameter number:" << parameterNumber;
                }
            } else {
                qDebug() << "EffectParameterName node had invalid Effect number:" << effectNumber;
            }
        } else {
            qDebug() << "EffectParameterName node had invalid EffectChain number:" << chainNumber;
        }
    } else {
        qDebug() << "EffectParameterName node had invalid EffectRack number:" << rackNumber;
    }
}

void WEffectParameter::setEffectParameterSlot(EffectParameterSlotPointer pEffectParameterSlot) {
    if (pEffectParameterSlot) {
        m_pEffectParameterSlot = pEffectParameterSlot;
        connect(pEffectParameterSlot.data(), SIGNAL(updated()),
                this, SLOT(parameterUpdated()));
        parameterUpdated();
    }
}

void WEffectParameter::parameterUpdated() {
    if (m_pEffectParameterSlot) {
        setText(m_pEffectParameterSlot->name());
        setBaseTooltip(m_pEffectParameterSlot->description());
    } else {
        setText(tr("None"));
        setBaseTooltip(tr("No effect loaded."));
    }
}
