#pragma once

#include <QDialog>
#include <QLabel>
#include <QProgressBar>

#include "util/parented_ptr.h"
#include "util/performancetimer.h"

class LibraryScannerDlg : public QDialog {
    Q_OBJECT
  public:
    LibraryScannerDlg(QWidget* pParent = nullptr);

    void resetTaskCount();
    void addQueuedTasks(int num);

  public slots:
    void slotUpdate(const QString& path);
    void slotUpdateCover(const QString& path);
    void slotCancel();
    void slotScanFinished();
    void slotScanStarted();

  signals:
    void scanCancelled();

  private:
    void updateProgressBar();

    PerformanceTimer m_timer;

    parented_ptr<QLabel> m_pLabelCurrent;
    parented_ptr<QProgressBar> m_pProgressBar;

    bool m_bCancelled;
    int m_tasksDone;
    int m_tasksTotal;
    bool m_showNoTasksQueuedWarning;
};
