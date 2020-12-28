#pragma once

#include <QCheckBox>
#include <QLineEdit>
#include <QObject>

#include "control/controlproxy.h"
#include "preferences/usersettings.h"


struct ListenBrainzWidgets {
    QCheckBox* m_pEnabled;
    QLineEdit* m_pUserToken;
};

struct ListenBrainzSettings {
    bool enabled;
    QString userToken;
};

class ListenBrainzSettingsManager : public QObject {
    Q_OBJECT
  public:
    ListenBrainzSettingsManager(UserSettingsPointer pSettings, const ListenBrainzWidgets& widgets);
    static ListenBrainzSettings getPersistedSettings(UserSettingsPointer pSettings);
    static ListenBrainzSettings getLatestSettings();
    void applySettings();
    void cancelSettings();
    void setSettingsToDefault();

    static const ConfigKey kListenBrainzSettingsChanged;

  private:
    void setUpWidgets();
    bool settingsDifferent();
    bool settingsCorrect();
    void updateLatestSettingsAndNotify();
    void persistSettings();
    void resetSettingsToDefault();

  private:
    ListenBrainzWidgets m_widgets;
    UserSettingsPointer m_pUserSettings;
    static ListenBrainzSettings s_latestSettings;
    ControlProxy m_CPSettingsChanged;
};
