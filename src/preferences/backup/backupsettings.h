#pragma once

#include <QDateTime>
#include <QObject>
#include <QStringList>

#include "preferences/usersettings.h"

class BackUpWorker;

class BackUpSettings : public QObject {
    Q_OBJECT

  public:
    explicit BackUpSettings(
            UserSettingsPointer config,
            QObject* parent = nullptr);

    ~BackUpSettings() = default;

  public slots:
    void createSettingsBackUp(UserSettingsPointer m_pConfig);
    void startBackUpWorker(UserSettingsPointer config);

  private:
    UserSettingsPointer m_pConfig;
    QString lastMixxxVersionBU;
    QString currentMixxxVersion;
    bool upgradeBU;
    bool startBU;
};
