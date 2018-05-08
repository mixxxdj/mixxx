#include <QtDebug>

#include "widget/weffectselector.h"

#include "effects/effectsmanager.h"
#include "widget/effectwidgetutils.h"

WEffectSelector::WEffectSelector(QWidget* pParent, EffectsManager* pEffectsManager)
        : QComboBox(pParent),
          WBaseWidget(pParent),
          m_pEffectsManager(pEffectsManager),
          m_scaleFactor(1.0) {
    // Prevent this widget from getting focused to avoid
    // interfering with using the library via keyboard.
    setFocusPolicy(Qt::NoFocus);
}

void WEffectSelector::setup(const QDomNode& node, const SkinContext& context) {
    m_scaleFactor = context.getScaleFactor();

    // EffectWidgetUtils propagates NULLs so this is all safe.
    m_pRack = EffectWidgetUtils::getEffectRackFromNode(
            node, context, m_pEffectsManager);
    m_pChainSlot = EffectWidgetUtils::getEffectChainSlotFromNode(
            node, context, m_pRack);
    m_pEffectSlot = EffectWidgetUtils::getEffectSlotFromNode(
            node, context, m_pChainSlot);

    if (m_pEffectSlot != nullptr) {
        connect(m_pEffectSlot.data(), SIGNAL(updated()),
                this, SLOT(slotEffectUpdated()));
        connect(this, SIGNAL(currentIndexChanged(int)),
                this, SLOT(slotEffectSelected(int)));
    } else {
        SKIN_WARNING(node, context)
                << "EffectSelector node could not attach to effect slot.";
    }

    populate();
}


void WEffectSelector::populate() {
    blockSignals(true);
    clear();

    // TODO(xxx): filter out blacklisted effects
    // https://bugs.launchpad.net/mixxx/+bug/1653140
    const QList<EffectManifest> availableEffectManifests =
            m_pEffectsManager->getAvailableEffectManifests();
    QFontMetrics metrics(font());

    for (int i = 0; i < availableEffectManifests.size(); ++i) {
        const EffectManifest& manifest = availableEffectManifests.at(i);
        QString elidedDisplayName = metrics.elidedText(manifest.displayName(),
                                                       Qt::ElideMiddle,
                                                       width() - 2);
        addItem(elidedDisplayName, QVariant(manifest.id()));

        //: %1 = effect name; %2 = effect description
        QString description = tr("%1: %2").arg(manifest.name(), manifest.description());
        // The <span/> is a hack to get Qt to treat the string as rich text so
        // it automatically wraps long lines.
        setItemData(i, QVariant("<span/>" + description), Qt::ToolTipRole);
    }

    //: Displayed when no effect is loaded
    addItem(tr("None"), QVariant());
    setItemData(availableEffectManifests.size(), QVariant(tr("No effect loaded.")),
                Qt::ToolTipRole);

    slotEffectUpdated();
    blockSignals(false);
}

void WEffectSelector::slotEffectSelected(int newIndex) {
    const QString id = itemData(newIndex).toString();

    m_pRack->maybeLoadEffect(
            m_pChainSlot->getChainSlotNumber(),
            m_pEffectSlot->getEffectSlotNumber(),
            id);

    setBaseTooltip(itemData(newIndex, Qt::ToolTipRole).toString());
}

void WEffectSelector::slotEffectUpdated() {
    int newIndex;

    if (m_pEffectSlot != nullptr) {
        EffectPointer pEffect = m_pEffectSlot->getEffect();
        if (pEffect != nullptr) {
            const EffectManifest& manifest = pEffect->getManifest();
            newIndex = findData(QVariant(manifest.id()));
        } else {
            newIndex = findData(QVariant());
        }
    } else {
        newIndex = findData(QVariant());
    }

    if (newIndex != -1 && newIndex != currentIndex()) {
        setCurrentIndex(newIndex);
    }
}

bool WEffectSelector::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    } else if (pEvent->type() == QEvent::FontChange) {
        const QFont& fonti = font();
        // Change the new font on the fly by casting away its constancy
        // using setFont() here, would results into a recursive loop
        // resetting the font to the original css values.
        // Only scale pixel size fonts, point size fonts are scaled by the OS
        if (fonti.pixelSize() > 0) {
            const_cast<QFont&>(fonti).setPixelSize(fonti.pixelSize() * m_scaleFactor);
        }
        // repopulate to add text according to the new font measures
        populate();
    }

    return QComboBox::event(pEvent);
}
