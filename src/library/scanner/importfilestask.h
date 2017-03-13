#ifndef IMPORTFILESTASK_H
#define IMPORTFILESTASK_H

#include <QLinkedList>
#include <QFileInfo>

#include "util/sandbox.h"
#include "library/scanner/scannertask.h"
#include "library/scanner/scannerglobal.h"

// Import the provided files. Successful if the scan completed without being
// cancelled. False if the scan was cancelled part-way through.
class ImportFilesTask : public ScannerTask {
    Q_OBJECT
  public:
    ImportFilesTask(LibraryScanner* pScanner,
                    const ScannerGlobalPointer scannerGlobal,
                    const QLinkedList<QFileInfo>& filesToImport,
                    const QLinkedList<QFileInfo>& possibleCovers,
                    SecurityTokenPointer pToken);
    virtual ~ImportFilesTask() {}

    virtual void run();

  private:
    const QLinkedList<QFileInfo> m_filesToImport;
    const QLinkedList<QFileInfo> m_possibleCovers;
    SecurityTokenPointer m_pToken;
};

#endif /* IMPORTFILESTASK_H */
