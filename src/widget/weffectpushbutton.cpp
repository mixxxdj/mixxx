#include "widget/weffectpushbutton.h"

#include <QActionGroup>
#include <QMenu>
#include <QMouseEvent>

#include "effects/effectparameterslotbase.h"
#include "effects/presets/effectchainpreset.h"
#include "moc_weffectpushbutton.cpp"
#include "widget/effectwidgetutils.h"

WEffectPushButton::WEffectPushButton(QWidget* pParent, EffectsManager* pEffectsManager)
        : WPushButton(pParent),
          m_pEffectsManager(pEffectsManager),
          m_pButtonMenu(nullptr) {
    setFocusPolicy(Qt::NoFocus);
}

void WEffectPushButton::setup(const QDomNode& node, const SkinContext& context) {
    // Setup parent class.
    WPushButton::setup(node, context);

    auto pChainSlot = EffectWidgetUtils::getEffectChainFromNode(
            node, context, m_pEffectsManager);
    auto pEffectSlot = EffectWidgetUtils::getEffectSlotFromNode(node, context, pChainSlot);
    m_pEffectParameterSlot = EffectWidgetUtils::getButtonParameterSlotFromNode(
            node, context, pEffectSlot);
    if (!m_pEffectParameterSlot) {
        SKIN_WARNING(node, context, QStringLiteral("Could not find effect parameter slot"));
        DEBUG_ASSERT(false);
    }
    connect(m_pEffectParameterSlot.data(),
            &EffectParameterSlotBase::updated,
            this,
            &WEffectPushButton::parameterUpdated);

    m_pButtonMenu = new QMenu(this);
    connect(m_pButtonMenu,
            &QMenu::triggered,
            this,
            &WEffectPushButton::slotActionChosen);
    parameterUpdated();
}

void WEffectPushButton::onConnectedControlChanged(double dParameter, double dValue) {
    const QList<QAction*> actions = m_pButtonMenu->actions();
    for (const auto& action : actions) {
        if (action->data().toDouble() == dValue) {
            action->setChecked(true);
            break;
        }
    }
    WPushButton::onConnectedControlChanged(dParameter, dValue);
}

void WEffectPushButton::mousePressEvent(QMouseEvent* e) {
    const bool rightClick = e->button() == Qt::RightButton;
    if (rightClick && !m_pButtonMenu->actions().isEmpty()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_pButtonMenu->exec(e->globalPosition().toPoint());
#else
        m_pButtonMenu->exec(e->globalPos());
#endif
        return;
    }

    // Pass all other press events to the base class.
    WPushButton::mousePressEvent(e);

    // The push handler may have set the left value. Check the corresponding
    // QAction.
    const double leftValue = getControlParameterLeft();
    const QList<QAction*> actions = m_pButtonMenu->actions();
    for (const auto& action : actions) {
        if (action->data().toDouble() == leftValue) {
            action->setChecked(true);
            break;
        }
    }
}

void WEffectPushButton::mouseReleaseEvent(QMouseEvent* e) {
    // Pass all other release events to the base class.
    WPushButton::mouseReleaseEvent(e);

    // The release handler may have set the left value. Check the corresponding
    // QAction.
    const double leftValue = getControlParameterLeft();
    const QList<QAction*> actions = m_pButtonMenu->actions();
    for (QAction* action : actions) {
        if (action->data().toDouble() == leftValue) {
            action->setChecked(true);
            break;
        }
    }
}

void WEffectPushButton::parameterUpdated() {
    VERIFY_OR_DEBUG_ASSERT(m_pEffectParameterSlot) {
        return;
    }

    // Set tooltip
    if (m_pEffectParameterSlot->isLoaded()) {
        setBaseTooltip(QStringLiteral("%1\n%2").arg(
                m_pEffectParameterSlot->name(),
                m_pEffectParameterSlot->description()));
    } else {
        // The button should be hidden by the skin when the buttonparameterX_loaded
        // ControlObject indicates no parameter is loaded, so this tooltip should
        // never be shown.
        setBaseTooltip("");
    }

    m_pButtonMenu->clear();
    EffectManifestParameterPointer pManifest = m_pEffectParameterSlot->getManifest();
    QList<QPair<QString, double>> options;
    if (pManifest) {
        options = pManifest->getSteps();
    }

    // qDebug() << " HERE IS THE OPTIONS SIZE: " << options.size() << m_pEffectParameterSlot->getManifest().name();
    m_iNoStates = options.size();
    if (m_iNoStates == 0) {
        // Toggle button without a menu
        m_iNoStates = 2;
        return;
    }
    double value = getControlParameterLeft();

    auto* actionGroup = new QActionGroup(m_pButtonMenu);
    actionGroup->setExclusive(true);
    for (const auto& option : std::as_const(options)) {
        // action is added automatically to actionGroup
        auto* action = new QAction(actionGroup);
        // qDebug() << options[i].first;
        action->setText(option.first);
        action->setData(option.second);
        action->setCheckable(true);

        if (option.second == value) {
            action->setChecked(true);
        }
        m_pButtonMenu->addAction(action);
    }
}

void WEffectPushButton::slotActionChosen(QAction* action) {
    action->setChecked(true);
    setControlParameterLeftDown(action->data().toDouble());
}
