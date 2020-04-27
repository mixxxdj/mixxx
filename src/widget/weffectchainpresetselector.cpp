#include "widget/weffectchainpresetselector.h"

#include <QtDebug>

#include "effects/effectsmanager.h"
#include "widget/effectwidgetutils.h"

WEffectChainPresetSelector::WEffectChainPresetSelector(
        QWidget* pParent, EffectsManager* pEffectsManager)
        : QComboBox(pParent),
          WBaseWidget(this),
          m_pEffectsManager(pEffectsManager) {
    // Prevent this widget from getting focused to avoid
    // interfering with using the library via keyboard.
    setFocusPolicy(Qt::NoFocus);
}

void WEffectChainPresetSelector::setup(const QDomNode& node, const SkinContext& context) {
    m_pChainSlot = EffectWidgetUtils::getEffectChainSlotFromNode(
            node, context, m_pEffectsManager);

    VERIFY_OR_DEBUG_ASSERT(m_pChainSlot != nullptr) {
        SKIN_WARNING(node, context)
                << "EffectChainPresetSelector node could not attach to EffectChainSlot";
        return;
    }

    connect(m_pEffectsManager,
            &EffectsManager::effectChainPresetListUpdated,
            this,
            &WEffectChainPresetSelector::populate);
    connect(m_pChainSlot.data(),
            &EffectChainSlot::nameChanged,
            this,
            &WEffectChainPresetSelector::slotEffectChainNameChanged);
    connect(this,
            QOverload<int>::of(&QComboBox::activated),
            this,
            &WEffectChainPresetSelector::slotEffectChainPresetSelected);

    populate();
}

void WEffectChainPresetSelector::populate() {
    blockSignals(true);
    clear();

    QFontMetrics metrics(font());

    for (const auto& pChainPreset : m_pEffectsManager->getAvailableChainPresets()) {
        QString elidedDisplayName = metrics.elidedText(pChainPreset->name(),
                Qt::ElideMiddle,
                width() - 2);
        addItem(elidedDisplayName, QVariant(pChainPreset->name()));
    }

    slotEffectChainNameChanged(m_pChainSlot->presetName());
    blockSignals(false);
}

void WEffectChainPresetSelector::slotEffectChainPresetSelected(int index) {
    m_pEffectsManager->loadEffectChainPreset(m_pChainSlot.get(), currentData().toString());
}

void WEffectChainPresetSelector::slotEffectChainNameChanged(const QString& name) {
    setCurrentIndex(findData(name));
}
