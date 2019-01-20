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
ShortcutChangeWatcher* KeyboardShortcutsUpdater::getWatcher(ConfigKey configKey) const {
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
    // Iterate over all keyboard shortcuts. If a shortcut is found whose
    // ConfigKey is the same as the config key this watcher is watching,
    // update QAction with new shortcut.
    QMultiHash<QString, ConfigKey>::iterator it;
    for (it = pShortcuts->begin(); it != pShortcuts->end(); ++it) {
        Q_ASSERT(it.value().group == "[KeyboardShortcuts]");
        if (it.value() == m_configKey) {
            const char* keyseq = it.key().toLatin1().data();
            m_pAction->setShortcut(
                    QKeySequence(tr(keyseq))
            );
            return;
        }
    }

    // Restore to default shortcut if none found
    restoreDefault();
}

void ShortcutChangeWatcher::restoreDefault() {
    m_pAction->setShortcut(m_defaultKeySeq);
}
