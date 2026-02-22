#include "widget/weffectchain.h"

#include "moc_weffectchain.cpp"
#include "widget/effectwidgetutils.h"

WEffectChain::WEffectChain(QWidget* pParent, EffectsManager* pEffectsManager)
        : WLabel(pParent),
          m_pEffectsManager(pEffectsManager) {
}

void WEffectChain::setup(const QDomNode& node, const SkinContext& context) {
    WLabel::setup(node, context);
    // EffectWidgetUtils propagates NULLs so this is all safe.
    EffectChainPointer pChainSlot = EffectWidgetUtils::getEffectChainFromNode(
            node, context, m_pEffectsManager);
    if (pChainSlot) {
        setEffectChain(pChainSlot);
    } else {
        SKIN_WARNING(node,
                context,
                QStringLiteral("EffectChain node could not attach to effect "
                               "chain slot."));
    }
}

void WEffectChain::setEffectChain(EffectChainPointer pEffectChain) {
    if (pEffectChain) {
        m_pEffectChain = pEffectChain;
        connect(pEffectChain.data(),
                &EffectChain::chainPresetChanged,
                this,
                &WEffectChain::chainPresetChanged);
        chainPresetChanged(m_pEffectChain->presetName());
    }
}

void WEffectChain::chainPresetChanged(const QString& newName) {
    setText(newName);
}
