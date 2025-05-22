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

const QString kConfigGroup = QStringLiteral("[BackUp]");
const QString kBackUpEnabled = QStringLiteral("BackUpEnabled");
const QString kBackUpFrequency = QStringLiteral("BackUpFrequency");
const QString kLastBackUp = QStringLiteral("LastBackUp");
const QString kLastMixxxVersionBU = QStringLiteral("LastMixxxVersionBU");
const QString kKeepXBUs = QStringLiteral("KeepXBUs");

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

void BackUpSettings::startBackUpWorker() {
    int keepXBUs = m_pConfig->getValue<int>(ConfigKey(kConfigGroup, kKeepXBUs));
    // bool backUpSucces = false;
    qDebug() << "[BackUp] -> version upgrade ? " << upgradeBU;

    QThread* thread = new QThread();
    BackUpWorker* worker = new BackUpWorker(m_pConfig, keepXBUs, upgradeBU);

    worker->moveToThread(thread);

    // auto handler = [](bool success, const QString& msg) {
    //     qDebug() << (success ? "[BackUp] -> Succeeded:" : "[BackUp] -> Failed:") << msg;
    // };

    connect(thread, &QThread::started, worker, &BackUpWorker::performBackUp);
    // connect(worker, &BackUpWorker::backUpFinished, this, [&backUpSucces](bool
    // success, QString path) {
    //     qDebug() << (success ? "[BackUp] -> Succeeded:" : "[BackUp] ->
    //     Failed:") << path; backUpSucces = success;
    // });
    connect(worker, &BackUpWorker::progressChanged, this, [](int percent) {
        qDebug() << "[BackUp] -> Creation: " << percent << "%";
    });

    if (!upgradeBU) {
        // qDebug() << "[BackUp] -> BackUp Creation Successful, now deleting old BackUps ";
        connect(worker, &BackUpWorker::backUpFinished, this, [this, keepXBUs, worker]() {
            if (keepXBUs > 0) {
                worker->deleteOldBackUps();
                connect(worker,
                        &BackUpWorker::backUpRemoved,
                        this,
                        [keepXBUs](const QString& removedBU) {
                            qDebug() << "[BackUp] -> Removing Old BackUp(s) "
                                     << removedBU << " (Only " << keepXBUs
                                     << " BUs are kept) ";
                        });
            }
        });
        // qDebug() << "[BackUp] -> Cleaneing up old BackUps finished";
    }

    // if (!upgradeBU) {
    //     if ((keepXBUs > 0) && (backUpSucces)) {
    //         connect(thread, &QThread::started, worker, &BackUpWorker::deleteOldBackUps);
    //         connect(worker, &BackUpWorker::backUpRemoved, this, [keepXBUs](QString removedBU) {
    //             qDebug() << "[BackUp] -> Removing Old BackUp(s) "
    //                      << removedBU << " (Only " << keepXBUs << " BUs are kept) ";
    //         });
    //     }
    // }

    connect(worker, &BackUpWorker::backUpFinished, thread, &QThread::quit);
    connect(thread, &QThread::finished, worker, &BackUpWorker::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();
}

void BackUpSettings::createSettingsBackUp() {
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
        startBackUpWorker();
        m_pConfig->setValue(ConfigKey(kConfigGroup, kLastBackUp), today.toString("yyyyMMdd"));
        m_pConfig->set(ConfigKey(kConfigGroup, kLastMixxxVersionBU),
                ConfigValue(currentMixxxVersion));
    }
}
