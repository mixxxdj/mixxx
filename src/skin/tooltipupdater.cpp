#include "widget/wslidercomposed.h"
#include "widget/wpushbutton.h"
#include "tooltipupdater.h"

TooltipShortcutUpdater::TooltipShortcutUpdater() { }

TooltipShortcutUpdater::~TooltipShortcutUpdater() {
    // TODO(Tomasito) Delete WidgetTooltipWatchers
}

void TooltipShortcutUpdater::addWatcher(QList<ConfigKey> configKeys, WBaseWidget *pWidget) {
    WSliderComposed *sliderComposed = toSliderComposed(pWidget);
    WPushButton *pushButton = toPushButton(pWidget);

    if (sliderComposed) {
        m_pWatchers.append(new SliderTooltipWatcher(
                &m_pKbdPreset,
                 configKeys,
                 sliderComposed
        ));
    } else if (pushButton) {
        m_pWatchers.append(new PushButtonTooltipWatcher(
                &m_pKbdPreset,
                configKeys,
                pushButton
        ));
    } else {
        m_pWatchers.append(new WidgetTooltipWatcher(
                &m_pKbdPreset,
                configKeys,
                pWidget
        ));
    }
}


void TooltipShortcutUpdater::updateShortcuts(ControllerPresetPointer pPreset) {
    QSharedPointer<KeyboardControllerPreset> pKbdPreset = pPreset.dynamicCast<KeyboardControllerPreset>();
    if (pKbdPreset.isNull()) return;
    m_pKbdPreset = pKbdPreset;

    foreach (WidgetTooltipWatcher* pTooltip, m_pWatchers) {
        pTooltip->update();
    }
}

WSliderComposed * TooltipShortcutUpdater::toSliderComposed(WBaseWidget *pWidget) {
    return qobject_cast<WSliderComposed*>(pWidget->toQWidget());
}

WPushButton * TooltipShortcutUpdater::toPushButton(WBaseWidget *pWidget) {
    return qobject_cast<WPushButton*>(pWidget->toQWidget());
}



//  --------------------------------------------
//  TooltipShortcutUpdater::Tooltip (base class)
//  --------------------------------------------

// TODO(Tomasito) WidgetTooltipWatcher is not a good name, since it's not watching the widget tooltips. It's
// ...            watching the keyboard changes and then updating the widget tooltip. The same goes for
// ...            SliderTooltipWatcher and PushButtonTooltipWatcher

WidgetTooltipWatcher::WidgetTooltipWatcher(KeyboardControllerPresetPointer *ppKbdPreset,
                                           QList<ConfigKey> configKeys, WBaseWidget *pWidget) :
        m_configKeys(configKeys),
        m_ppKbdPreset(ppKbdPreset),
        m_pWidget(pWidget) { }

void WidgetTooltipWatcher::update() {
    m_pTooltipShortcuts = "";
    foreach (ConfigKey configKey, m_configKeys) {
        updateShortcuts(configKey);
    }
    pushShortcutsToWidget();
}

void WidgetTooltipWatcher::addShortcut(const ConfigKey &configKey, const QString &keySuffix, const QString &cmd) {
    ConfigKey subKey(configKey);

    if (!keySuffix.isEmpty()) {
        subKey.item += keySuffix;
    }

    QString shortcut = (*m_ppKbdPreset)->getKeySequencesToString(subKey, ", ");
    if (shortcut.isEmpty()) { return; }

    QString tooltip;

    // translate shortcut to native text
#if QT_VERSION >= 0x040700
    QString nativeShortcut = QKeySequence(shortcut, QKeySequence::PortableText).toString(QKeySequence::NativeText);
#else
    QKeySequence keySec = QKeySequence::fromString(shortcut, QKeySequence::PortableText);
    QString nativeShortcut = keySec.toString(QKeySequence::NativeText);
#endif

    tooltip += "\n";
    tooltip += tr("Shortcut");
    if (!cmd.isEmpty()) {
        tooltip += " ";
        tooltip += cmd;
    }
    tooltip += ": ";
    tooltip += nativeShortcut;

    m_pTooltipShortcuts += tooltip;
}

void WidgetTooltipWatcher::updateShortcuts(const ConfigKey &configKey) {
    // do not add Shortcut string for feedback connections
    addShortcut(configKey, "", "");
}

void WidgetTooltipWatcher::pushShortcutsToWidget() {
    QString baseTooltip = m_pWidget->baseTooltip();
    QWidget* qWidget = m_pWidget->toQWidget();
    qWidget->setToolTip(baseTooltip + m_pTooltipShortcuts);
}



//  --------------------------------------------
//  TooltipShortcutUpdater::SliderTooltipWatcher
//  --------------------------------------------

int SliderTooltipWatcher::HORIZONTAL = 0;
int SliderTooltipWatcher::VERTICAL = 1;

SliderTooltipWatcher::SliderTooltipWatcher(KeyboardControllerPresetPointer *ppKbdPreset,
                                           QList<ConfigKey> &configKeys,
                                           WSliderComposed *pSlider)  :
        WidgetTooltipWatcher(ppKbdPreset, configKeys, pSlider),
        m_direction(getDirection(pSlider)) { }

void SliderTooltipWatcher::updateShortcuts(const ConfigKey &configKey) {
    WidgetTooltipWatcher::updateShortcuts(configKey);

    if (m_direction == HORIZONTAL) {
        addShortcut(configKey, "_up", tr("right"));
        addShortcut(configKey, "_down", tr("left"));
        addShortcut(configKey, "_up_small", tr("right small"));
        addShortcut(configKey, "_down_small", tr("left small"));
    } else if (m_direction == VERTICAL) {
        addShortcut(configKey, "_up", tr("up"));
        addShortcut(configKey, "_down", tr("down"));
        addShortcut(configKey, "_up_small", tr("up small"));
        addShortcut(configKey, "_down_small", tr("down small"));
    }
}

int SliderTooltipWatcher::getDirection(WSliderComposed *pSlider) {
    return pSlider->isHorizontal() ? HORIZONTAL : VERTICAL;
}



//  --------------------------------------------
//     TooltipShortcutUpdater::ButtonTooltip
//  --------------------------------------------

PushButtonTooltipWatcher::PushButtonTooltipWatcher(KeyboardControllerPresetPointer *ppKbdPreset,
                                                   QList<ConfigKey> &configKeys,
                                                   WPushButton *pPushButton) :
        WidgetTooltipWatcher(ppKbdPreset, configKeys, pPushButton) { }

void PushButtonTooltipWatcher::updateShortcuts(const ConfigKey &configKey) {
    WidgetTooltipWatcher::updateShortcuts(configKey);

    addShortcut(configKey, "_activate", tr("activate"));
    addShortcut(configKey, "_toggle", tr("toggle"));
}
