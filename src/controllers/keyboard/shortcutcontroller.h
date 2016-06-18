#ifndef SHORTCUTCONTROLLER_H
#define SHORTCUTCONTROLLER_H

#include <QList>

#include "shortcutchangewatcher.h"
#include "controllers/controllerpreset.h"

class ShortcutController : public QObject {
    Q_OBJECT
public:
    ShortcutController();
    virtual ~ShortcutController();

    void addWatcher(ShortcutChangeWatcher* watcher);

public slots:
    void slotUpdateShortcuts(ControllerPresetPointer pPreset);

private:
    QList<ShortcutChangeWatcher*> m_shortcutChangeWatchers;
    ShortcutChangeWatcher* getWatcher(ConfigKey configKey);
};


#endif //SHORTCUTCONTROLLER_H
