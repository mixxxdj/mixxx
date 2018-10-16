#ifndef PREFERENCES_SETTINGSMANAGER_H
#define PREFERENCES_SETTINGSMANAGER_H

#include <QObject>
#include <QSharedPointer>
#include <QString>

#include "preferences/broadcastsettings.h"
#include "preferences/usersettings.h"

class SettingsManager : public QObject {
    Q_OBJECT
  public:
    SettingsManager(QObject* pParent, const QString& settingsPath);
    virtual ~SettingsManager();

    UserSettingsPointer settings() const {
        return m_pSettings;
    }

    BroadcastSettingsPointer broadcastSettings() const {
        return m_pBroadcastSettings;
    }

    void save() {
        m_pSettings->save();
    }

    bool shouldRescanLibrary() {
        return m_bShouldRescanLibrary;
    }

  private:
    void initializeDefaults();

    UserSettingsPointer m_pSettings;
    bool m_bShouldRescanLibrary;
    BroadcastSettingsPointer m_pBroadcastSettings;
};

#endif /* PREFERENCES_SETTINGSMANAGER_H */
