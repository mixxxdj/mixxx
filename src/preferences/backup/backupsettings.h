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
    void createSettingsBackUp();
    void startBackUpWorker();

  private:
    UserSettingsPointer m_pConfig;
    QString lastMixxxVersionBU;
    QString currentMixxxVersion;
    bool upgradeBU;
    bool startBU;
};
