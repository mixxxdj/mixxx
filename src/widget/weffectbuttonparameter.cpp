#include <QtDebug>

#include "widget/weffectbuttonparameter.h"
#include "effects/effectsmanager.h"

WEffectButtonParameter::WEffectButtonParameter(QWidget* pParent, EffectsManager* pEffectsManager)
        : WEffectParameterBase(pParent, pEffectsManager) {
}

WEffectButtonParameter::~WEffectButtonParameter() {
}

void WEffectButtonParameter::setup(QDomNode node, const SkinContext& context) {
    bool rackOk = false;
    int rackNumber = context.selectInt(node, "EffectRack", &rackOk) - 1;
    bool chainOk = false;
    int chainNumber = context.selectInt(node, "EffectUnit", &chainOk) - 1;
    bool effectOk = false;
    int effectNumber = context.selectInt(node, "Effect", &effectOk) - 1;
    bool parameterOk = false;
    int parameterNumber = context.selectInt(node, "EffectButtonParameter", &parameterOk) - 1;

    // Tolerate no <EffectRack>. Use the default one.
    if (!rackOk) {
        rackNumber = 0;
    }

    if (!chainOk) {
        qDebug() << "EffectButtonParameterName node had invalid EffectUnit number:" << chainNumber;
    }

    if (!effectOk) {
        qDebug() << "EffectButtonParameterName node had invalid Effect number:" << effectNumber;
    }

    if (!parameterOk) {
        qDebug() << "EffectButtonParameterName node had invalid ButtonParameter number:" << parameterNumber;
    }

    EffectRackPointer pRack = m_pEffectsManager->getEffectRack(rackNumber);
    if (pRack) {
        EffectChainSlotPointer pChainSlot = pRack->getEffectChainSlot(chainNumber);
        if (pChainSlot) {
            EffectSlotPointer pEffectSlot = pChainSlot->getEffectSlot(effectNumber);
            if (pEffectSlot) {
                EffectParameterSlotBasePointer pParameterSlot =
                        pEffectSlot->getEffectButtonParameterSlot(parameterNumber);
                if (pParameterSlot) {
                    setEffectParameterSlot(pParameterSlot);
                } else {
                    qDebug() << "EffectButtonParameterName node had invalid ButtonParameter number:" << parameterNumber;
                }
            } else {
                qDebug() << "EffectButtonParameterName node had invalid Effect number:" << effectNumber;
            }
        } else {
            qDebug() << "EffectButtonParameterName node had invalid EffectUnit number:" << chainNumber;
        }
    } else {
        qDebug() << "EffectButtonParameterName node had invalid EffectRack number:" << rackNumber;
    }
}
