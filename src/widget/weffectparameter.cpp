#include <QtDebug>

#include "widget/weffectparameter.h"
#include "effects/effectsmanager.h"

WEffectParameter::WEffectParameter(QWidget* pParent, EffectsManager* pEffectsManager)
        : WEffectParameterBase(pParent, pEffectsManager) {
}

WEffectParameter::~WEffectParameter() {
}

void WEffectParameter::setup(QDomNode node, const SkinContext& context) {
    bool rackOk = false;
    int rackNumber = context.selectInt(node, "EffectRack", &rackOk) - 1;
    bool chainOk = false;
    int chainNumber = context.selectInt(node, "EffectUnit", &chainOk) - 1;
    bool effectOk = false;
    int effectNumber = context.selectInt(node, "Effect", &effectOk) - 1;
    bool parameterOk = false;
    int parameterNumber = context.selectInt(node, "EffectParameter", &parameterOk) - 1;

    // Tolerate no <EffectRack>. Use the default one.
    if (!rackOk) {
        rackNumber = 0;
    }

    if (!chainOk) {
        qDebug() << "EffectParameterName node had invalid EffectUnit number:" << chainNumber;
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
                EffectParameterSlotBasePointer pParameterSlot =
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
            qDebug() << "EffectParameterName node had invalid EffectUnit number:" << chainNumber;
        }
    } else {
        qDebug() << "EffectParameterName node had invalid EffectRack number:" << rackNumber;
    }
}
