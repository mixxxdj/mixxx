#include <QAction>

#include "controllers/keyboard/keyboardshortcutsupdater.h"

KeyboardShortcutsUpdater::KeyboardShortcutsUpdater() { }
KeyboardShortcutsUpdater::~KeyboardShortcutsUpdater() { }

void KeyboardShortcutsUpdater::addWatcher(ShortcutChangeWatcher* watcher) {
    // TODO(Tomasito) Add test to see if there is already a watcher with the same ConfigKey. If there is,
    // ...            replace the watcher with the new one. (maybe QMultiHash<ConfigKey, ShortcutChangeWatcher> ?)

    m_shortcutChangeWatchers.append(watcher);
}

// NOTE(Tomasito): There shouldn't be more than one watchers with the same ConfigKey, but if
// ...             there are, this will return the first one that is found.
ShortcutChangeWatcher* KeyboardShortcutsUpdater::getWatcher(ConfigKey configKey) {
    for (ShortcutChangeWatcher* watcher: m_shortcutChangeWatchers) {
        if (watcher->m_configKey == configKey) {
            return watcher;
        }
    }
    return nullptr;
}

void KeyboardShortcutsUpdater::slotUpdateShortcuts(KeyboardControllerPresetPointer pKbdPreset) {
    QMultiHash<QString, ConfigKey> keyboardShortcuts =
            pKbdPreset->getMappingByGroup("[KeyboardShortcuts]");

    for (ShortcutChangeWatcher* watcher: m_shortcutChangeWatchers) {
        watcher->updateShortcut(&keyboardShortcuts);
    }
}



ShortcutChangeWatcher::ShortcutChangeWatcher(QAction* action, ConfigKey configKey, QKeySequence defaultKeySeq) :
        QObject(action),
        m_configKey(configKey),
        m_pAction(action),
        m_defaultKeySeq(defaultKeySeq) {

    restoreDefault();
}

ShortcutChangeWatcher::~ShortcutChangeWatcher() { }

void ShortcutChangeWatcher::updateShortcut(QMultiHash<QString, ConfigKey>* pShortcuts) {
    // Check if shortcut is found in the given QMultiHash
    bool shortcutFound = false;

    // Iterate over all keyboard shortcuts. If a shortcut is found
    // with the same ConfigKey, the bound KeySequence is bound to
    // the QAction
    QMultiHash<QString, ConfigKey>::iterator it;
    for (it = pShortcuts->begin(); it != pShortcuts->end(); ++it) {
        Q_ASSERT(it.value().group == "[KeyboardShortcuts]");
        if (it.value() == m_configKey) {
            QString keyseq = it.key();
            m_pAction->setShortcut(keyseq);
            shortcutFound = true;
        }
    }

    // NOTE: We could get rid of shortcutFound and return
    //       when it.value() == m_configKey. But if there
    //       were more than one ConfigValueKbd bound to
    //       our ConfigKey, we want to make sure that the
    //       last one is taken
    if (!shortcutFound) {
        restoreDefault();
    }
}

void ShortcutChangeWatcher::restoreDefault() {
    m_pAction->setShortcut(m_defaultKeySeq);
}