#include "preferences/BackUp/backupsettings.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QThread>

#include "moc_backupsettings.cpp"
#include "preferences/BackUp/backupworker.h"
#include "preferences/usersettings.h"

// Starts a backup of the Mixxx settings directory if enabled in config.
// Excludes the "analysis" subfolder, and saves the archive to ~/Documents/Mixxx-BackUps.
// When a the config -> version is different then LastMixxxVersionBU
// a BackUp will be created even if BackUp is disabled

const QString kConfigGroup = "[BackUp]";
const QString kBackUpEnabled = "BackUpEnabled";
const QString kBackUpFrequency = "BackUpFrequency";
const QString kLastBackUp = "LastBackUp";
const QString kLastMixxxVersionBU = "LastMixxxVersionBU";
const QString kKeepXBUs = "KeepXBUs";

BackUpSettings::BackUpSettings(
        UserSettingsPointer config,
        QObject* parent)
        : QObject(parent),
          m_pConfig(config) {
    lastMixxxVersionBU = m_pConfig->getValue(ConfigKey(kConfigGroup, kLastMixxxVersionBU));
    currentMixxxVersion = m_pConfig->getValue(ConfigKey("[Config]", "Version"));
    upgradeBU = (lastMixxxVersionBU != currentMixxxVersion);
    startBU = false;
}

void BackUpSettings::startBackUpWorker(UserSettingsPointer config) {
    int keepXBUs = m_pConfig->getValue<int>(ConfigKey(kConfigGroup, kKeepXBUs));
    qDebug() << "[BackUp] -> version upgrade ? " << upgradeBU;

    QThread* thread = new QThread();
    BackUpWorker* worker = new BackUpWorker(config, keepXBUs, upgradeBU);

    worker->moveToThread(thread);

    auto handler = [](bool success, const QString& msg) {
        qDebug() << (success ? "[BackUp] -> Succeeded:" : "[BackUp] -> Failed:") << msg;
    };

    connect(thread, &QThread::started, worker, &BackUpWorker::performBackUp);
    connect(worker, &BackUpWorker::backUpFinished, this, [](bool success, QString path) {
        qDebug() << (success ? "[BackUp] -> Succeeded:" : "[BackUp] -> Failed:") << path;
    });
    connect(worker, &BackUpWorker::progressChanged, this, [](int percent) {
        qDebug() << "[BackUp] -> Creation: " << percent << "%";
    });

    if (!upgradeBU) {
        if (keepXBUs > 0) {
            connect(thread, &QThread::started, worker, &BackUpWorker::deleteOldBackUps);
            connect(worker, &BackUpWorker::backUpRemoved, this, [keepXBUs](QString removedBU) {
                qDebug() << "[BackUp] -> Removing Old BackUp(s) "
                         << removedBU << " (Only " << keepXBUs << " BUs are kept) ";
            });
        }
    }

    connect(worker, &BackUpWorker::backUpFinished, thread, &QThread::quit);
    connect(thread, &QThread::finished, worker, &BackUpWorker::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();
}

void BackUpSettings::createSettingsBackUp(UserSettingsPointer m_pConfig) {
    // default the BackUp is set to enabled
    if (!m_pConfig->exists(ConfigKey(kConfigGroup, kBackUpEnabled))) {
        m_pConfig->set(ConfigKey(kConfigGroup, kBackUpEnabled), ConfigValue((int)1));
    }
    // default the BckUp frequency is set to daily = 1 a day / can also be always
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
    // default 5 BackUps are kept, delete happens after creation
    if (!m_pConfig->exists(ConfigKey(kConfigGroup, kKeepXBUs))) {
        m_pConfig->set(ConfigKey(kConfigGroup, kKeepXBUs), ConfigValue((int)5));
    }

    qDebug() << "[BackUp] -> BackUp enabled: "
             << m_pConfig->getValue<bool>(
                        ConfigKey(kConfigGroup, kBackUpEnabled));
    qDebug() << "[BackUp] -> BackUp frequency: "
             << m_pConfig->getValue(ConfigKey(kConfigGroup, kBackUpFrequency));

    bool startBU = false;
    QDate today = QDate::currentDate();
    qDebug() << "[BackUp] -> today: " << today;

    // enabled?
    if (!m_pConfig->getValue<bool>(ConfigKey(kConfigGroup, kBackUpEnabled))) {
        qDebug() << "[BackUp] -> BackUp disabled in settings.";
        startBU = false;
        // BackUpFrequencies: always or daily
    } else {
        if (m_pConfig->getValue(ConfigKey(kConfigGroup, kBackUpFrequency)) == "daily") {
            QString lastBackUpStr = m_pConfig->getValue(ConfigKey(kConfigGroup, kLastBackUp), "");
            QDate lastDate = QDate::fromString(lastBackUpStr, "yyyyMMdd");
            qDebug() << "[BackUp] -> lastDate: " << lastDate;

            if (lastDate == today) {
                qDebug() << "[BackUp] -> BackUp already performed today. Skipping.";
                startBU = false;
            } else {
                qDebug() << "[BackUp] -> 1st Start of Mixxx today -> BackUp will be created.";
                startBU = true;
            }
        }
        if (m_pConfig->getValue(ConfigKey(kConfigGroup, kBackUpFrequency)) == "always") {
            qDebug() << "[BackUp] -> always when Mixxx starts -> BackUp will be created.";
            startBU = true;
        }
    }

    if (upgradeBU) {
        qDebug() << "[BackUp] -> Version upgrade -> BackUp will be created.";
        startBU = true;
    }

    const QString settingsDir = m_pConfig->getSettingsPath();
    if (!QDir(settingsDir).exists()) {
        qWarning() << "[BackUp] -> Settings directory not found:" << settingsDir;
        startBU = false;
    }
    if (startBU) {
        startBackUpWorker(m_pConfig);
        m_pConfig->setValue(ConfigKey(kConfigGroup, kLastBackUp), today.toString("yyyyMMdd"));
        m_pConfig->set(ConfigKey(kConfigGroup, kLastMixxxVersionBU),
                ConfigValue(currentMixxxVersion));
    }
}
