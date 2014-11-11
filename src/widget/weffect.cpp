#include <QtDebug>

#include "widget/weffect.h"

#include "effects/effectsmanager.h"

WEffect::WEffect(QWidget* pParent, EffectsManager* pEffectsManager)
        : WLabel(pParent),
          m_pEffectsManager(pEffectsManager) {
    effectUpdated();
}

WEffect::~WEffect() {
}

void WEffect::setup(QDomNode node, const SkinContext& context) {
    bool rackOk = false;
    int rackNumber = context.selectInt(node, "EffectRack", &rackOk) - 1;
    bool chainOk = false;
    int chainNumber = context.selectInt(node, "EffectUnit", &chainOk) - 1;
    bool effectOk = false;
    int effectNumber = context.selectInt(node, "Effect", &effectOk) - 1;

    // Tolerate no <EffectRack>. Use the default one.
    if (!rackOk) {
        rackNumber = 0;
    }

    if (!chainOk) {
        qDebug() << "EffectName node had invalid EffectUnit number:" << chainNumber;
    }

    if (!effectOk) {
        qDebug() << "EffectName node had invalid Effect number:" << effectNumber;
    }

    EffectRackPointer pRack = m_pEffectsManager->getEffectRack(rackNumber);
    if (pRack) {
        EffectChainSlotPointer pChainSlot = pRack->getEffectChainSlot(chainNumber);
        if (pChainSlot) {
            EffectSlotPointer pEffectSlot = pChainSlot->getEffectSlot(effectNumber);
            if (pEffectSlot) {
                setEffectSlot(pEffectSlot);
            } else {
                qDebug() << "EffectName node had invalid Effect number:" << effectNumber;
            }
        } else {
            qDebug() << "EffectName node had invalid EffectUnit number:" << chainNumber;
        }
    } else {
        qDebug() << "EffectName node had invalid EffectRack number:" << rackNumber;
    }
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
    QString name;
    QString description;
    if (m_pEffectSlot) {
        Effect* pEffect = m_pEffectSlot->getEffect();
        if (pEffect) {
            const EffectManifest& manifest = pEffect->getManifest();
            name = manifest.name();
            description = manifest.description();
        }
    } else {
        name = tr("None");
        description = tr("No effect loaded.");
    }
    setText(name);
    setBaseTooltip(description);
}
