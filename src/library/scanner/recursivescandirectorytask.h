#pragma once

#include <QDir>

#include "library/scanner/scannertask.h"
#include "util/fileaccess.h"

/// Recursively scan a music library. Doesn't import tracks for any directories
/// that have already been scanned and have not changed. Changes are tracked by
/// performing a hash of the directory's file list, and those hashes are stored
/// in the database. Successful if the scan completed without being
/// cancelled. False if the scan was cancelled part-way through.
class RecursiveScanDirectoryTask : public ScannerTask {
    Q_OBJECT
  public:
    RecursiveScanDirectoryTask(LibraryScanner* pScanner,
            const ScannerGlobalPointer& scannerGlobal,
            const mixxx::FileAccess&& dirAccess,
            bool scanUnhashed);
    ~RecursiveScanDirectoryTask() override = default;

    void run() override;

  private:
    const mixxx::FileAccess m_dirAccess;
    const bool m_scanUnhashed;
};
