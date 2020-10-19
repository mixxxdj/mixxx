#include <QtDebug>

#include "widget/weffectchain.h"
#include "effects/effectsmanager.h"
#include "widget/effectwidgetutils.h"

WEffectChain::WEffectChain(QWidget* pParent, EffectsManager* pEffectsManager)
        : WLabel(pParent),
          m_pEffectsManager(pEffectsManager) {
    chainUpdated();
}

void WEffectChain::setup(const QDomNode& node, const SkinContext& context) {
    WLabel::setup(node, context);
    // EffectWidgetUtils propagates NULLs so this is all safe.
    EffectChainPointer pChainSlot = EffectWidgetUtils::getEffectChainFromNode(
            node, context, m_pEffectsManager);
    if (pChainSlot) {
        setEffectChain(pChainSlot);
    } else {
        SKIN_WARNING(node, context)
                << "EffectChain node could not attach to effect chain slot.";
    }
}

void WEffectChain::setEffectChain(EffectChainPointer pEffectChain) {
    if (pEffectChain) {
        m_pEffectChain = pEffectChain;
        connect(pEffectChain.data(), SIGNAL(updated()),
                this, SLOT(chainUpdated()));
        chainUpdated();
    }
}

void WEffectChain::chainUpdated() {
    // qDebug() << "chainUpdated()";
    QString name = tr("None");
    if (m_pEffectChain) {
        name = m_pEffectChain->presetName();
    }
    setText(name);
}
