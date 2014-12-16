#ifndef RECURSIVESCANDIRECTORYTASK_H
#define RECURSIVESCANDIRECTORYTASK_H

#include <QDir>

#include "library/scanner/scannertask.h"
#include "util/sandbox.h"

// Recursively scan a music library. Doesn't import tracks for any directories
// that have already been scanned and have not changed. Changes are tracked by
// performing a hash of the directory's file list, and those hashes are stored
// in the database. Successful if the scan completed without being
// cancelled. False if the scan was cancelled part-way through.
class RecursiveScanDirectoryTask : public ScannerTask {
    Q_OBJECT
  public:

    RecursiveScanDirectoryTask(LibraryScanner* pScanner,
                               const ScannerGlobalPointer scannerGlobal,
                               const QDir& dir,
                               SecurityTokenPointer pToken);
    virtual ~RecursiveScanDirectoryTask() {}

    virtual void run();

  private:
    QDir m_dir;
    SecurityTokenPointer m_pToken;
};

#endif /* RECURSIVESCANDIRECTORYTASK_H */
