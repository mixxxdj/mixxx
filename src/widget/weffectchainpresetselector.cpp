#include "widget/weffectchainpresetselector.h"

#include <QAbstractItemView>
#include <QStyleOption>
#include <QStylePainter>

#include "effects/chains/quickeffectchain.h"
#include "effects/effectsmanager.h"
#include "effects/presets/effectchainpreset.h"
#include "effects/presets/effectpreset.h"
#include "moc_weffectchainpresetselector.cpp"
#include "widget/effectwidgetutils.h"

class QPaintEvent;

WEffectChainPresetSelector::WEffectChainPresetSelector(
        QWidget* pParent, EffectsManager* pEffectsManager)
        : QComboBox(pParent),
          WBaseWidget(this),
          m_bQuickEffectChain(false),
          m_pChainPresetManager(pEffectsManager->getChainPresetManager()),
          m_pEffectsManager(pEffectsManager) {
    // Prevent this widget from getting focused by Tab/Shift+Tab
    // to avoid interfering with using the library via keyboard.
    // Allow click focus though so the list can always be opened by mouse,
    // see https://github.com/mixxxdj/mixxx/issues/10184
    setFocusPolicy(Qt::ClickFocus);
}

void WEffectChainPresetSelector::setup(const QDomNode& node, const SkinContext& context) {
    m_pChain = EffectWidgetUtils::getEffectChainFromNode(
            node, context, m_pEffectsManager);

#ifdef __STEM__
    VERIFY_OR_DEBUG_ASSERT(m_pChain != nullptr) {
        SKIN_WARNING(node,
                context,
                QStringLiteral("EffectChainPresetSelector node could not "
                               "attach to EffectChain"));
        return;
    }
#else
    if (m_pChain == nullptr) {
        // This happens if the skin has stem nodes but Mixxx has no stem support.
        return;
    }
#endif

    auto* pQuickEffectChain = qobject_cast<QuickEffectChain*>(m_pChain.data());
    if (pQuickEffectChain) {
        connect(m_pChainPresetManager.data(),
                &EffectChainPresetManager::quickEffectChainPresetListUpdated,
                this,
                &WEffectChainPresetSelector::populate);
        m_bQuickEffectChain = true;
    } else {
        connect(m_pChainPresetManager.data(),
                &EffectChainPresetManager::effectChainPresetListUpdated,
                this,
                &WEffectChainPresetSelector::populate);
    }
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

    const EffectsBackendManagerPointer pBackendManager = m_pEffectsManager->getBackendManager();
    QStringList effectNames;
    for (int i = 0; i < presetList.size(); i++) {
        auto pChainPreset = presetList.at(i);
        QString elidedDisplayName = metrics.elidedText(pChainPreset->name(),
                Qt::ElideMiddle,
                view()->width() - 2);
        addItem(elidedDisplayName, QVariant(pChainPreset->name()));
        QString tooltip =
                QStringLiteral("<b>") + pChainPreset->name() + QStringLiteral("</b>");
        for (const auto& pEffectPreset : pChainPreset->effectPresets()) {
            if (!pEffectPreset->isEmpty()) {
                EffectManifestPointer pManifest = pBackendManager->getManifest(pEffectPreset);
                if (pManifest) {
                    effectNames.append(pManifest->name());
                }
            }
        }
        if (effectNames.size() > 1) {
            tooltip.append("<br/>");
            tooltip.append(effectNames.join("<br/>"));
        }
        effectNames.clear();
        setItemData(i, tooltip, Qt::ToolTipRole);
    }

    slotChainPresetChanged(m_pChain->presetName());
    blockSignals(false);
}

void WEffectChainPresetSelector::slotEffectChainPresetSelected(int index) {
    Q_UNUSED(index);
    m_pChain->loadChainPreset(
            m_pChainPresetManager->getPreset(currentData().toString()));
    // Clicking a chain item moves keyboard focus to the list view.
    // Move focus back to the previously focused library widget.
    ControlObject::set(ConfigKey("[Library]", "refocus_prev_widget"), 1);
}

void WEffectChainPresetSelector::slotChainPresetChanged(const QString& name) {
    setCurrentIndex(findData(name));
    setBaseTooltip(itemData(currentIndex(), Qt::ToolTipRole).toString());
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

void WEffectChainPresetSelector::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
    // The default paint implementation aligns the text based on the layout direction.
    // Override to allow qss to align the text of the closed combobox with the
    // Quick effect controls in the mixer.
    QStylePainter painter(this);
    QStyleOptionComboBox comboStyle;
    // Initialize the style and draw the frame, down-arrow etc.
    // Note: using 'comboStyle.initFrom(this)' and 'painter.drawComplexControl(...)
    // here would not paint the hover style of the down arrow.
    initStyleOption(&comboStyle);
    style()->drawComplexControl(QStyle::CC_ComboBox, &comboStyle, &painter, this);

    QStyleOptionButton buttonStyle;
    buttonStyle.initFrom(this);
    QRect buttonRect = style()->subControlRect(
            QStyle::CC_ComboBox, &comboStyle, QStyle::SC_ComboBoxEditField, this);
    buttonStyle.rect = buttonRect;
    QFontMetrics metrics(font());
    // Since the chain selector and the popup can differ in width,
    // elide the button text independently from the popup display name.
    buttonStyle.text = metrics.elidedText(
            currentData().toString(),
            Qt::ElideRight,
            buttonRect.width());
    // Draw the text for the selector button. Alternative: painter.drawControl(...)
    style()->drawControl(QStyle::CE_PushButtonLabel, &buttonStyle, &painter, this);
}
