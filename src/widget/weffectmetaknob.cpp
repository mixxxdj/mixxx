#include "widget/weffectmetaknob.h"

#include "effects/effectslot.h"
#include "moc_weffectmetaknob.cpp"
#include "widget/effectwidgetutils.h"

WEffectMetaKnob::WEffectMetaKnob(QWidget* pParent, EffectsManager* pEffectsManager)
        : WKnobComposed(pParent),
          m_pEffectsManager(pEffectsManager) {
}

void WEffectMetaKnob::setup(const QDomNode& node, const SkinContext& context) {
    WKnobComposed::setup(node, context);
    auto pChainSlot = EffectWidgetUtils::getEffectChainFromNode(
            node, context, m_pEffectsManager);
    m_pEffectSlot =
            EffectWidgetUtils::getEffectSlotFromNode(node, context, pChainSlot);
    VERIFY_OR_DEBUG_ASSERT(m_pEffectSlot) {
        SKIN_WARNING(node, context, QStringLiteral("Could not find effect slot"));
        return;
    }
    connect(m_pEffectSlot.data(),
            &EffectSlot::effectChanged,
            this,
            &WEffectMetaKnob::effectChanged);
    effectChanged();
}

void WEffectMetaKnob::effectChanged() {
    EffectManifestPointer pManifest = nullptr;
    if (m_pEffectSlot->isLoaded()) {
        pManifest = m_pEffectSlot->getManifest();
    }

    if (pManifest) {
        setDefaultAngleFromParameterOrReset(pManifest->metaknobDefault());
        setBaseTooltip(QStringLiteral("%1\n%2").arg(
                pManifest->name(),
                pManifest->description()));
    } else {
        setDefaultAngleFromParameterOrReset(std::nullopt);
        setBaseTooltip("");
    }
    update();
}
