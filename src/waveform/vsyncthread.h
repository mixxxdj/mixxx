#ifndef VSYNCTHREAD_H
#define VSYNCTHREAD_H

#include <QByteArrayData>
#include <QGLWidget>
#include <QPair>
#include <QSemaphore>
#include <QString>
#include <QThread>
#include <QTime>

#include "util/performancetimer.h"

class QGLWidget;
class QObject;
template<class T1, class T2>
struct QPair;
template<typename T>
class QList;

class VSyncThread : public QThread {
    Q_OBJECT
  public:
    enum VSyncMode {
        ST_TIMER = 0,
        ST_MESA_VBLANK_MODE_1,
        ST_SGI_VIDEO_SYNC,
        ST_OML_SYNC_CONTROL,
        ST_FREE,
        ST_COUNT // Dummy Type at last, counting possible types
    };

    VSyncThread(QObject* pParent);
    ~VSyncThread();

    void run();

    bool waitForVideoSync(QGLWidget* glw);
    int elapsed();
    int toNextSyncMicros();
    void setSyncIntervalTimeMicros(int usSyncTimer);
    void setVSyncType(int mode);
    int droppedFrames();
    void setSwapWait(int sw);
    int fromTimerToNextSyncMicros(const PerformanceTimer& timer);
    void vsyncSlotFinished();
    void getAvailableVSyncTypes(QList<QPair<int, QString > >* list);
    void setupSync(QGLWidget* glw, int index);
    void waitUntilSwap(QGLWidget* glw);

  signals:
    void vsyncRender();
    void vsyncSwap();

  private:
    bool m_bDoRendering;
    bool m_vSyncTypeChanged;
    int m_syncIntervalTimeMicros;
    int m_waitToSwapMicros;
    enum VSyncMode m_vSyncMode;
    bool m_syncOk;
    int m_droppedFrames;
    int m_swapWait;
    PerformanceTimer m_timer;
    QSemaphore m_semaVsyncSlot;
    double m_displayFrameRate;
    int m_vSyncPerRendering;
};


#endif // VSYNCTHREAD_H
