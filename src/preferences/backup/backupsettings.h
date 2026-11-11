#pragma once

#include <QDateTime>
#include <QObject>
#include <QStringList>

#include "preferences/usersettings.h"

class BackupWorker;

class BackupSettings : public QObject {
    Q_OBJECT

  public:
    explicit BackupSettings(
            UserSettingsPointer config,
            QObject* parent = nullptr);

    ~BackupSettings() = default;

  public slots:
    void createSettingsBackup();
    void startBackupWorker();

  private:
    UserSettingsPointer m_pConfig;
    QString lastMixxxVersionBU;
    QString currentMixxxVersion;
    bool upgradeBU;
    bool startBU;
};
