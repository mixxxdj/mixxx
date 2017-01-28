#ifndef PREFERENCES_SETTINGSMANAGER_H
#define PREFERENCES_SETTINGSMANAGER_H

#include <QObject>
#include <QString>

#include "preferences/usersettings.h"

class SettingsManager : public QObject {
    Q_OBJECT
  public:
    SettingsManager(QObject* pParent, const QString& settingsPath);
    virtual ~SettingsManager();

    UserSettingsPointer settings() const {
        return m_pSettings;
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
};


#endif /* PREFERENCES_SETTINGSMANAGER_H */
