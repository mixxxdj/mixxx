/**
* @file shortcutchangewatcher.cpp
* @author Jordi Ortolá jordi665@hotmail.com
* @date Fri Jun 17 2016
* @brief Manages shortcuts of a particular QAction, used to refresh shortcuts after kbd preset loaded.
*/

#include <QAction>

#include "shortcutchangewatcher.h"

ShortcutChangeWatcher::ShortcutChangeWatcher(QAction* action, ConfigKey configKey, QKeySequence defaultKeySeq) :
    QObject(action),
    m_pAction(action),
    m_configKey(configKey),
    m_defaultKeySeq(defaultKeySeq) {

    restoreDefault();
}

ShortcutChangeWatcher::~ShortcutChangeWatcher() { }

/* ------------------------------------------------------------------
Purpose: Update shortcut of the bound QAction. The shortcut is set to
         whatever ConfigValueKbd is found in the given MultiHash that
         matches m_configKey.
Input:   QMultiHash<ConfigValueKbd, ConfigKey>, containing mapping
         info.
Output:  -
------------------------------------------------------------------- */
void ShortcutChangeWatcher::updateShortcut(QMultiHash<ConfigValueKbd, ConfigKey>* pShortcuts) {
    // Check if shortcut is found in the given QMultiHash
    bool shortcutFound = false;

    // Iterate over all keyboard shortcuts. If a shortcut is found
    // with the same ConfigKey, the bound KeySequence is bound to
    // the QAction
    QMultiHash<ConfigValueKbd, ConfigKey>::iterator it;
    for (it = pShortcuts->begin(); it != pShortcuts->end(); ++it) {
        Q_ASSERT(it.value().group == "[KeyboardShortcuts]");
        if (it.value() == m_configKey) {
            ConfigValueKbd configValueKbd = it.key();
            m_pAction->setShortcut(configValueKbd.m_qKey);
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

/* ------------------------------------------------------------------
Purpose: Set the shortcut of the bound QAction to the default
         QKeySequence.
Input:   -
Output:  -
------------------------------------------------------------------- */
void ShortcutChangeWatcher::restoreDefault() {
    m_pAction->setShortcut(m_defaultKeySeq);
}