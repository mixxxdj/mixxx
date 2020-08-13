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
    void slotUpdate(QString path);
    void slotUpdateCover(QString path);
    void slotCancel();
    void slotScanFinished();
    void slotScanStarted();

  signals:
    void scanCancelled();
    void progress(QString);

  private:
    PerformanceTimer m_timer;
    bool m_bCancelled;
};
