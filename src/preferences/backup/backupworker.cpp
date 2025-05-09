#include "backupworker.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <bit7z/bit7z.hpp>

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
}

void BackUpWorker::performBackUp() {
    try {
        emit progressChanged(10);
        const QString settingsDir = m_pConfig->getSettingsPath();
        QString backupDir;
        QString archivePath;
        if (m_upgradeBU) {
            backupDir = QStandardPaths::writableLocation(
                                QStandardPaths::DocumentsLocation) +
                    "/Mixxx-BackUps/UpgradeBUs";
            archivePath = backupDir + "/MixxxSettings-Upgrrade-" +
                    currentMixxxVersion + "-" +
                    QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss") +
                    ".7z";
        } else {
            backupDir = QStandardPaths::writableLocation(
                                QStandardPaths::DocumentsLocation) +
                    "/Mixxx-BackUps";
            archivePath = backupDir + "/MixxxSettings-" +
                    QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss") +
                    ".7z";
        }

        QDir().mkpath(backupDir);
        const QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");

        // const QString archivePath = backupDir + "/MixxxSettings-" +
        // QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss") + ".7z";
        const QString originalDirName = QFileInfo(settingsDir).fileName();

        bit7z::Bit7zLibrary lib("7zip.dll");
        bit7z::BitFileCompressor compressor(lib, bit7z::BitFormat::SevenZip);

        QTemporaryDir tempDir;
        if (tempDir.isValid()) {
            // Create target directory structure
            const QString tempRoot = tempDir.path() + "/" + originalDirName + "-" + timestamp;
            QDir().mkpath(tempRoot);

            // Copy files excluding analysis
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

                QString destPath = tempRoot + "/" + relativePath;
                QFileInfo(destPath).dir().mkpath(".");
                QFile::copy(srcPath, destPath);
            }

            // 4. Compress the filtered directory
            compressor.compressDirectory(
                    tempRoot.toStdString(),
                    archivePath.toStdString());
        }

        emit progressChanged(80);
        emit backUpFinished(true, archivePath);
        emit progressChanged(100);

    } catch (const bit7z::BitException& ex) {
        emit errorOccurred(QString::fromStdString(ex.what()));
        emit backUpFinished(false, "Backup failed");
    }
}

void BackUpWorker::deleteOldBackUps() {
    const QString backupDir = QStandardPaths::writableLocation(
                                      QStandardPaths::DocumentsLocation) +
            "/Mixxx-BackUps";
    QDir dir(backupDir);
    dir.setNameFilters({"MixxxSettings-*.7z"});
    // dir.setSorting(QDir::Time | QDir::Reversed);
    dir.setSorting(QDir::Time);

    const auto backups = dir.entryInfoList();
    for (int i = m_keepBackUps; i < backups.size(); ++i) {
        dir.remove(backups[i].fileName());
        emit backUpRemoved(backups[i].fileName());
    }
}
