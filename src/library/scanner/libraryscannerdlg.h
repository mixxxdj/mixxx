#pragma once

#include <QDialog>

#include "util/performancetimer.h"

class LibraryScannerDlg : public QDialog {
    Q_OBJECT
  public:
    LibraryScannerDlg(QWidget* pParent = nullptr);

  public slots:
    void slotUpdate(const QString& path);
    void slotUpdateCover(const QString& path);
    void slotCancel();
    void slotScanFinished();
    void slotScanStarted();

  signals:
    void scanCancelled();
    void progress(const QString&);

  private:
    PerformanceTimer m_timer;
    bool m_bCancelled;
};
