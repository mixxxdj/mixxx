/**
* @file shortcutchangewatcher.cpp
* @author Jordi Ortolá jordi665@hotmail.com
* @date Fri Jun 17 2016
* @brief Manages shortcuts of a particular QAction, used to refresh shortcuts after kbd preset loaded.
*/

#ifndef KEYBOARDSHORTCUTSUPDATER_H
#define KEYBOARDSHORTCUTSUPDATER_H

#include <QList>

#include "preferences/configobject.h"
#include "controllers/keyboard/keyboardcontrollerpreset.h"

class ShortcutChangeWatcher;

class KeyboardShortcutsUpdater : public QObject {
    Q_OBJECT
  public:
    KeyboardShortcutsUpdater();
    virtual ~KeyboardShortcutsUpdater();

    // Add a watcher to the watcher list. When added, the
    // watcher will be dealt with in slotUpdateShortcuts()
    void addWatcher(ShortcutChangeWatcher* watcher);

  public slots:
    void slotUpdateShortcuts(KeyboardControllerPresetPointer pKbdPreset);

  private:
    QList<ShortcutChangeWatcher*> m_shortcutChangeWatchers;

    // Returns ShortcutChangeWatcher matching the given configKey, if found
    // in m_shortcutChangeWatchers. If no watcher is found, returns nullptr
    ShortcutChangeWatcher* getWatcher(ConfigKey configKey);
};



class ShortcutChangeWatcher : public QObject {
    Q_OBJECT
  public:
    ShortcutChangeWatcher(QAction* action, ConfigKey configKey, QKeySequence defaultKeySeq);
    virtual ~ShortcutChangeWatcher();

    // Update shortcut of bound QAction. The shortcut is set to whatever
    // ConfigValueKbd is found in the given MultiHash that matches m_configKey
    void updateShortcut(QMultiHash<QString, ConfigKey>* pHash);

    // Set the shortcut of the bound QAction to the default QKeySequence
    void restoreDefault();

    const ConfigKey m_configKey;

  private:
    // The actual action to which the shortcut will be bound
    QAction* const m_pAction;
    const QKeySequence m_defaultKeySeq;
};

#endif //SHORTCUTCONTROLLER_H
