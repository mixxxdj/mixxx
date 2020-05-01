#include <QtDebug>

#include "widget/weffectselector.h"

#include "effects/effectsmanager.h"
#include "widget/effectwidgetutils.h"

WEffectSelector::WEffectSelector(QWidget* pParent, EffectsManager* pEffectsManager)
        : QComboBox(pParent),
          WBaseWidget(this),
          m_pEffectsManager(pEffectsManager) {
    // Prevent this widget from getting focused to avoid
    // interfering with using the library via keyboard.
    setFocusPolicy(Qt::NoFocus);
}

void WEffectSelector::setup(const QDomNode& node, const SkinContext& context) {
    // EffectWidgetUtils propagates NULLs so this is all safe.
    m_pChainSlot = EffectWidgetUtils::getEffectChainSlotFromNode(
            node, context, m_pEffectsManager);
    m_pEffectSlot = EffectWidgetUtils::getEffectSlotFromNode(
            node, context, m_pChainSlot);

    if (m_pEffectSlot != nullptr) {
        connect(m_pEffectsManager,
                &EffectsManager::visibleEffectsUpdated,
                this,
                &WEffectSelector::populate);
        connect(m_pEffectSlot.data(),
                &EffectSlot::effectChanged,
                this,
                &WEffectSelector::slotEffectUpdated);
        connect(this,
                QOverload<int>::of(&QComboBox::activated),
                this,
                &WEffectSelector::slotEffectSelected);
    } else {
        SKIN_WARNING(node, context)
                << "EffectSelector node could not attach to effect slot.";
    }

    populate();
}


void WEffectSelector::populate() {
    blockSignals(true);
    clear();

    const QList<EffectManifestPointer> visibleEffectManifests =
            m_pEffectsManager->getVisibleEffectManifests();
    QFontMetrics metrics(font());

    for (int i = 0; i < visibleEffectManifests.size(); ++i) {
        const EffectManifestPointer pManifest = visibleEffectManifests.at(i);
        QString elidedDisplayName = metrics.elidedText(pManifest->displayName(),
                                                       Qt::ElideMiddle,
                                                       width() - 2);
        addItem(elidedDisplayName, QVariant(pManifest->uniqueId()));

        // NOTE(Be): Using \n instead of : as the separator does not work in
        // QComboBox item tooltips.
        QString description = tr("%1: %2").arg(pManifest->name(),
                                               pManifest->description());
        // The <span/> is a hack to get Qt to treat the string as rich text so
        // it automatically wraps long lines.
        setItemData(i, QVariant("<span/>" + description), Qt::ToolTipRole);
    }

    //: Displayed when no effect is loaded
    addItem(tr("None"), QVariant());
    setItemData(visibleEffectManifests.size(), QVariant(tr("No effect loaded.")),
                Qt::ToolTipRole);

    slotEffectUpdated();
    blockSignals(false);
}

void WEffectSelector::slotEffectSelected(int newIndex) {
    const EffectManifestPointer pManifest =
            m_pEffectsManager->getBackendManager()->getManifestFromUniqueId(
                    itemData(newIndex).toString());

    m_pEffectsManager->loadEffect(
            m_pChainSlot,
            m_pEffectSlot->getEffectSlotNumber(),
            pManifest);

    setBaseTooltip(itemData(newIndex, Qt::ToolTipRole).toString());
}

void WEffectSelector::slotEffectUpdated() {
    int newIndex;

    if (m_pEffectSlot != nullptr) {
        if (m_pEffectSlot->getManifest() != nullptr) {
            EffectManifestPointer pManifest = m_pEffectSlot->getManifest();
            newIndex = findData(QVariant(pManifest->uniqueId()));
        } else {
            newIndex = findData(QVariant());
        }
    } else {
        newIndex = findData(QVariant());
    }

    if (kEffectDebugOutput) {
        qDebug() << "WEffectSelector::slotEffectUpdated"
                 << "old" << itemData(currentIndex())
                 << "new" << itemData(newIndex);
    }

    if (newIndex != -1 && newIndex != currentIndex()) {
        setCurrentIndex(newIndex);
        setBaseTooltip(itemData(newIndex, Qt::ToolTipRole).toString());
    }
}
