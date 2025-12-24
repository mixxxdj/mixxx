#include "widget/weffectname.h"

#include "moc_weffectname.cpp"
#include "widget/effectwidgetutils.h"

WEffectName::WEffectName(QWidget* pParent, EffectsManager* pEffectsManager)
        : WLabel(pParent),
          m_pEffectsManager(pEffectsManager) {
    effectUpdated();
}

void WEffectName::setup(const QDomNode& node, const SkinContext& context) {
    WLabel::setup(node, context);
    // EffectWidgetUtils propagates NULLs so this is all safe.
    EffectChainPointer pChainSlot = EffectWidgetUtils::getEffectChainFromNode(
            node, context, m_pEffectsManager);
    EffectSlotPointer pEffectSlot = EffectWidgetUtils::getEffectSlotFromNode(
            node, context, pChainSlot);
    if (pEffectSlot) {
        setEffectSlot(pEffectSlot);
    } else {
        SKIN_WARNING(node,
                context,
                QStringLiteral(
                        "EffectName node could not attach to effect slot."));
    }
}

void WEffectName::setEffectSlot(EffectSlotPointer pEffectSlot) {
    if (pEffectSlot) {
        m_pEffectSlot = pEffectSlot;
        connect(pEffectSlot.data(), &EffectSlot::effectChanged, this, &WEffectName::effectUpdated);
        effectUpdated();
    }
}

void WEffectName::effectUpdated() {
    QString name;
    QString description;
    if (m_pEffectSlot && m_pEffectSlot->isLoaded()) {
        EffectManifestPointer pManifest = m_pEffectSlot->getManifest();
        name = pManifest->displayName();
        //: %1 = effect name; %2 = effect description
        description = tr("%1: %2").arg(pManifest->name(), pManifest->description());
    } else {
        name = kNoEffectString;
        description = tr("No effect loaded.");
    }
    setText(name);
    setBaseTooltip(description);
}
