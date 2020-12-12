#include "widget/weffectselector.h"

#include <QtDebug>

#include "effects/effectsmanager.h"
#include "moc_weffectselector.cpp"
#include "widget/effectwidgetutils.h"

WEffectSelector::WEffectSelector(QWidget* pParent, EffectsManager* pEffectsManager)
        : QComboBox(pParent),
          WBaseWidget(this),
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
        connect(m_pEffectsManager,
                &EffectsManager::visibleEffectsUpdated,
                this,
                &WEffectSelector::populate);
        connect(m_pEffectSlot.data(),
                &EffectSlot::updated,
                this,
                &WEffectSelector::slotEffectUpdated);
        connect(this,
                QOverload<int>::of(&WEffectSelector::currentIndexChanged),
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
        addItem(elidedDisplayName, QVariant(pManifest->id()));

        QString name = pManifest->name();
        QString description = pManifest->description();
        // <b> makes the effect name bold. Also, like <span> it serves as hack
        // to get Qt to treat the string as rich text so it automatically wraps long lines.
        setItemData(i, QVariant(QStringLiteral("<b>") + name + QStringLiteral("</b><br/>") +
                description), Qt::ToolTipRole);
    }

    //: Displayed when no effect is loaded
    addItem(tr("None"), QVariant());
    setItemData(visibleEffectManifests.size(), QVariant(tr("No effect loaded.")),
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
            EffectManifestPointer pManifest = pEffect->getManifest();
            newIndex = findData(QVariant(pManifest->id()));
        } else {
            newIndex = findData(QVariant());
        }
    } else {
        newIndex = findData(QVariant());
    }

    if (newIndex != -1 && newIndex != currentIndex()) {
        setCurrentIndex(newIndex);
        setBaseTooltip(itemData(newIndex, Qt::ToolTipRole).toString());
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
            const_cast<QFont&>(fonti).setPixelSize(
                    static_cast<int>(fonti.pixelSize() * m_scaleFactor));
        }
        // repopulate to add text according to the new font measures
        populate();
    } else if (pEvent->type() == QEvent::Wheel && !hasFocus()) {
        // don't change effect by scrolling hovered effect selector
        return true;
    }

    return QComboBox::event(pEvent);
}
