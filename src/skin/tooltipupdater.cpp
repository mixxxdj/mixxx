#include <widget/wslidercomposed.h>
#include <widget/wpushbutton.h>
#include "tooltipupdater.h"
#include "controllers/keyboard/keyboardcontrollerpreset.h"

TooltipShortcutUpdater::TooltipShortcutUpdater() { }

TooltipShortcutUpdater::~TooltipShortcutUpdater() { }

void TooltipShortcutUpdater::addWatcher(ConfigKey configKey, WBaseWidget *pWidget) {
    WSliderComposed *sliderComposed = toSliderComposed(pWidget);
    WPushButton *pushButton = toPushButton(pWidget);

    if (sliderComposed) {
        m_pWatchers.append(new SliderTooltipWatcher(
                &m_pKbdPreset,
                 configKey,
                 sliderComposed
        ));
    } else if (pushButton) {
        m_pWatchers.append(new PushButtonTooltipWatcher(
                &m_pKbdPreset,
                configKey,
                pushButton
        ));
    } else {
        m_pWatchers.append(new WidgetTooltipWatcher(
                &m_pKbdPreset,
                configKey,
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
                                 ConfigKey configKey, WBaseWidget *pWidget) :
        m_ConfigKey(configKey),
        m_ppKbdPreset(ppKbdPreset),
        m_pWidget(pWidget) { }


void WidgetTooltipWatcher::setTooltipShortcut(const QString &keySuffix, const QString &cmd) {
    ConfigKey configKey = m_ConfigKey;

    if (!keySuffix.isEmpty()) {
        configKey.item += keySuffix;
    }

    QString shortcut = (*m_ppKbdPreset)->getKeySequencesToString(configKey, ", ");

    if (shortcut.isEmpty()) {
        return;
    }

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
    m_pWidget->appendBaseTooltip(tooltip);
}



QString WidgetTooltipWatcher::getKeyString(KeyboardControllerPresetPointer kbdPreset, ConfigKey configKey) {
    QList<QKeySequence> keySequences = kbdPreset->getKeySequences(configKey);

    if (keySequences.isEmpty()) {
        return QString("");
    }
    return keySequences[0].toString();
}

void WidgetTooltipWatcher::update() {
    // do not add Shortcut string for feedback connections
    setTooltipShortcut("", "");
}



//  --------------------------------------------
//  TooltipShortcutUpdater::SliderTooltipWatcher
//  --------------------------------------------

int SliderTooltipWatcher::HORIZONTAL = 0;
int SliderTooltipWatcher::VERTICAL = 1;

SliderTooltipWatcher::SliderTooltipWatcher(KeyboardControllerPresetPointer *ppKbdPreset,
                                                     const ConfigKey &configKey,
                                                     WSliderComposed *pSlider)  :
        WidgetTooltipWatcher(ppKbdPreset, configKey, pSlider),
        m_direction(getDirection(pSlider)) { }

void SliderTooltipWatcher::update() {
    WidgetTooltipWatcher::update();

    if (m_direction == HORIZONTAL) {
        setTooltipShortcut("_up", tr("right"));
        setTooltipShortcut("_down", tr("left"));
        setTooltipShortcut("_up_small", tr("right small"));
        setTooltipShortcut("_down_small", tr("left small"));
    } else if (m_direction == VERTICAL) {
        setTooltipShortcut("_up", tr("up"));
        setTooltipShortcut("_down", tr("down"));
        setTooltipShortcut("_up_small", tr("up small"));
        setTooltipShortcut("_down_small", tr("down small"));
    }
}

int SliderTooltipWatcher::getDirection(WSliderComposed *pSlider) {
    return pSlider->isHorizontal() ? HORIZONTAL : VERTICAL;
}




//  --------------------------------------------
//     TooltipShortcutUpdater::ButtonTooltip
//  --------------------------------------------

PushButtonTooltipWatcher::PushButtonTooltipWatcher(KeyboardControllerPresetPointer *ppKbdPreset,
                                                             const ConfigKey &configKey,
                                                             WPushButton *pPushButton) :
        WidgetTooltipWatcher(ppKbdPreset, configKey, pPushButton) { }

void PushButtonTooltipWatcher::update() {
    WidgetTooltipWatcher::update();

    setTooltipShortcut("_activate", tr("activate"));
    setTooltipShortcut("_toggle", tr("toggle"));
}
