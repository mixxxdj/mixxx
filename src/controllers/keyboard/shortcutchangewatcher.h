#ifndef SHORTCUTCHANGEWATCHER_H
#define SHORTCUTCHANGEWATCHER_H

#include "preferences/configobject.h"

class ShortcutChangeWatcher : public QObject {
    Q_OBJECT
public:
    ShortcutChangeWatcher(QAction* action, ConfigKey configKey, QKeySequence defaultKeySeq);
    virtual ~ShortcutChangeWatcher();

    void updateShortcut(QMultiHash<ConfigValueKbd, ConfigKey> *pHash);
    void restoreDefault();

private:
    QAction* m_pAction;
    ConfigKey m_configKey;
    QKeySequence m_defaultKeySeq;
};


#endif // SHORTCUTCHANGEWATCHER_H
