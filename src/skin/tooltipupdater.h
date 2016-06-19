#ifndef TOOLTIPUPDATER_H
#define TOOLTIPUPDATER_H

#include "controllers/controllerpreset.h"
#include "preferences/configobject.h"

class WBaseWidget;
class WSliderComposed;
class WPushButton;
class KeyboardPresetChangeWatcher;
class KeyboardControllerPreset;
class WidgetTooltipWatcher;

typedef QSharedPointer<KeyboardControllerPreset> KeyboardControllerPresetPointer;

class TooltipShortcutUpdater : public QObject {
    Q_OBJECT
public:
    TooltipShortcutUpdater();
    virtual ~TooltipShortcutUpdater();

    void addWatcher(ConfigKey configKey, WBaseWidget* pWidget);

public slots:
    void updateShortcuts(ControllerPresetPointer pPreset);

private:
    QList<WidgetTooltipWatcher*> m_pWatchers;
    KeyboardControllerPresetPointer m_pKbdPreset;
    WSliderComposed * toSliderComposed(WBaseWidget *pWidget);
    WPushButton * toPushButton(WBaseWidget *pWidget);
};



// One instance of TooltipShortcutUpdater::Tooltip for each widget that
// has a tooltip, containing info about a keyboard shortcut.
class WidgetTooltipWatcher : public QObject {
    Q_OBJECT
public:
    WidgetTooltipWatcher(KeyboardControllerPresetPointer *ppKbdPreset, ConfigKey configKey, WBaseWidget *pWidget);
    virtual void update();

protected:
    ConfigKey m_ConfigKey;
    KeyboardControllerPresetPointer* m_ppKbdPreset;
    void setTooltipShortcut(const QString &keySuffix, const QString &cmd);
    QString getKeyString(QSharedPointer<KeyboardControllerPreset>, ConfigKey configKey);

private:
    WBaseWidget* m_pWidget;
};



// For slider widgets, the shortcut info will display: "shortcut up:", "shortcut down:", etc.
class SliderTooltipWatcher : public WidgetTooltipWatcher {
    Q_OBJECT
public:
    static int HORIZONTAL, VERTICAL;
    int m_direction;

    SliderTooltipWatcher(KeyboardControllerPresetPointer *ppKbdPreset,
                  const ConfigKey &configKey,
                  WSliderComposed *pSlider);
    virtual void update() override;

private:
    int getDirection(WSliderComposed* pSlider);
};



// For pushbutton widgets, the shortcut info will display: "shortcut activate:" or "shortcut toggle"
class PushButtonTooltipWatcher : public WidgetTooltipWatcher {
    Q_OBJECT
public:
    PushButtonTooltipWatcher(KeyboardControllerPresetPointer *ppKbdPreset,
                      const ConfigKey &configKey,
                      WPushButton *pPushButton);
    virtual void update() override;
};



#endif //TOOLTIPUPDATER_H
