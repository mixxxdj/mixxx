#ifndef SHORTCUTCHANGEWATCHER_H
#define SHORTCUTCHANGEWATCHER_H

#include "preferences/configobject.h"

class ShortcutChangeWatcher : public QObject {
    Q_OBJECT
public:
    ShortcutChangeWatcher(QAction* action, ConfigKey configKey);
    virtual ~ShortcutChangeWatcher();

    void updateShortcut(QMultiHash<ConfigValueKbd, ConfigKey> *pHash);

private:
    QAction* m_pAction;
    ConfigKey m_configKey;
};


#endif // SHORTCUTCHANGEWATCHER_H
