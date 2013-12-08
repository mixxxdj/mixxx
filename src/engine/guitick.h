#ifndef GUITICK_H
#define GUITICK_H

#include <QObject>

#include <portaudio.h>

#include "util/performancetimer.h"

class ControlObject;
class QTimer;

class GuiTick : public QObject {
    Q_OBJECT
  public:
    GuiTick(QObject* pParent=NULL);
    ~GuiTick();
    void process();

    static void setStreamTime(double streamTime);
    static double streamTime();
    static double cpuTime();

  private slots:
    void slotBackupTimerExpired();

  private:
    ControlObject* m_pCOStreamTime;
    ControlObject* m_pCOCpuTime;
    ControlObject* m_pCOGuiTick50ms;

    PerformanceTimer m_cpuTimer;
    QTimer* m_backupTimer;

    double m_lastUpdateTime;

    static double m_streamTime; // Stream Time in seconds
    static double m_cpuTime; // Stream Time in seconds
};

#endif // GUITICK_H
