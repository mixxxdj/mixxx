#include "library/scanner/scannertask.h"

#include "library/scanner/libraryscanner.h"
#include "moc_scannertask.cpp"

ScannerTask::ScannerTask(LibraryScanner* pScanner,
                         const ScannerGlobalPointer scannerGlobal)
        : m_pScanner(pScanner),
          m_scannerGlobal(scannerGlobal),
          m_success(false) {
    setAutoDelete(true);
}

ScannerTask::~ScannerTask() {
    if (!m_success) {
        m_scannerGlobal->clearScanFinishedCleanly();
    }
    m_scannerGlobal->getTaskWatcher().taskDone();
}
