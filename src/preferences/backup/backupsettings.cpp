#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QStandardPaths>

#include "preferences/usersettings.h"

const QString kConfigGroup = "[BackUp]";
const QString kBackUpEnabled = "BackUpEnabled";
const QString kBackUpFrequency = "BackUpFrequency";
const QString kLastBackUp = "LastBackUp";

void createSettingsBackUp(UserSettingsPointer m_pConfig) {

    qDebug() << "[BackUp] -> BackUp enabled: "
             << m_pConfig->getValue<bool>(
                        ConfigKey(kConfigGroup, kBackUpEnabled));
    qDebug() << "[BackUp] -> BackUp frequency: "
             << m_pConfig->getValue(ConfigKey(kConfigGroup, kBackUpFrequency));

    // enabled?
    if (!m_pConfig->getValue<bool>(ConfigKey(kConfigGroup, kBackUpEnabled))) {
        qDebug() << "[BackUp] -> BackUp disabled in settings.";
        return;
    }

    // BackUpFrequencies: always or daily
    QDate today = QDate::currentDate();
    qDebug() << "[BackUp] -> today: " << today;

    if (m_pConfig->getValue(ConfigKey(kConfigGroup, kBackUpFrequency)) == "daily") {
        QString lastBackupStr = m_pConfig->getValue(ConfigKey(kConfigGroup, kLastBackUp), "");
        QDate lastDate = QDate::fromString(lastBackupStr, "yyyyMMdd");
        qDebug() << "[BackUp] -> lastDate: " << lastDate;

        if (lastDate == today) {
            qDebug() << "[BackUp] -> BackUp already performed today. Skipping.";
            return;
        }
    }

    QString settingsDir = m_pConfig->getSettingsPath();
    if (!QDir(settingsDir).exists()) {
        qWarning() << "[BackUp] -> Settings directory not found:" << settingsDir;
        return;
    }

    QString documentsDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString backupFolder = documentsDir + "/Mixxx-BackUps";
    QDir().mkpath(backupFolder);

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
    QString zipFilePath = backupFolder + "/Mixxx-BackUp-" + timestamp + ".7z";

    QString zipExecutable;
#if defined(Q_OS_WIN)
    zipExecutable = "7z.exe";
#else
    zipExecutable = "7z";
#endif

    if (QStandardPaths::findExecutable(zipExecutable).isEmpty()) {
        qWarning() << "[BackUp] -> 7z not found in PATH. Cannot create backup.";
        return;
    }

    // the settings directory is added to the BackUp except the analysis folder (can be to big)
    QStringList arguments;
    arguments << "a"
              << "-t7z"
              << zipFilePath
              << settingsDir
              << "-xr!analysis";

    bool started = QProcess::startDetached(zipExecutable, arguments);
    if (started) {
        qDebug() << "[BackUp] -> Settings backup started to:" << zipFilePath;
        m_pConfig->setValue(ConfigKey(kConfigGroup, kLastBackUp), today.toString("yyyyMMdd"));
    } else {
        qWarning() << "[BackUp] -> Failed to launch 7z backup process.";
    }
}
