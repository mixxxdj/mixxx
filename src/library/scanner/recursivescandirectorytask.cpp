#include "library/scanner/recursivescandirectorytask.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>

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
    ScopedTimer timer(u"RecursiveScanDirectoryTask::run");
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
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::System);
    // sort directory by file name to increase chance that files are sorted sensible
    dir.setSorting(QDir::SortFlag::DirsFirst | QDir::SortFlag::Name);
    const QFileInfoList children = dir.entryInfoList();

    std::list<QFileInfo> filesToImport;
    std::list<QFileInfo> possibleCovers;
    std::list<mixxx::FileInfo> dirsToScan;

    QCryptographicHash hasher(QCryptographicHash::Sha256);

    // TODO(rryan) benchmark QRegularExpression copy versus QMutex/QRegularExpression in ScannerGlobal
    // versus slicing the extension off and checking for set/list containment.
    QRegularExpression supportedExtensionsRegex =
            m_scannerGlobal->supportedExtensionsRegex();
    QRegularExpression supportedCoverExtensionsRegex =
            m_scannerGlobal->supportedCoverExtensionsRegex();

    for (const auto& currentFileInfo : children) {
        QString currentFile = currentFileInfo.filePath();

        if (currentFileInfo.isFile()) {
            const QString& fileName = currentFileInfo.fileName();
            const QRegularExpressionMatch supportedExtensionsMatch =
                    supportedExtensionsRegex.match(fileName);
            if (supportedExtensionsMatch.hasMatch()) {
                hasher.addData(currentFile.toUtf8());
                filesToImport.push_back(currentFileInfo);
            } else {
                const QRegularExpressionMatch supportedCoverExtensionsMatch =
                        supportedCoverExtensionsRegex.match(fileName);
                if (supportedCoverExtensionsMatch.hasMatch()) {
                    possibleCovers.push_back(currentFileInfo);
                }
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
