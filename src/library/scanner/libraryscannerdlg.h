#pragma once

#include <QThread>
#include <QWidget>
#include <QString>

#include "util/performancetimer.h"

class LibraryScannerDlg : public QWidget {
    Q_OBJECT
  public:
    LibraryScannerDlg(QWidget* parent = NULL, Qt::WindowFlags f = Qt::Dialog);
    virtual ~LibraryScannerDlg();

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
