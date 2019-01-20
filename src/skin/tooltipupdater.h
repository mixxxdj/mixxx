#ifndef TOOLTIPUPDATER_H
#define TOOLTIPUPDATER_H

#include "controllers/keyboard/keyboardcontrollerpreset.h"
#include "preferences/configobject.h"

class WBaseWidget;
class WSliderComposed;
class WPushButton;
class KeyboardPresetChangeWatcher;
class KeyboardControllerPreset;
class WidgetTooltipWatcher;

class TooltipShortcutUpdater : public QObject {
    Q_OBJECT

  public:
    TooltipShortcutUpdater();
    virtual ~TooltipShortcutUpdater();
    void addWatcher(WidgetTooltipWatcher* tooltipWatcher);

  public slots:
    void updateShortcuts(KeyboardControllerPresetPointer pPreset);

  private:
    QList<WidgetTooltipWatcher*> m_pWatchers;
    KeyboardControllerPresetPointer m_pKbdPreset;
};



// One instance of TooltipShortcutUpdater::Tooltip for each widget that
// has a tooltip, containing info about a keyboard shortcut.
class WidgetTooltipWatcher : public QObject {
    Q_OBJECT
  public:
    WidgetTooltipWatcher(WBaseWidget* pWidget, QList<ConfigKey> configKeys,
                             KeyboardControllerPresetPointer* ppKbdPreset);
    void update();
    inline void setKeyboardPreset(KeyboardControllerPresetPointer* ppKbdPreset) {
        m_ppKbdPreset = ppKbdPreset;
    }

  protected:
    const QList<ConfigKey> m_configKeys;

    // A pointer to QSHaredPointer<KeyboardControllerPreset>, stored in TooltipShortcutUpdater
    KeyboardControllerPresetPointer* m_ppKbdPreset;

    // Updates m_pTooltipShortcuts, but do not update widget
    virtual void updateShortcuts(const ConfigKey& configKey);
    void addShortcut(const ConfigKey& configKey, const QString& keySuffix, const QString& cmd);

    // Call when m_pTooltipShortcuts are updated to push
    // the shortcut information to the widget tooltip
    void pushShortcutsToWidget();

  private:
    WBaseWidget* m_pWidget;
    QString m_pTooltipShortcuts;
};



// For slider widgets, the shortcut info will display: "shortcut up:", "shortcut down:", etc.
class SliderTooltipWatcher : public WidgetTooltipWatcher {
    Q_OBJECT
  public:
    static const int HORIZONTAL = 0;
    static const int VERTICAL = 1;

    int m_direction;

    SliderTooltipWatcher(WSliderComposed* pSlider, QList<ConfigKey>& configKey,
                             KeyboardControllerPresetPointer* ppKbdPreset);
    virtual void updateShortcuts(const ConfigKey& configKey) override;

  private:
    int getDirection(WSliderComposed* pSlider);
};



// For pushbutton widgets, the shortcut info will display: "shortcut activate:" or "shortcut toggle"
class PushButtonTooltipWatcher : public WidgetTooltipWatcher {
    Q_OBJECT
  public:
    PushButtonTooltipWatcher(WPushButton* pPushButton, QList<ConfigKey>& configKeys,
                                 KeyboardControllerPresetPointer* ppKbdPreset);
    virtual void updateShortcuts(const ConfigKey& configKeys) override;
};



#endif //TOOLTIPUPDATER_H
