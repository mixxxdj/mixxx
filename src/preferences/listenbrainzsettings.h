#pragma once

#include <QObject>
#include <QLineEdit>
#include <QCheckBox>

#include "preferences/usersettings.h"
#include "control/controlproxy.h"

namespace {
    const ConfigKey kListenBrainzEnabled("[Livemetadata]","ListenBrainzEnabled");
    const ConfigKey kListenBrainzUserToken("[Livemetadata]","ListenBrainzUserToken");
    const ConfigKey kListenBrainzSettingsChanged("[Livemetadata]","ListenBrainzSettingsChanged");
    const bool defaultListenBrainzEnabled = false;
}

struct ListenBrainzWidgets {
    QCheckBox *m_pEnabled;
    QLineEdit *m_pUserToken;
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
