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
    // default the BackUp is set to enabled
    if (!m_pConfig->exists(ConfigKey(kConfigGroup, kBackUpEnabled))) {
        m_pConfig->set(ConfigKey(kConfigGroup, kBackUpEnabled), ConfigValue((int)1));
    }
    // default the BckUp frequency is set to always = on every start, can be daily to
    if (!m_pConfig->exists(ConfigKey(kConfigGroup, kBackUpFrequency))) {
        m_pConfig->set(ConfigKey(kConfigGroup, kBackUpFrequency), ConfigValue("always"));
    }
    if (!m_pConfig->exists(ConfigKey(kConfigGroup, kLastBackUp))) {
        m_pConfig->set(ConfigKey(kConfigGroup, kLastBackUp), ConfigValue(""));
    }

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

    QString zipExecutable;
    QStringList arguments;
    QString zipFilePath;

#if defined(Q_OS_MACOS)
    // using zip that is in macOS -> can be called in sandbox
    zipExecutable = "/usr/bin/zip";
    zipFilePath = backupFolder + "/Mixxx-BackUp-" + timestamp + ".zip";

    // the settings directory is added to the BackUp except the analysis folder (can be to big)
    // I thought about excluding the log files to but these can be useful for to find bugs
    // as the logs are constantly overwritten
    arguments << "-r" << zipFilePath << settingsDir << "-x" << settingsDir + "/analysis/*";
#elif defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    // Windows & Linux: use 7z
    zipExecutable = QStandardPaths::findExecutable("7z");
    zipFilePath = backupFolder + "/Mixxx-BackUp-" + timestamp + ".7z";

    // the settings directory is added to the BackUp except the analysis folder (can be to big)
    // I thought about excluding the log files to but these can be useful for to find bugs
    // as the logs are constantly overwritten
    arguments << "a" << "-t7z" << zipFilePath << settingsDir << "-xr!analysis";

// Linux
#if defined(Q_OS_LINUX)
    if (zipExecutable.isEmpty()) {
        const QStringList linuxPaths = {"/usr/bin/7z", "/usr/local/bin/7z", "/bin/7z"};
        for (const QString& path : linuxPaths) {
            if (QFile::exists(path)) {
                zipExecutable = path;
                break;
            }
        }
    }
#endif

// Windows PowerShell fallback
#if defined(Q_OS_WIN)
    if (zipExecutable.isEmpty()) {
        QString psPath = QStandardPaths::findExecutable("powershell");
        if (psPath.isEmpty()) {
            qWarning() << "[BackUp] -> PowerShell not found. Cannot create backup.";
            return;
        }

        zipExecutable = psPath;
        zipFilePath = backupFolder + "/Mixxx-BackUp-" + timestamp + ".zip";

        QString tempBackupPath = QDir::tempPath() + "/mixxx_backup_temp";
        QDir(tempBackupPath).removeRecursively(); // cleanup
        QDir().mkpath(tempBackupPath);

        // Copy the settings directory except the analysis folder (can be too big)
        QDir sourceDir(settingsDir);
        const QStringList entries = sourceDir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries);
        for (const QString& entry : entries) {
            if (entry.compare("analysis", Qt::CaseInsensitive) == 0) {
                continue;
            }
            QString sourcePath = sourceDir.filePath(entry);
            QString destPath = tempBackupPath + "/" + entry;
            QFileInfo entryInfo(sourcePath);
            if (entryInfo.isDir()) {
                QDir().mkpath(destPath);
                QDir subDir(sourcePath);
                for (const QFileInfo& subEntry : subDir.entryInfoList(
                             QDir::NoDotAndDotDot | QDir::AllEntries)) {
                    QFile::copy(subEntry.absoluteFilePath(), destPath + "/" + subEntry.fileName());
                }
            } else {
                QFile::copy(sourcePath, destPath);
            }
        }

        // PowerShell script -> temp file
        QString psScriptPath = QDir::tempPath() + "/mixxx_backup.ps1";
        QFile psScript(psScriptPath);
        if (psScript.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&psScript);
            out << "$ErrorActionPreference = 'Stop'\n";
            out << "Compress-Archive -Path \"" << tempBackupPath << "\\*\" "
                << "-DestinationPath \"" << zipFilePath << "\" -Force\n";
            out << "if (!(Test-Path \"" << zipFilePath << "\")) {\n";
            out << "    Write-Error \"Backup failed: Archive not created.\"\n";
            out << "    exit 1\n";
            out << "}\n";
            psScript.close();
        } else {
            qWarning() << "[BackUp] -> Failed to write PowerShell script.";
            return;
        }

        arguments.clear();
        arguments << "-NoProfile" << "-ExecutionPolicy" << "Bypass"
                  << "-File" << psScriptPath;
    }
#endif

#endif

    // where is the 7z we use?
    if (zipExecutable.isEmpty()) {
        qWarning() << "[BackUp] -> 7z/Zip not found in PATH or fallback locations. "
                      "Cannot create backup.";
        return;
    } else {
        qDebug() << "[BackUp] -> 7z/Zip found in: " << zipExecutable;
        qDebug() << "[BackUp] -> Executing:" << zipExecutable << arguments.join(" ");
    }

    bool started = QProcess::startDetached(zipExecutable, arguments);
    if (started) {
        qDebug() << "[BackUp] -> Settings backup started to:" << zipFilePath;
        m_pConfig->setValue(ConfigKey(kConfigGroup, kLastBackUp), today.toString("yyyyMMdd"));
    } else {
        qWarning() << "[BackUp] -> Failed to launch 7z backup process.";
    }
}
