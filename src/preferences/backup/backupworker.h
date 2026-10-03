#pragma once

#include <QDateTime>
#include <QObject>
#include <QStringList>

#include "preferences/usersettings.h"

class BackupWorker : public QObject {
    Q_OBJECT
  public:
    explicit BackupWorker(
            UserSettingsPointer config,
            int keepBackups = 5,
            bool upgradeBu = false,
            QObject* parent = nullptr);
    ~BackupWorker() = default;

    static QString resolveDocumentsDir();

  public slots:
    void performBackup();
    void deleteOldBackups();
    bool copySettingsToTempDir(const QString& settingsDir, const QString& tempDirPath);

  signals:
    void progressChanged(int percentage);
    void backupFinished(bool success, const QString& message);
    void errorOccurred(const QString& error);
    void backupRemoved(const QString& error);

  private:
    UserSettingsPointer m_pConfig;
    int m_keepBackups;
    bool m_upgradeBu;
    QString currentMixxxVersion;
    bool useBit7z;
};
