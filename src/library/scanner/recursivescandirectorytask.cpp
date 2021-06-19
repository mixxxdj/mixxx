#include "library/scanner/recursivescandirectorytask.h"

#include <QCryptographicHash>
#include <QDirIterator>

#include "library/scanner/importfilestask.h"
#include "library/scanner/libraryscanner.h"
#include "moc_recursivescandirectorytask.cpp"
#include "util/timer.h"

RecursiveScanDirectoryTask::RecursiveScanDirectoryTask(
        LibraryScanner* pScanner,
        const ScannerGlobalPointer& scannerGlobal,
        const mixxx::FileAccess&& dirAccess,
        bool scanUnhashed)
        : ScannerTask(pScanner, scannerGlobal),
          m_dirAccess(std::move(dirAccess)),
          m_scanUnhashed(scanUnhashed) {
}

void RecursiveScanDirectoryTask::run() {
    ScopedTimer timer("RecursiveScanDirectoryTask::run");
    if (m_scannerGlobal->shouldCancel()) {
        setSuccess(false);
        return;
    }

    // For making the scanner slow
    //qDebug() << "Burn CPU";
    //for (int i = 0;i < 1000000000; i++) asm("nop");

    // Note, we save on filesystem operations (and random work) by initializing
    // a QDirIterator with a QDir instead of a QString -- but it inherits its
    // Filter from the QDir so we have to set it first. If the QDir has not done
    // any FS operations yet then this should be lightweight.
    auto dir = m_dirAccess.info().toQDir();
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    QDirIterator it(dir);

    std::list<QFileInfo> filesToImport;
    std::list<QFileInfo> possibleCovers;
    std::list<mixxx::FileInfo> dirsToScan;

    QCryptographicHash hasher(QCryptographicHash::Sha256);

    // TODO(rryan) benchmark QRegExp copy versus QMutex/QRegExp in ScannerGlobal
    // versus slicing the extension off and checking for set/list containment.
    QRegExp supportedExtensionsRegex =
            m_scannerGlobal->supportedExtensionsRegex();
    QRegExp supportedCoverExtensionsRegex =
            m_scannerGlobal->supportedCoverExtensionsRegex();

    while (it.hasNext()) {
        QString currentFile = it.next();
        QFileInfo currentFileInfo = it.fileInfo();

        if (currentFileInfo.isFile()) {
            const QString& fileName = currentFileInfo.fileName();
            if (supportedExtensionsRegex.indexIn(fileName) != -1) {
                hasher.addData(currentFile.toUtf8());
                filesToImport.push_back(currentFileInfo);
            } else if (supportedCoverExtensionsRegex.indexIn(fileName) != -1) {
                possibleCovers.push_back(currentFileInfo);
            }
        } else {
            // File is a directory
            if (m_scannerGlobal->directoryBlacklisted(currentFile)) {
                // Skip blacklisted directories like the iTunes Album
                // Art Folder since it is probably a waste of time.
                continue;
            }
            dirsToScan.push_back(mixxx::FileInfo(std::move(currentFileInfo)));
        }
    }

    // Calculate a hash of the directory's file list.
    const mixxx::cache_key_t newHash = mixxx::cacheKeyFromMessageDigest(hasher.result());

    QString dirLocation = m_dirAccess.info().location();

    // Try to retrieve a hash from the last time that directory was scanned.
    const mixxx::cache_key_t prevHash = m_scannerGlobal->directoryHashInDatabase(dirLocation);
    const bool prevHashExists = mixxx::isValidCacheKey(prevHash);

    if (prevHashExists || m_scanUnhashed) {
        // Compare the hashes, and if they don't match, rescan the files in that
        // directory!
        if (prevHash != newHash) {
            // Rescan that mofo! If importing fails then the scan was cancelled so
            // we return immediately.
            if (!filesToImport.empty()) {
                m_pScanner->queueTask(new ImportFilesTask(m_pScanner,
                        m_scannerGlobal,
                        dirLocation,
                        prevHashExists,
                        newHash,
                        filesToImport,
                        possibleCovers,
                        m_dirAccess.token()));
            } else {
                emit directoryHashedAndScanned(dirLocation, !prevHashExists, newHash);
            }
        } else {
            emit directoryUnchanged(dirLocation);
        }
    } else {
        m_scannerGlobal->addUnhashedDir(m_dirAccess);
    }

    // Process all of the sub-directories.
    for (const mixxx::FileInfo& dirInfo : dirsToScan) {
        // Atomically test and mark the directory as scanned to avoid
        // that the same directory is scanned multiple times by different
        // tasks.
        if (!m_scannerGlobal->testAndMarkDirectoryScanned(dirInfo.toQDir())) {
            m_pScanner->queueTask(
                    new RecursiveScanDirectoryTask(
                            m_pScanner,
                            m_scannerGlobal,
                            mixxx::FileAccess(dirInfo, m_dirAccess.token()),
                            m_scanUnhashed));
        }
    }
    setSuccess(true);
}
