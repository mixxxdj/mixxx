#pragma once

#include <QByteArrayData>
#include <QDir>
#include <QString>

#include "library/scanner/scannerglobal.h"
#include "library/scanner/scannertask.h"
#include "util/sandbox.h"

class LibraryScanner;
class QObject;

/// Recursively scan a music library. Doesn't import tracks for any directories
/// that have already been scanned and have not changed. Changes are tracked by
/// performing a hash of the directory's file list, and those hashes are stored
/// in the database. Successful if the scan completed without being
/// cancelled. False if the scan was cancelled part-way through.
class RecursiveScanDirectoryTask : public ScannerTask {
    Q_OBJECT
  public:

    RecursiveScanDirectoryTask(LibraryScanner* pScanner,
                               const ScannerGlobalPointer scannerGlobal,
                               const QDir& dir,
                               SecurityTokenPointer pToken,
                               bool scanUnhashed);
    virtual ~RecursiveScanDirectoryTask() {}

    virtual void run();

  private:
    QDir m_dir;
    SecurityTokenPointer m_pToken;
    bool m_scanUnhashed;
};
