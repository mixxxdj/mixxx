#include "backupworker.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
// Needs to be uncommented when bit7z and 7z are in the dependencies
// and th parts in the CMakeLists are uncommented too.
// #include <bit7z/bit7z.hpp>

#include "moc_backupworker.cpp"

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

void BackUpWorker::performBackUp() {
    QString backupDir;
    QString archivePath;
    QString archivePath7zExt;
    QString archivePathZipExt;
    const QString settingsDir = m_pConfig->getSettingsPath();
    QString zipExecutable;
    QStringList arguments;
    if (m_upgradeBU) {
        backupDir = QStandardPaths::writableLocation(
                            QStandardPaths::DocumentsLocation) +
                "/Mixxx-BackUps/UpgradeBUs";
        archivePath = backupDir + "/MixxxSettings-Upgrrade-" +
                currentMixxxVersion + "-" +
                QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
        archivePath7zExt = archivePath + ".7z";
        archivePathZipExt = archivePath + ".zip";
    } else {
        backupDir = QStandardPaths::writableLocation(
                            QStandardPaths::DocumentsLocation) +
                "/Mixxx-BackUps";
        archivePath = backupDir + "/MixxxSettings-" +
                QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
        archivePath7zExt = archivePath + ".7z";
        archivePathZipExt = archivePath + ".zip";
    }

    QDir().mkpath(backupDir);
    const QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");

#if defined(Q_OS_MACOS)
    zipExecutable = "/usr/bin/zip";

#endif

#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    // Windows & Linux: use 7z
    zipExecutable = QStandardPaths::findExecutable("7z");

    // If not found in PATH -> check additional locations
    if (zipExecutable.isEmpty()) {
#if defined(Q_OS_WIN)
        QStringList winPaths;
        QString scoopPath = QDir::homePath() + "/scoop/apps/7zip/current/7z.exe";
        winPaths << scoopPath;
        winPaths << "C:\\Program Files\\7-Zip\\7z.exe";
        winPaths << "C:\\Program Files (x86)\\7-Zip\\7z.exe";

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
        qDebug() << "[BackUp] -> 7z/Zip found in: " << zipExecutable;
        QString tempBackupDir = archivePath + "_temp";
        QDir().mkpath(tempBackupDir);
#if defined(Q_OS_WIN)
        QProcess robocopy;
        robocopy.start("robocopy",
                {QDir::toNativeSeparators(settingsDir),
                        QDir::toNativeSeparators(tempBackupDir),
                        "/E", // show dirs
                        "/XD",
                        "analysis", // exclude analysis
                        "/R:3",     // retry 3 times if file is locked
                        "/W:2",     // wait 2 seconds between retries
                        "/NP",      // progress display off
                        "/LOG+:" +  // write log file -> can be quoted later
                                QDir::toNativeSeparators(
                                        archivePath + "_robocopy.log")});

        if (!robocopy.waitForFinished(30000) || robocopy.exitCode() >= 8) {
            qCritical() << "File copy failed! Check log:" << archivePath + "_robocopy.log";
            QDir(tempBackupDir).removeRecursively();
            return;
        }

#elif defined(Q_OS_LINUX)
        QProcess rsync;
        rsync.start("rsync", {// archive mode
                                     "-a",
                                     "--exclude=analysis/", // exclude analysis
                                     "--retries=3",         // retry 3 times if file is locked
                                     settingsDir + "/",
                                     tempBackupDir + "/"});

        if (!rsync.waitForFinished() || rsync.exitCode() != 0) {
            qCritical() << "rsync failed!";
            return;
        }
#endif

#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
        QProcess process;
        process.setProcessChannelMode(QProcess::MergedChannels);
        process.start(zipExecutable,
                {"a",
                        "-t7z",           // archive path with ext
                        archivePath7zExt, // temp copy as source
                        tempBackupDir +
                                "/*",   // exclude analysis, repeet for safety
                        "-xr!analysis", // compression level, set to 4 for
                                        // speed/quality, 9 takes too long
                        "-mx=6"});
        if (!process.waitForFinished(300000)) {
            qCritical() << "7z compression timed out!";
            process.kill();
            // Needs to be uncommented when bit7z and 7z are in the dependencies
            // and the parts in the CMakeLists are uncommented too.
            // useBit7z = true;
        } else if (process.exitCode() != 0) {
            qCritical() << "7z failed:" << process.readAllStandardOutput();
            // Needs to be uncommented when bit7z and 7z are in the dependencies
            // and the parts in the CMakeLists are uncommented too.
            // useBit7z = true;
        } else {
            qDebug() << "Backup succeeded! Archive:" << archivePath + ".7z";
            useBit7z = false;
            // if BackUp Succeeds we don't need to try bit7z -> emit
            emit backUpFinished(true, archivePath);
        }

        QDir(tempBackupDir).removeRecursively();

#elif defined(Q_OS_MACOS)

        arguments << "-r" << archivePathZipExt << settingsDir << "-x"
                  << settingsDir + "/analysis/*";
        qDebug() << "[BackUp] -> Executing:" << zipExecutable << arguments.join(" ");
        bool started = QProcess::startDetached(zipExecutable, arguments);
        if (started) {
            qDebug() << "[BackUp] -> MacOS Settings backup started to:" << archivePathZipExt;
            useBit7z = false;
        } else {
            qWarning() << "[BackUp] -> MacOS Failed to launch zip backup process.";
            useBit7z = false;
        }
        // needs some results from the zip exec for deteleting the old BackUps
#endif

    } else {
        qWarning() << "[BackUp] -> 7z/Zip not found in PATH or fallback locations. "
                      "-> using  7zip bit7z from internal library.";
        useBit7z = false;
    }

    if (useBit7z) {
        // All next lines need to be uncommented when bit7z and 7z are in the dependencies
        // and the parts in the CMakeLists are uncommented too.

        //        try {
        //            emit progressChanged(10);
        //            const QString originalDirName = QFileInfo(settingsDir).fileName();

        //            bit7z::Bit7zLibrary lib("7zip.dll");
        //            bit7z::BitFileCompressor compressor(lib, bit7z::BitFormat::SevenZip);

        //            QTemporaryDir tempDir;
        //            if (tempDir.isValid()) {
        //                // Create target directory structure
        //                const QString tempRoot = tempDir.path() + "/" +
        //                originalDirName + "-" + timestamp;
        //                QDir().mkpath(tempRoot);

        // Copy files excluding analysis
        // thinking of robocopy to

        //                QDirIterator it(settingsDir,
        //                        QDir::Files | QDir::NoDotAndDotDot,
        //                        QDirIterator::Subdirectories);

        //                while (it.hasNext()) {
        //                    QString srcPath = it.next();
        //                    QString relativePath = QDir(settingsDir).relativeFilePath(srcPath);

        // Skip analysis folders at any level
        //                    if (relativePath.contains("analysis/") ||
        //                            relativePath.startsWith("analysis/") ||
        //                            relativePath.endsWith("/analysis")) {
        //                        continue;
        //                    }

        //                    QString destPath = tempRoot + "/" + relativePath;
        //                    QFileInfo(destPath).dir().mkpath(".");
        //                    QFile::copy(srcPath, destPath);
        //                }

        // Needs to be uncommented when bit7z and 7z are in the dependencies
        // and the parts in the CMakeLists are uncommented too.
        //                compressor.compressDirectory(
        //                        tempRoot.toStdString(),
        //                        archivePath.toStdString());
        //            }

        //            emit progressChanged(80);
        //            emit backUpFinished(true, archivePath);
        //            emit progressChanged(100);

        //          } catch (const bit7z::BitException& ex) {
        //             emit errorOccurred(QString::fromStdString(ex.what()));
        //             emit backUpFinished(false, "Backup failed");
        //        }
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
        dir.remove(backups[i].fileName().replace(".7z", "_robocopy.log"));
        emit backUpRemoved(backups[i].fileName());
    }
}
