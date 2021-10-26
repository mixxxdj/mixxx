#include "widget/weffectchainpresetselector.h"

#include <QtDebug>

#include "effects/chains/quickeffectchain.h"
#include "effects/effectsmanager.h"
#include "widget/effectwidgetutils.h"

WEffectChainPresetSelector::WEffectChainPresetSelector(
        QWidget* pParent, EffectsManager* pEffectsManager)
        : QComboBox(pParent),
          WBaseWidget(this),
          m_bQuickEffectChain(false),
          m_pChainPresetManager(pEffectsManager->getChainPresetManager()),
          m_pEffectsManager(pEffectsManager) {
    // Prevent this widget from getting focused to avoid
    // interfering with using the library via keyboard.
    setFocusPolicy(Qt::NoFocus);
}

void WEffectChainPresetSelector::setup(const QDomNode& node, const SkinContext& context) {
    m_pChain = EffectWidgetUtils::getEffectChainFromNode(
            node, context, m_pEffectsManager);

    VERIFY_OR_DEBUG_ASSERT(m_pChain != nullptr) {
        SKIN_WARNING(node, context)
                << "EffectChainPresetSelector node could not attach to EffectChain";
        return;
    }

    auto chainPresetListUpdateSignal = &EffectChainPresetManager::effectChainPresetListUpdated;
    auto pQuickEffectChain = dynamic_cast<QuickEffectChain*>(m_pChain.data());
    if (pQuickEffectChain) {
        chainPresetListUpdateSignal = &EffectChainPresetManager::quickEffectChainPresetListUpdated;
        m_bQuickEffectChain = true;
    }
    connect(m_pChainPresetManager.data(),
            chainPresetListUpdateSignal,
            this,
            &WEffectChainPresetSelector::populate);
    connect(m_pChain.data(),
            &EffectChain::chainPresetChanged,
            this,
            &WEffectChainPresetSelector::slotChainPresetChanged);
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

    QList<EffectChainPresetPointer> presetList;
    if (m_bQuickEffectChain) {
        presetList = m_pEffectsManager->getChainPresetManager()->getQuickEffectPresetsSorted();
    } else {
        presetList = m_pEffectsManager->getChainPresetManager()->getPresetsSorted();
    }

    for (int i = 0; i < presetList.size(); i++) {
        auto pChainPreset = presetList.at(i);
        QString elidedDisplayName = metrics.elidedText(pChainPreset->name(),
                Qt::ElideMiddle,
                width() - 2);
        addItem(elidedDisplayName, QVariant(pChainPreset->name()));
        setItemData(i, pChainPreset->name(), Qt::ToolTipRole);
    }

    slotChainPresetChanged(m_pChain->presetName());
    blockSignals(false);
}

void WEffectChainPresetSelector::slotEffectChainPresetSelected(int index) {
    Q_UNUSED(index);
    m_pChain->loadChainPreset(
            m_pChainPresetManager->getPreset(currentData().toString()));
    setBaseTooltip(itemData(index, Qt::ToolTipRole).toString());
}

void WEffectChainPresetSelector::slotChainPresetChanged(const QString& name) {
    setCurrentIndex(findData(name));
    setBaseTooltip(name);
}

bool WEffectChainPresetSelector::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    } else if (pEvent->type() == QEvent::Wheel && !hasFocus()) {
        // don't change preset by scrolling hovered preset selector
        return true;
    }

    return QComboBox::event(pEvent);
}
