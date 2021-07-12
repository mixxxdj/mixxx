#pragma once

#include <QObject>
#include <QRunnable>

#include "library/scanner/scannerglobal.h"

class LibraryScanner;

class ScannerTask : public QObject, public QRunnable {
    Q_OBJECT
  public:
    ScannerTask(LibraryScanner* pScanner,
                const ScannerGlobalPointer scannerGlobal);
    virtual ~ScannerTask();

    virtual void run() = 0;

  signals:
    void taskDone(bool success);
    void queueTask(ScannerTask* pTask);
    void directoryHashedAndScanned(const QString& directoryPath,
                                   bool newDirectory, mixxx::cache_key_t hash);
    void directoryUnchanged(const QString& directoryPath);
    void trackExists(const QString& filePath);
    void addNewTrack(const QString& filePath);

    // Feedback to GUI
    void progressLoading(const QString& fileName);
    void progressHashing(const QString& directoryPath);

  protected:
    void setSuccess(bool success) {
        m_success = success;
    }

    LibraryScanner* m_pScanner;
    const ScannerGlobalPointer m_scannerGlobal;

  private:
    bool m_success;
};
