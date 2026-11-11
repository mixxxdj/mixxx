#include "preferences/backup/backupsettings.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QThread>

#include "moc_backupsettings.cpp"
#include "preferences/backup/backupworker.h"
#include "preferences/usersettings.h"

// Starts a backup of the Mixxx settings directory if enabled in config.
// Excludes the "analysis" subfolder, and saves the archive to ~/Documents/Mixxx-BackUps.
// When a the config -> version is different then LastMixxxVersionBU
// a BackUp will be created even if BackUp is disabled

const QString kConfigGroup = QStringLiteral("[Backup]");
const QString kBackUpEnabled = QStringLiteral("BackupEnabled");
const QString kBackUpFrequency = QStringLiteral("BackupFrequency");
const QString kLastBackUp = QStringLiteral("LastBackup");
const QString kLastMixxxVersionBU = QStringLiteral("LastMixxxVersionBU");
const QString kKeepXBUs = QStringLiteral("KeepXBUs");

BackupSettings::BackupSettings(
        UserSettingsPointer config,
        QObject* parent)
        : QObject(parent),
          m_pConfig(config) {
    lastMixxxVersionBU = m_pConfig->getValue(ConfigKey(kConfigGroup, kLastMixxxVersionBU));
    currentMixxxVersion = m_pConfig->getValue(ConfigKey("[Config]", "Version"));
    upgradeBU = (lastMixxxVersionBU != currentMixxxVersion);
    startBU = false;
}

void BackupSettings::startBackupWorker() {
    int keepXBUs = m_pConfig->getValue<int>(ConfigKey(kConfigGroup, kKeepXBUs));
    qDebug() << "[Backup] -> version upgrade ? " << upgradeBU;

    QThread* thread = new QThread();
    BackupWorker* worker = new BackupWorker(m_pConfig, keepXBUs, upgradeBU);

    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &BackupWorker::performBackup);
    connect(worker, &BackupWorker::progressChanged, this, [](int percent) {
        qDebug() << "[Backup] -> Creation: " << percent << "%";
    });

    //    If keepXBUs == 0, we simply do NOT connect deleteOldBackups()
    //    This keeps ALL backups intact without canceling the creation of new ones!
    if (!upgradeBU && keepXBUs > 0) {
        connect(worker, &BackupWorker::backupFinished, this, [this, keepXBUs, worker]() {
            worker->deleteOldBackups();
            connect(worker,
                    &BackupWorker::backupRemoved,
                    this,
                    [keepXBUs](const QString& removedBU) {
                        qDebug() << "[Backup] -> Removing Old Backup(s) "
                                 << removedBU << " (Only " << keepXBUs
                                 << " BUs are kept) ";
                    });
        });
    } else if (keepXBUs == 0) {
        qDebug() << "[Backup] -> keepXBUs=0: Keeping all historical backups.";
    }

    connect(worker, &BackupWorker::backupFinished, thread, &QThread::quit);
    connect(thread, &QThread::finished, worker, &BackupWorker::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();
}

void BackupSettings::createSettingsBackup() {
    // Default configuration settings if missing
    if (!m_pConfig->exists(ConfigKey(kConfigGroup, kBackUpEnabled))) {
        m_pConfig->set(ConfigKey(kConfigGroup, kBackUpEnabled), ConfigValue((int)1));
    }
    if (!m_pConfig->exists(ConfigKey(kConfigGroup, kBackUpFrequency))) {
        m_pConfig->set(ConfigKey(kConfigGroup, kBackUpFrequency), ConfigValue("daily"));
    }
    if (!m_pConfig->exists(ConfigKey(kConfigGroup, kLastBackUp))) {
        m_pConfig->set(ConfigKey(kConfigGroup, kLastBackUp), ConfigValue(""));
    }
    if (!m_pConfig->exists(ConfigKey(kConfigGroup, kLastMixxxVersionBU))) {
        m_pConfig->set(ConfigKey(kConfigGroup, kLastMixxxVersionBU),
                ConfigValue(currentMixxxVersion));
    }
    if (!m_pConfig->exists(ConfigKey(kConfigGroup, kKeepXBUs))) {
        m_pConfig->set(ConfigKey(kConfigGroup, kKeepXBUs), ConfigValue((int)0));
    }

    // Refresh version states after defaults check
    currentMixxxVersion = m_pConfig->getValue(ConfigKey("[Config]", "Version"));
    lastMixxxVersionBU = m_pConfig->getValue(ConfigKey(kConfigGroup, kLastMixxxVersionBU));
    upgradeBU = (lastMixxxVersionBU != currentMixxxVersion);

    qDebug() << "[Backup] -> Backup enabled: "
             << m_pConfig->getValue<bool>(ConfigKey(kConfigGroup, kBackUpEnabled));
    qDebug() << "[Backup] -> Backup frequency: "
             << m_pConfig->getValue(ConfigKey(kConfigGroup, kBackUpFrequency));

    bool startBU = false;
    QDate today = QDate::currentDate();
    qDebug() << "[Backup] -> today: " << today;

    if (m_pConfig->getValue<bool>(ConfigKey(kConfigGroup, kBackUpEnabled))) {
        QString frequency = m_pConfig->getValue(ConfigKey(kConfigGroup, kBackUpFrequency));

        if (frequency == "always") {
            qDebug() << "[Backup] -> Frequency 'always': creating backup on Mixxx start.";
            startBU = true;
        } else if (frequency == "daily") {
            QString lastBackUpStr = m_pConfig->getValue(ConfigKey(kConfigGroup, kLastBackUp), "");
            QDate lastDate = QDate::fromString(lastBackUpStr, "yyyyMMdd");
            qDebug() << "[Backup] -> lastDate: " << lastDate;

            if (lastDate == today) {
                qDebug() << "[Backup] -> Backup already performed today. Skipping.";
                startBU = false;
            } else {
                qDebug() << "[Backup] -> 1st start of Mixxx today: creating backup.";
                startBU = true;
            }
        }
    } else {
        qDebug() << "[Backup] -> Backup disabled in settings.";
    }

    // Version upgrade always overrides and forces a backup
    if (upgradeBU) {
        qDebug() << "[Backup] -> Version upgrade detected: forcing backup.";
        startBU = true;
    }

    const QString settingsDir = m_pConfig->getSettingsPath();
    if (!QDir(settingsDir).exists()) {
        qWarning() << "[Backup] -> Settings directory not found:" << settingsDir;
        startBU = false;
    }

    if (startBU) {
        const QString documentsDir = BackupWorker::resolveDocumentsDir();
        qDebug() << "[Backup] -> Resolved backup base directory:" << documentsDir;
        if (documentsDir.isEmpty() ||
                !QDir().mkpath(documentsDir + "/Mixxx-Backups")) {
            qWarning() << "[Backup] -> Cannot create backup destination under:"
                       << documentsDir << "- skipping backup.";
            startBU = false;
        }
    }

    if (startBU) {
        startBackupWorker();
        m_pConfig->setValue(ConfigKey(kConfigGroup, kLastBackUp), today.toString("yyyyMMdd"));
        m_pConfig->set(ConfigKey(kConfigGroup, kLastMixxxVersionBU),
                ConfigValue(currentMixxxVersion));
    }
}
