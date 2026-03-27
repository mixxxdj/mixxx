#pragma once

#include <QFileSystemWatcher>
#include <QLocale>
#include <QMultiHash>
#include <QObject>

#include "control/controlobject.h"
#include "preferences/configobject.h"
#include "util/autofilereloader.h"

class ControlObject;
class QEvent;
class QKeyEvent;
class WBaseWidget;
class WSearchLineEdit;

// This class provides handling of keyboard events.
class KeyboardEventFilter : public QObject {
    Q_OBJECT
  public:
    KeyboardEventFilter(UserSettingsPointer pConfig,
            const QLocale& locale,
            QObject* pParent = nullptr);
    virtual ~KeyboardEventFilter();

    bool eventFilter(QObject* obj, QEvent* e);

    std::shared_ptr<ConfigObject<ConfigValueKbd>> getKeyboardConfig() const {
        return m_pKbdConfig;
    };

    // Returns a valid QString with modifier keys from a QKeyEvent
    static QKeySequence getKeySeq(QKeyEvent* e);

    bool isEnabled() {
        return m_enabled;
    }

    void registerShortcutWidget(WBaseWidget* pWidget);
    void connectShowOnlyKbdShortcuts(WBaseWidget* pWidget);
    void updateWidgetShortcuts();
    void clearWidgets();
    QString buildShortcutString(const QString& shortcut, const QString& cmd) const;

    void registerSearchBar(WSearchLineEdit* pSearchBar);
    void updateSearchBarShortcuts();

    void registerMenuBarActionSetShortcut(
            QAction* pAction,
            const ConfigKey& command,
            const QString& defaultShortcut);
    void clearMenuBarActions();
    void updateMenuBarActionShortcuts();

  public slots:
    void setEnabled(bool enabled);
    void setShowOnlyKbdShortcuts(bool enabled);
    void reloadKeyboardConfig();

  signals:
#ifndef __APPLE__
    void altPressedWithoutKeys();
#endif
    // We're only the relay here: CoreServices -> this -> WBaseWidget
    void shortcutsEnabled(bool enabled);
    void showOnlyKbdShortcuts(bool enabled);

  private:
    struct KeyDownInformation {
        KeyDownInformation(int keyId, Qt::KeyboardModifiers modifiers, ControlObject* pControl)
                : keyId(keyId),
                  modifiers(modifiers),
                  pControl(pControl) {
        }

        int keyId;
        Qt::KeyboardModifiers modifiers;
        ControlObject* pControl;
    };

#ifndef __APPLE__
    bool m_altPressedWithoutKey;
#endif

    // Run through list of active keys to see if the pressed key is already active
    // and is not a control that repeats when held.
    bool shouldSkipHeldKey(int keyId) {
        return std::any_of(
                m_qActiveKeyList.cbegin(),
                m_qActiveKeyList.cend(),
                [&](const KeyDownInformation& keyDownInfo) {
                    return keyDownInfo.keyId == keyId && !keyDownInfo.pControl->getKbdRepeatable();
                });
    }

    void createKeyboardConfig();

    QString localizeShortcutKeys(const QString& shortcut) const;

    // List containing keys which is currently pressed
    QList<KeyDownInformation> m_qActiveKeyList;

    UserSettingsPointer m_pConfig;
    // Pointer to keyboard config object
    std::shared_ptr<ConfigObject<ConfigValueKbd>> m_pKbdConfig;
    QLocale m_locale;
    bool m_enabled;
    bool m_showOnlyKeyboardShortcuts;

    AutoFileReloader m_autoReloader;

    // Actions in the menu bar
    // Value pair is the ConfigKey and the default QKeySequence (as QString).
    QHash<QAction*, std::pair<ConfigKey, QString>> m_menuBarActions;

    // Widgets that have mappable connections, registered by LegacySkinParser
    // during skin construction.
    QList<WBaseWidget*> m_widgets;
    WSearchLineEdit* m_pSearchBar;

    // Multi-hash of key sequence to
    QMultiHash<ConfigValueKbd, ConfigKey> m_keySequenceToControlHash;
};
