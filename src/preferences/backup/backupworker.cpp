#include "backupworker.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryFile>

#include "moc_backupworker.cpp"

#if defined(Q_OS_WIN)
#include <QSettings>
#endif

namespace {

#if defined(Q_OS_WIN)
// Look up HKLM\SOFTWARE\7-Zip\Path in the Windows registry
QString find7ZipFromRegistry() {
    const QStringList registryKeys = {
            QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\7-Zip"),
            QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\7-Zip"),
    };
    for (const QString& key : registryKeys) {
        QSettings reg(key, QSettings::NativeFormat);
        const QString dir = reg.value(QStringLiteral("Path")).toString();
        if (dir.isEmpty()) {
            continue;
        }
        const QString exe = QDir(dir).filePath(QStringLiteral("7z.exe"));
        if (QFile::exists(exe)) {
            return exe;
        }
    }
    return QString();
}
#endif

#if !defined(Q_OS_MACOS)
QString findExternal7z() {
#if defined(Q_OS_WIN)
    QString exe = QStandardPaths::findExecutable(QStringLiteral("7z"));
    if (!exe.isEmpty()) {
        return exe;
    }
    exe = find7ZipFromRegistry();
    if (!exe.isEmpty()) {
        return exe;
    }
    const QStringList winPaths = {
            QDir::homePath() + QStringLiteral("/scoop/apps/7zip/current/7z.exe"),
            QStringLiteral("C:\\Program Files\\7-Zip\\7z.exe"),
            QStringLiteral("C:\\Program Files (x86)\\7-Zip\\7z.exe")};
    for (const QString& path : winPaths) {
        if (QFile::exists(path)) {
            return path;
        }
    }
    return QString();
#elif defined(Q_OS_LINUX)
    QString exe = QStandardPaths::findExecutable(QStringLiteral("7z"));
    if (!exe.isEmpty()) {
        return exe;
    }
    const QStringList linuxPaths = {
            QStringLiteral("/usr/bin/7z"),
            QStringLiteral("/usr/local/bin/7z"),
            QStringLiteral("/bin/7z")};
    for (const QString& path : linuxPaths) {
        if (QFile::exists(path)) {
            return path;
        }
    }
    return QString();
#else
    return QString();
#endif
}
#endif

const QStringList kExcludedFolders = {
        QStringLiteral("analysis"),
        QStringLiteral("lut"),
        QStringLiteral("samples"),
        QStringLiteral("bpmcurve"),
        QStringLiteral("keycurve"),
        QStringLiteral("fingerprints")};

} // anonymous namespace

BackupWorker::BackupWorker(
        UserSettingsPointer config,
        int keepBackups,
        bool upgradeBu,
        QObject* parent)
        : QObject(parent),
          m_pConfig(config),
          m_keepBackups(keepBackups),
          m_upgradeBu(upgradeBu) {
    currentMixxxVersion = m_pConfig->getValue(ConfigKey("[Config]", "Version"));
    useBit7z = false;
}

// Helper (Windows) to resolve the user's Documents folder.
// Standardized on writing test files to handle localized Win32 aliases (eg NL "Documenten").
QString BackupWorker::resolveDocumentsDir() {
    auto isWritableDir = [](const QString& dir) -> bool {
        if (dir.isEmpty() || !QDir(dir).exists()) {
            return false;
        }
        QTemporaryFile probe(QDir(dir).filePath(QStringLiteral("mixxx-write-test-XXXXXX")));
        return probe.open();
    };

    const QString standard = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (isWritableDir(standard)) {
        return standard;
    }
    qWarning() << "[Backup] -> [BackupWorker] -> DocumentsLocation"
               << standard << "not writable, trying fallbacks";

    const QString homeDocuments = QDir(QDir::homePath()).filePath(QStringLiteral("Documents"));
    if (isWritableDir(homeDocuments)) {
        qWarning() << "[Backup] -> [BackupWorker] -> using" << homeDocuments;
        return homeDocuments;
    }

#if defined(Q_OS_WIN)
    QString systemRoot = QDir::rootPath();
    while (systemRoot.endsWith('/')) {
        systemRoot.chop(1);
    }
    if (systemRoot.isEmpty()) {
        systemRoot = QStringLiteral("C:");
    }
#else
    QString systemRoot = QDir::rootPath();
#endif
    qWarning() << "[Backup] -> [BackupWorker] -> falling back to system disk root:" << systemRoot;
    return systemRoot;
}

bool BackupWorker::copySettingsToTempDir(const QString& settingsDir, const QString& tempDirPath) {
#if defined(Q_OS_WIN)
    const QString robocopyLog = QDir::toNativeSeparators(
            QDir::tempPath() + QStringLiteral("/mixxx-backup-robocopy.log"));

    qDebug() << "[Backup] -> [BackupWorker] -> start creation tempdir (robocopy)";

    QProcess robocopy;
    robocopy.setProcessChannelMode(QProcess::MergedChannels);

    QObject::connect(&robocopy, &QProcess::errorOccurred, [](QProcess::ProcessError e) {
        qWarning() << "[Backup] -> [BackupWorker] -> robocopy process error:" << e;
    });

    QStringList robocopyArgs = {
            QDir::toNativeSeparators(settingsDir),
            QDir::toNativeSeparators(tempDirPath),
            QStringLiteral("/E")};

    for (const QString& folder : kExcludedFolders) {
        robocopyArgs << QStringLiteral("/XD") << folder;
    }

    robocopyArgs << QStringLiteral("/R:3")
                 << QStringLiteral("/W:2")
                 << QStringLiteral("/NP")
                 << QStringLiteral("/NFL")
                 << QStringLiteral("/NDL")
                 << QStringLiteral("/LOG+:") + robocopyLog;

    robocopy.start(QStringLiteral("robocopy"), robocopyArgs);

    if (!robocopy.waitForFinished(300000)) {
        qCritical() << "[Backup] -> [BackupWorker] -> robocopy timed out! See log:" << robocopyLog;
        return false;
    }

    const int rc = robocopy.exitCode();
    if (rc >= 8) {
        qCritical() << "[Backup] -> [BackupWorker] -> robocopy failed with exit code"
                    << rc << "- see log:" << robocopyLog;
        QFile log(robocopyLog);
        if (log.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qCritical().noquote() << log.readAll();
        }
        return false;
    }
    return true;

#elif defined(Q_OS_LINUX)
    qDebug() << "[Backup] -> [BackupWorker] -> start creation tempdir (rsync)";

    QProcess rsync;
    rsync.setProgram(QStringLiteral("rsync"));

    QStringList rsyncArgs = {QStringLiteral("-a")};
    for (const QString& folder : kExcludedFolders) {
        rsyncArgs << QStringLiteral("--exclude=") + folder + QStringLiteral("/");
    }
    rsyncArgs << settingsDir + QStringLiteral("/") << tempDirPath + QStringLiteral("/");

    rsync.setArguments(rsyncArgs);
    rsync.start();
    rsync.waitForFinished();

    qDebug() << "stdout:" << rsync.readAllStandardOutput();
    qDebug() << "stderr:" << rsync.readAllStandardError();

    if (rsync.exitCode() != 0) {
        qCritical() << "[Backup] -> [BackupWorker] -> rsync failed! Exit code:" << rsync.exitCode();
        return false;
    }
    return true;

#else
    // Fallback: Qt File Copy (eg macOS)
    QDirIterator it(settingsDir, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

    while (it.hasNext()) {
        const QString srcPath = it.next();
        const QString relativePath = QDir(settingsDir).relativeFilePath(srcPath);

        bool skip = false;
        for (const QString& folder : kExcludedFolders) {
            if (relativePath.contains(folder + QStringLiteral("/")) ||
                    relativePath.startsWith(folder + QStringLiteral("/")) ||
                    relativePath.endsWith(QStringLiteral("/") + folder)) {
                skip = true;
                break;
            }
        }
        if (skip) {
            continue;
        }

        const QString destPath = QDir(tempDirPath).filePath(relativePath);
        QFileInfo(destPath).dir().mkpath(QStringLiteral("."));
        if (!QFile::copy(srcPath, destPath)) {
            qCritical() << "Failed to copy file:" << srcPath << "to" << destPath;
            return false;
        }
    }
    return true;
#endif
}

void BackupWorker::performBackup() {
    QString backupDir;
    QString archivePath;

    const QString settingsDir = m_pConfig->getSettingsPath();
    const QString timestamp = QDateTime::currentDateTime().toString(
            QStringLiteral("yyyyMMdd-HHmmss"));
    const QString documentsDir = resolveDocumentsDir();

    if (m_upgradeBu) {
        backupDir = QDir(documentsDir).filePath(QStringLiteral("Mixxx-Backups/UpgradeBUs"));
        archivePath = QDir(backupDir).filePath(
                QStringLiteral("MixxxSettings-Upgrade-") + currentMixxxVersion +
                QStringLiteral("-") + timestamp);
    } else {
        backupDir = QDir(documentsDir).filePath(QStringLiteral("Mixxx-Backups"));
        archivePath = QDir(backupDir).filePath(QStringLiteral("MixxxSettings-") + timestamp);
    }

    qDebug() << "[Backup] -> [BackupWorker] -> documentsDir:" << documentsDir;
    qDebug() << "[Backup] -> [BackupWorker] -> backupDir:" << backupDir;

    if (!QDir().mkpath(backupDir)) {
        qCritical() << "[Backup] -> [BackupWorker] -> could not create backup dir:" << backupDir;
        emit backupFinished(false,
                QStringLiteral("Backup failed: could not create %1")
                        .arg(backupDir));
        return;
    }

    emit progressChanged(10);

#if defined(Q_OS_LINUX)
    // If 7z is installed on the system -> use it preferably
    const QString zipExecutable = findExternal7z();
    if (!zipExecutable.isEmpty()) {
        const QString archivePath7zExt = archivePath + QStringLiteral(".7z");
        const QString tempBackupDir = archivePath + QStringLiteral("_temp");

        if (QDir().mkpath(tempBackupDir) && copySettingsToTempDir(settingsDir, tempBackupDir)) {
            QProcess process;
            process.setProcessChannelMode(QProcess::MergedChannels);
            process.start(zipExecutable,
                    {QStringLiteral("a"),
                            QStringLiteral("-t7z"),
                            archivePath7zExt,
                            tempBackupDir + QStringLiteral("/*"),
                            QStringLiteral("-mx=6")});

            if (process.waitForFinished(300000) && process.exitCode() == 0) {
                QDir(tempBackupDir).removeRecursively();
                emit progressChanged(100);
                qDebug() << "[Backup] -> [BackupWorker] -> Linux 7z backup succeeded:"
                         << archivePath7zExt;
                emit backupFinished(true, archivePath7zExt);
                return;
            }
            QDir(tempBackupDir).removeRecursively();
        }
    }

    // Fallback: tar.gz included in system
    const QString archivePathTarGz = archivePath + QStringLiteral(".tar.gz");
    qDebug() << "[Backup] -> [BackupWorker] -> Native tar.gz starting:" << archivePathTarGz;

    QProcess process;
    process.setWorkingDirectory(settingsDir);
    process.setProcessChannelMode(QProcess::MergedChannels);

    QStringList tarArgs = {
            QStringLiteral("-czf"),
            archivePathTarGz};

    for (const QString& folder : kExcludedFolders) {
        tarArgs << (QStringLiteral("--exclude=") + folder);
    }
    tarArgs << QStringLiteral(".");

    process.start(QStringLiteral("tar"), tarArgs);

    if (!process.waitForFinished(300000) || process.exitCode() != 0) {
        qCritical() << "[Backup] -> [BackupWorker] -> tar failed:"
                    << process.readAllStandardOutput();
        emit backupFinished(false, QStringLiteral("Backup failed during tar execution."));
        return;
    }

    emit progressChanged(100);
    qDebug() << "[Backup] -> [BackupWorker] -> Linux tar.gz backup succeeded:" << archivePathTarGz;
    emit backupFinished(true, archivePathTarGz);

#elif defined(Q_OS_MACOS)
    const QString archivePathZipExt = archivePath + QStringLiteral(".zip");
    const QString zipExecutable = QStringLiteral("/usr/bin/zip");

    QStringList zipArgs = {
            QStringLiteral("-r"),
            archivePathZipExt,
            settingsDir};

    for (const QString& folder : kExcludedFolders) {
        zipArgs << QStringLiteral("-x")
                << (settingsDir + QStringLiteral("/") + folder + QStringLiteral("/*"));
    }

    qDebug() << "[Backup] -> [BackupWorker] -> Executing:" << zipExecutable << zipArgs.join(" ");

    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start(zipExecutable, zipArgs);

    if (!process.waitForFinished(300000) || process.exitCode() != 0) {
        qCritical() << "[Backup] -> [BackupWorker] -> MacOS zip failed:"
                    << process.readAllStandardOutput();
        emit backupFinished(false, QStringLiteral("Backup failed during zip execution."));
        return;
    }

    emit progressChanged(100);
    qDebug() << "[Backup] -> [BackupWorker] -> MacOS zip backup succeeded:" << archivePathZipExt;
    emit backupFinished(true, archivePathZipExt);

#elif defined(Q_OS_WIN)
    // If 7z is installed on the system -> use it preferably (search registry, path, Program Files)
    const QString zipExecutable = findExternal7z();
    if (!zipExecutable.isEmpty()) {
        const QString archivePath7zExt = archivePath + QStringLiteral(".7z");
        const QString tempBackupDir = archivePath + QStringLiteral("_temp");

        if (QDir().mkpath(tempBackupDir) && copySettingsToTempDir(settingsDir, tempBackupDir)) {
            QProcess process;
            process.setProcessChannelMode(QProcess::MergedChannels);
            process.start(zipExecutable,
                    {QStringLiteral("a"),
                            QStringLiteral("-t7z"),
                            archivePath7zExt,
                            tempBackupDir + QStringLiteral("/*"),
                            QStringLiteral("-mx=6")});

            if (process.waitForFinished(300000) && process.exitCode() == 0) {
                QDir(tempBackupDir).removeRecursively();
                emit progressChanged(100);
                qDebug() << "[Backup] -> [BackupWorker] -> Windows 7z.exe backup succeeded:"
                         << archivePath7zExt;
                emit backupFinished(true, archivePath7zExt);
                return;
            }
            QDir(tempBackupDir).removeRecursively();
        }
    }

    // Fallback 1: tar.exe for Windows 10 17063+
    const QString archivePathZipExt = archivePath + QStringLiteral(".zip");
    QProcess tarProcess;
    tarProcess.setWorkingDirectory(settingsDir);
    tarProcess.setProcessChannelMode(QProcess::MergedChannels);

    QStringList tarArgs = {
            QStringLiteral("-a"),
            QStringLiteral("-cf"),
            archivePathZipExt};

    for (const QString& folder : kExcludedFolders) {
        tarArgs << (QStringLiteral("--exclude=") + folder);
    }
    tarArgs << QStringLiteral(".");

    tarProcess.start(QStringLiteral("tar.exe"), tarArgs);

    if (tarProcess.waitForFinished(300000) && tarProcess.exitCode() == 0) {
        emit progressChanged(100);
        qDebug() << "[Backup] -> [BackupWorker] -> Windows tar.exe backup succeeded:"
                 << archivePathZipExt;
        emit backupFinished(true, archivePathZipExt);
        return;
    }

    // Fallback 2: PowerShell for Windows 10 before 17063
    const QString tempBackupDir = archivePath + QStringLiteral("_temp");
    if (QDir().mkpath(tempBackupDir) && copySettingsToTempDir(settingsDir, tempBackupDir)) {
        QProcess psProcess;
        psProcess.setProcessChannelMode(QProcess::MergedChannels);

        const QString psCommand = QStringLiteral(
                "Compress-Archive -Path '%1\\*' -DestinationPath '%2' -Force")
                                          .arg(QDir::toNativeSeparators(
                                                       tempBackupDir),
                                                  QDir::toNativeSeparators(
                                                          archivePathZipExt));

        QStringList psArgs = {
                QStringLiteral("-NoProfile"),
                QStringLiteral("-NonInteractive"),
                QStringLiteral("-Command"),
                psCommand};

        psProcess.start(QStringLiteral("powershell.exe"), psArgs);
        if (psProcess.waitForFinished(300000) && psProcess.exitCode() == 0) {
            QDir(tempBackupDir).removeRecursively();
            emit progressChanged(100);
            qDebug() << "[Backup] -> [BackupWorker] -> Windows PowerShell backup succeeded:"
                     << archivePathZipExt;
            emit backupFinished(true, archivePathZipExt);
            return;
        }
        QDir(tempBackupDir).removeRecursively();
    }

    qCritical() << "[Backup] -> [BackupWorker] -> All Windows compression methods failed.";
    emit backupFinished(false,
            QStringLiteral("Backup failed: no supported compression executable available."));
#endif
}

void BackupWorker::deleteOldBackups() {
    const QString backupDir = QDir(resolveDocumentsDir()).filePath(QStringLiteral("Mixxx-Backups"));
    QDir dir(backupDir);
    dir.setNameFilters({QStringLiteral("MixxxSettings-*.7z"),
            QStringLiteral("MixxxSettings-*.tar.gz"),
            QStringLiteral("MixxxSettings-*.zip")});
    dir.setSorting(QDir::Time);

    if (m_keepBackups == 0) {
        qDebug() << "[Backup] -> [BackupWorker] -> keeping all backups (m_keepBackups=0)";
        return;
    }

    const auto backups = dir.entryInfoList();
    for (int i = m_keepBackups; i < backups.size(); ++i) {
        dir.remove(backups[i].fileName());
        emit backupRemoved(backups[i].fileName());
    }
}
