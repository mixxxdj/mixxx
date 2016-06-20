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
#include "controllers/controllerpreset.h"

class ShortcutChangeWatcher;

class KeyboardShortcutsUpdater : public QObject {
    Q_OBJECT
public:
    KeyboardShortcutsUpdater();
    virtual ~KeyboardShortcutsUpdater();

    void addWatcher(ShortcutChangeWatcher* watcher);

public slots:
    void slotUpdateShortcuts(ControllerPresetPointer pPreset);

private:
    QList<ShortcutChangeWatcher*> m_shortcutChangeWatchers;
    ShortcutChangeWatcher* getWatcher(ConfigKey configKey);
};



class ShortcutChangeWatcher : public QObject {
    Q_OBJECT
public:
    ShortcutChangeWatcher(QAction* action, ConfigKey configKey, QKeySequence defaultKeySeq);
    virtual ~ShortcutChangeWatcher();

    void updateShortcut(QMultiHash<ConfigValueKbd, ConfigKey>* pHash);
    void restoreDefault();

    const ConfigKey m_configKey;

private:
    QAction* const m_pAction;
    const QKeySequence m_defaultKeySeq;
};

#endif //SHORTCUTCONTROLLER_H
