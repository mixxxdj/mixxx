#pragma once

#include <QDateTime>
#include <QObject>
#include <QStringList>

#include "preferences/usersettings.h"

class BackUpWorker : public QObject {
    Q_OBJECT
  public:
    explicit BackUpWorker(
            UserSettingsPointer config,
            int keepBackups = 5,
            bool upgradeBU = false,
            QObject* parent = nullptr);
    ~BackUpWorker() = default;

  public slots:
    void performBackUp();
    void deleteOldBackUps();
    bool copySettingsToTempDir(const QString& settingsDir, const QString& tempDirPath);

  signals:
    void progressChanged(int percentage);
    void backUpFinished(bool success, const QString& message);
    void errorOccurred(const QString& error);
    void backUpRemoved(const QString& error);

  private:
    UserSettingsPointer m_pConfig;
    int m_keepBackUps;
    bool m_upgradeBU;
    QString currentMixxxVersion;
    bool useBit7z;
};
