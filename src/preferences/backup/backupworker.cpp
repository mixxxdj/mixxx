#include "backupworker.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
// Needs to be uncommented when bit7z and 7z are in the dependencies
// and th parts in the CMakeLists are uncommented too.
#include <bit7z/bit7z.hpp>

#include "moc_backupworker.cpp"
// #include "registercodecs.cpp"
// #include "registerhashers.cpp"

// extern "C" void RegisterCodecs();
// extern "C" void RegisterHashers();

BackUpWorker::BackUpWorker(
        UserSettingsPointer config,
        int keepBackUps,
        bool upgradeBU,
        QObject* parent)
        : QObject(parent),
          m_pConfig(config),
          m_keepBackUps(keepBackUps),
          m_upgradeBU(upgradeBU) {
    currentMixxxVersion = m_pConfig->getValue(ConfigKey("[Config]", "Version"));
    useBit7z = false;
}

bool BackUpWorker::copySettingsToTempDir(const QString& settingsDir, const QString& tempDirPath) {
#if defined(Q_OS_WIN)
    QProcess robocopy;
    qDebug() << "[BackUp] -> [BAckUpWorker] -> start creation tempdir robocopy/rsync";
    robocopy.start("robocopy",
            {QDir::toNativeSeparators(settingsDir),
                    QDir::toNativeSeparators(tempDirPath),
                    "/E", // show dirs
                    "/XD",
                    "analysis", // exclude analysis
                    "/XD",
                    "lut",     // exclude timecode lut
                    "/R:3",    // retry 3 times if file is locked
                    "/W:2",    // wait 2 seconds between retries
                    "/NP",     // progress display off
                    "/LOG+:" + // write log file -> can be quoted later
                            QDir::toNativeSeparators(
                                    tempDirPath + "_robocopy.log")});

    if (!robocopy.waitForFinished(30000) || robocopy.exitCode() >= 8) {
        qCritical()
                << "[BackUp] -> [BAckUpWorker] -> File copy failed! Check log:"
                << tempDirPath + "_robocopy.log";
        return false;
    }
    return true;

#elif defined(Q_OS_LINUX)
    QProcess rsync;
    rsync.setProgram("rsync");
    rsync.setArguments({"-a",
            "--exclude=analysis/", // exclude analysis
            "--exclude=lut/",      // exclude timecode lut
            settingsDir + "/",
            tempDirPath + "/"});
    rsync.start();
    rsync.waitForFinished();

    qDebug() << "stdout:" << rsync.readAllStandardOutput();
    qDebug() << "stderr:" << rsync.readAllStandardError();

    if (rsync.exitCode() != 0) {
        qCritical() << "[BackUp] -> [BackUpWorker] -> rsync failed! Exit code:" << rsync.exitCode();
        return false;
    }
    return true;
#else
    // For macOS or other platforms, use Qt's file copy
    QDirIterator it(settingsDir,
            QDir::Files | QDir::NoDotAndDotDot,
            QDirIterator::Subdirectories);

    while (it.hasNext()) {
        QString srcPath = it.next();
        QString relativePath = QDir(settingsDir).relativeFilePath(srcPath);

        // Skip analysis folders at any level
        if (relativePath.contains("analysis/") ||
                relativePath.startsWith("analysis/") ||
                relativePath.endsWith("/analysis")) {
            continue;
        }
        // Skip lut folders at any level
        if (relativePath.contains("lut/") ||
                relativePath.startsWith("lut/") ||
                relativePath.endsWith("/lut")) {
            continue;
        }

        QString destPath = tempDirPath + "/" + relativePath;
        QFileInfo(destPath).dir().mkpath(".");
        if (!QFile::copy(srcPath, destPath)) {
            qCritical() << "Failed to copy file:" << srcPath << "to" << destPath;
            return false;
        }
    }
    return true;
#endif
}

void BackUpWorker::performBackUp() {
    QString backupDir, archivePath, archivePath7zExt, archivePathZipExt, zipExecutable;
    const QString settingsDir = m_pConfig->getSettingsPath();
    const QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");

    if (m_upgradeBU) {
        backupDir = QStandardPaths::writableLocation(
                            QStandardPaths::DocumentsLocation) +
                "/Mixxx-BackUps/UpgradeBUs";
        archivePath = backupDir + "/MixxxSettings-Upgrrade-" +
                currentMixxxVersion + "-" + timestamp;
    } else {
        backupDir = QStandardPaths::writableLocation(
                            QStandardPaths::DocumentsLocation) +
                "/Mixxx-BackUps";
        archivePath = backupDir + "/MixxxSettings-" + timestamp;
    }

    archivePath7zExt = archivePath + ".7z";
    archivePathZipExt = archivePath + ".zip";
    QDir().mkpath(backupDir);

#if defined(Q_OS_MACOS)
    zipExecutable = "/usr/bin/zip";
#endif

#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    // Windows & Linux: use 7z
    zipExecutable = QStandardPaths::findExecutable("7z");

    // If not found in PATH -> check additional locations
    if (zipExecutable.isEmpty()) {
#if defined(Q_OS_WIN)
        const QStringList winPaths = {
                QDir::homePath() + "/scoop/apps/7zip/current/7z.exe",
                "C:\\Program Files\\7-Zip\\7z.exe",
                "C:\\Program Files (x86)\\7-Zip\\7z.exe"};
        for (const QString& path : winPaths) {
            if (QFile::exists(path)) {
                zipExecutable = path;
                break;
            }
        }
#elif defined(Q_OS_LINUX)
        // Linux fallback paths (unchanged)
        const QStringList linuxPaths = {"/usr/bin/7z", "/usr/local/bin/7z", "/bin/7z"};
        for (const QString& path : linuxPaths) {
            if (QFile::exists(path)) {
                zipExecutable = path;
                break;
            }
        }
#endif
    }
#endif

    if (!zipExecutable.isEmpty()) {
        qDebug() << "[BackUp] -> [BAckUpWorker] -> 7z/Zip found in: " << zipExecutable;
        QString tempBackupDir = archivePath + "_temp";
        QDir().mkpath(tempBackupDir);

        if (!copySettingsToTempDir(settingsDir, tempBackupDir)) {
            QDir(tempBackupDir).removeRecursively();
            qDebug() << "[BackUp] -> [BAckUpWorker] -> error creating tempdir";
            return;
        }

#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
        QProcess process;
        process.setProcessChannelMode(QProcess::MergedChannels);
        process.start(zipExecutable,
                {"a",
                        "-t7z",
                        archivePath7zExt,
                        tempBackupDir + "/*",
                        "-xr!analysis",
                        "-xr!lut",
                        "-mx=6"});

        if (!process.waitForFinished(300000)) {
            qCritical() << "[BackUp] -> [BAckUpWorker] -> 7z compression timed out!";
            process.kill();
            useBit7z = true;
        } else if (process.exitCode() != 0) {
            qCritical() << "[BackUp] -> [BAckUpWorker] -> 7z failed:"
                        << process.readAllStandardOutput();
            useBit7z = true;
        } else {
            qDebug() << "[BackUp] -> [BAckUpWorker] -> Backup succeeded! "
                        "Archive:"
                     << archivePath7zExt;
            emit backUpFinished(true, archivePath);
            useBit7z = false;
        }

        QDir(tempBackupDir).removeRecursively();
        return;

#elif defined(Q_OS_MACOS)
        QStringList arguments = {"-r",
                archivePathZipExt,
                settingsDir,
                "-x",
                settingsDir + "/analysis/*",
                "-x",
                settingsDir + "/lut/*"};
        qDebug() << "[BackUp] -> [BAckUpWorker] -> Executing:" << zipExecutable
                 << arguments.join(" ");
        bool started = QProcess::startDetached(zipExecutable, arguments);
        if (started) {
            qDebug() << "[BackUp] -> [BAckUpWorker] -> MacOS zip backup "
                        "started to:"
                     << archivePathZipExt;
            emit backUpFinished(true, archivePath);
        } else {
            qWarning() << "[BackUp] -> [BAckUpWorker] -> MacOS zip backup failed.";
            useBit7z = true;
        }
        return;
#endif

    } else {
        qWarning() << "[BackUp] -> [BAckUpWorker] -> 7z/Zip not found in PATH "
                      "or fallback locations. "
                      "-> using  7zip bit7z from internal library.";
        useBit7z = true;
    }

    if (useBit7z) {
        // All next lines need to be uncommented when bit7z and 7z are in the dependencies
        // and the parts in the CMakeLists are uncommented too.
        qDebug() << "[BackUp] -> [BAckUpWorker] -> Bit7z started";
        QString path7z;
#if defined(Q_OS_WIN)
        path7z = QCoreApplication::applicationDirPath() + "/7z.dll";
#else
        path7z = QCoreApplication::applicationDirPath() + "/7z";
#endif
        emit progressChanged(0);
        if (!QFile::exists(path7z)) {
            qWarning() << "7z not found:" << path7z;
            emit backUpFinished(false, "Backup failed");
            return;
        }
        try {
            // bit7z::Bit7zLibrary lib("7zip.dll");
            // bit7z::Bit7zLibrary lib("7z.dll");

            QTemporaryDir tempDir;
            if (tempDir.isValid()) {
                QString tempBackupDir = archivePath + "_temp";
                QDir().mkpath(tempBackupDir);

                if (!copySettingsToTempDir(settingsDir, tempBackupDir)) {
                    emit errorOccurred("Could not create temporary directory for backup.");
                    emit backUpFinished(false, "Backup failed");
                    return;
                }

                bit7z::Bit7zLibrary lib(path7z.toStdString());
                // bit7z::Bit7zLibrary lib;
                // bit7z::Bit7zLibrary lib("7zip.dll");
                bit7z::BitFileCompressor compressor(lib, bit7z::BitFormat::SevenZip);

                archivePath7zExt = archivePath + ".7z";
                emit progressChanged(10);
                compressor.compressDirectory(
                        tempBackupDir.toStdString(),
                        archivePath7zExt.toStdString());
                emit progressChanged(80);
                QDir(tempBackupDir).removeRecursively();
            }

            emit progressChanged(100);
            emit backUpFinished(true, archivePath);

        } catch (const bit7z::BitException& ex) {
            emit errorOccurred(QString::fromStdString(ex.what()));
            qDebug() << "[BackUp] -> [BAckUpWorker] -> error " << QString::fromStdString(ex.what());
            emit backUpFinished(false, "Backup failed");
        }

        qDebug() << "[BAckUpWorker] --> Bit7z ended";
    }
}

void BackUpWorker::deleteOldBackUps() {
    const QString backupDir = QStandardPaths::writableLocation(
                                      QStandardPaths::DocumentsLocation) +
            "/Mixxx-BackUps";
    QDir dir(backupDir);
    dir.setNameFilters({"MixxxSettings-*.7z"});
    dir.setSorting(QDir::Time);

    const auto backups = dir.entryInfoList();
    for (int i = m_keepBackUps; i < backups.size(); ++i) {
        dir.remove(backups[i].fileName());
        dir.remove(backups[i].fileName().replace(".7z", "_temp_robocopy.log"));
        emit backUpRemoved(backups[i].fileName());
    }
}
