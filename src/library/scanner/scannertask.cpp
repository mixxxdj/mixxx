#include "library/scanner/scannertask.h"
#include "library/libraryscanner.h"

ScannerTask::ScannerTask(LibraryScanner* pScanner,
                         const ScannerGlobalPointer scannerGlobal)
        : m_pScanner(pScanner),
          m_scannerGlobal(scannerGlobal),
          m_success(false) {
    setAutoDelete(true);
}

ScannerTask::~ScannerTask() {
    emit(taskDone(m_success));
}
