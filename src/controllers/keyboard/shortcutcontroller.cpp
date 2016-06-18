#include "shortcutcontroller.h"
#include "keyboardcontrollerpreset.h"


ShortcutController::ShortcutController() {

}

ShortcutController::~ShortcutController() {

}

void ShortcutController::addWatcher(ShortcutChangeWatcher* watcher) {
    // TODO(Tomasito) Add test to see if there is already a watcher with the same ConfigKey. If there is,
    // ...            replace the watcher with the new one. (maybe QMultiHash<ConfigKey, ShortcutChangeWatcher> ?)

    m_shortcutChangeWatchers.append(watcher);
}

// NOTE(Tomasito): There shouldn't be more than one watchers with the same ConfigKey, but if
// ...             there are, this will return the first one that is found.
ShortcutChangeWatcher* ShortcutController::getWatcher(ConfigKey configKey) {
    foreach (ShortcutChangeWatcher* watcher, m_shortcutChangeWatchers) {
        if (watcher->m_configKey == configKey) {
            return watcher;
        }
    }
    return nullptr;
}

void ShortcutController::slotUpdateShortcuts(ControllerPresetPointer pPreset) {
    QSharedPointer<KeyboardControllerPreset> keyboardPreset = pPreset.dynamicCast<KeyboardControllerPreset>();
    QMultiHash<ConfigValueKbd, ConfigKey> keyboardShortcuts =
            keyboardPreset->getMappingByGroup("[KeyboardShortcuts]");

    QMultiHash<ConfigValueKbd, ConfigKey>::iterator it;
    for (it = keyboardShortcuts.begin(); it != keyboardShortcuts.end(); ++it) {
        qDebug() << it.value();
    }

    foreach (ShortcutChangeWatcher* watcher, m_shortcutChangeWatchers) {
        watcher->updateShortcut(&keyboardShortcuts);
    }
}

