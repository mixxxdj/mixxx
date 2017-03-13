#include "library/scanner/scannertask.h"
#include "library/scanner/libraryscanner.h"

ScannerTask::ScannerTask(LibraryScanner* pScanner,
                         const ScannerGlobalPointer scannerGlobal)
        : m_pScanner(pScanner),
          m_scannerGlobal(scannerGlobal),
          m_success(false) {
    setAutoDelete(true);
    connect(this, SIGNAL(directoryHashed(QString, bool, int)),
            this, SIGNAL(directoryHashedAndScanned(QString, bool, int)));
}

ScannerTask::~ScannerTask() {
    emit(taskDone(m_success));
}
