#ifndef SCANNERTASK_H
#define SCANNERTASK_H

#include <QObject>
#include <QRunnable>

#include "trackinfoobject.h"
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
    void directoryHashed(const QString& directoryPath, bool newDirectory,
                         int hash);
    void directoryUnchanged(const QString& directoryPath);
    void trackExists(const QString& filePath);
    void addNewTrack(TrackPointer pTrack);

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

#endif /* SCANNERTASK_H */
