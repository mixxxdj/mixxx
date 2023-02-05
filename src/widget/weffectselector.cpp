#include "widget/weffectselector.h"

#include <QAbstractItemView>
#include <QtDebug>

#include "effects/effectsmanager.h"
#include "effects/visibleeffectslist.h"
#include "library/library_decl.h"
#include "moc_weffectselector.cpp"
#include "widget/effectwidgetutils.h"

WEffectSelector::WEffectSelector(QWidget* pParent, EffectsManager* pEffectsManager)
        : QComboBox(pParent),
          WBaseWidget(this),
          m_pEffectsManager(pEffectsManager),
          m_pVisibleEffectsList(pEffectsManager->getVisibleEffectsList()) {
    // Prevent this widget from getting focused by Tab/Shift+Tab
    // to avoid interfering with using the library via keyboard.
    // Allow click focus though so the list can always be opened by mouse,
    // see https://bugs.launchpad.net/mixxx/+bug/1902125
    setFocusPolicy(Qt::ClickFocus);
}

void WEffectSelector::setup(const QDomNode& node, const SkinContext& context) {
    // EffectWidgetUtils propagates NULLs so this is all safe.
    EffectChainPointer pChainSlot = EffectWidgetUtils::getEffectChainFromNode(
            node, context, m_pEffectsManager);
    m_pEffectSlot = EffectWidgetUtils::getEffectSlotFromNode(
            node, context, pChainSlot);

    if (m_pEffectSlot != nullptr) {
        connect(m_pVisibleEffectsList.data(),
                &VisibleEffectsList::visibleEffectsListChanged,
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

    const QList<EffectManifestPointer> visibleEffectManifests = m_pVisibleEffectsList->getList();
    QFontMetrics metrics(font());

    // Add empty item: no effect
    addItem(kNoEffectString);
    setItemData(0, QVariant(tr("No effect loaded.")), Qt::ToolTipRole);

    for (int i = 0; i < visibleEffectManifests.size(); ++i) {
        const EffectManifestPointer pManifest = visibleEffectManifests.at(i);
        QString elidedDisplayName = metrics.elidedText(pManifest->displayName(),
                Qt::ElideMiddle,
                view()->width() - 2);
        addItem(elidedDisplayName, QVariant(pManifest->uniqueId()));

        QString name = pManifest->name();
        QString description = pManifest->description();
        // <b> makes the effect name bold. Also, like <span> it serves as hack
        // to get Qt to treat the string as rich text so it automatically wraps long lines.
        setItemData(i + 1,
                QVariant(QStringLiteral("<b>") + name +
                        QStringLiteral("</b><br/>") + description),
                Qt::ToolTipRole);
    }

    slotEffectUpdated();
    blockSignals(false);
}

void WEffectSelector::slotEffectSelected(int newIndex) {
    const EffectManifestPointer pManifest =
            m_pEffectsManager->getBackendManager()->getManifestFromUniqueId(
                    itemData(newIndex).toString());

    m_pEffectSlot->loadEffectWithDefaults(pManifest);

    setBaseTooltip(itemData(newIndex, Qt::ToolTipRole).toString());
    // After selecting an effect send Shift+Tab to move focus to tracks table
    // in order to immediately allow keyboard shortcuts again.
    // TODO(ronso0) switch to previously focused (library?) widget instead
    ControlObject::set(ConfigKey("[Library]", "focused_widget"),
            static_cast<double>(FocusWidget::TracksTable));
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

bool WEffectSelector::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    } else if (pEvent->type() == QEvent::Wheel && !hasFocus()) {
        // don't change effect by scrolling hovered effect selector
        return true;
    }

    return QComboBox::event(pEvent);
}
