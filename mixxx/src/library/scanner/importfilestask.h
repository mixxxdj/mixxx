#pragma once

#include <QFileInfo>

#include "util/sandbox.h"
#include "library/scanner/scannertask.h"

/// Import the provided files. Successful if the scan completed without being
/// cancelled. False if the scan was cancelled part-way through.
class ImportFilesTask : public ScannerTask {
    Q_OBJECT
  public:
    ImportFilesTask(LibraryScanner* pScanner,
            const ScannerGlobalPointer scannerGlobal,
            const QString& dirPath,
            const bool prevHashExists,
            const mixxx::cache_key_t newHash,
            const std::list<QFileInfo>& filesToImport,
            const std::list<QFileInfo>& possibleCovers,
            SecurityTokenPointer pToken);
    virtual ~ImportFilesTask() {}

    virtual void run();

  private:
    const QString m_dirPath;
    const bool m_prevHashExists;
    const mixxx::cache_key_t m_newHash;
    const std::list<QFileInfo> m_filesToImport;
    const std::list<QFileInfo> m_possibleCovers;
    SecurityTokenPointer m_pToken;
};
