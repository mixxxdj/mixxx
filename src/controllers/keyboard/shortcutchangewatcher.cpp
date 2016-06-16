#include <QAction>

#include "shortcutchangewatcher.h"


ShortcutChangeWatcher::ShortcutChangeWatcher(QAction* action, ConfigKey configKey) :
    QObject(action),
    m_pAction(action),
    m_configKey(configKey) { }

ShortcutChangeWatcher::~ShortcutChangeWatcher() { }


// TODO(Tomasito) If the current shortcut is not the default one defined in WMainMenuBar::initialize()
// ...            and there is no new shortcut defined in pShortcuts for this particular configKey,
// ...            we should set the action shortcut to default again
void ShortcutChangeWatcher::updateShortcut(QMultiHash<ConfigValueKbd, ConfigKey>* pShortcuts) {
    QMultiHash<ConfigValueKbd, ConfigKey>::iterator it;

    // Iterate over all keyboard shortcuts. If a shortcut is found
    // with the same ConfigKey, the bound KeySequence is bound to
    // the QAction
    for (it = pShortcuts->begin(); it != pShortcuts->end(); ++it) {
        Q_ASSERT(it.value().group == "[KeyboardShortcuts]");
        if (it.value() == m_configKey) {
            ConfigValueKbd configValueKbd = it.key();
            m_pAction->setShortcut(configValueKbd.m_qKey);
            return;
        }
    }

    // TODO(Tomasito) Set default shortcut
}