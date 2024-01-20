#pragma once

#include <QPair>
#include <QSemaphore>
#include <QThread>
#include <mutex>

#include "util/performancetimer.h"

class WGLWidget;

class VSyncThread : public QThread {
    Q_OBJECT
  public:
    enum VSyncMode {
        ST_TIMER = 0,
        ST_MESA_VBLANK_MODE_1,
        ST_SGI_VIDEO_SYNC,
        ST_OML_SYNC_CONTROL,
        ST_FREE,
        ST_PLL,
        ST_COUNT // Dummy Type at last, counting possible types
    };

    VSyncThread(QObject* pParent);
    ~VSyncThread();

    void run();

    bool waitForVideoSync(WGLWidget* glw);
    int elapsed();
    void setSyncIntervalTimeMicros(int usSyncTimer);
    void setVSyncType(int mode);
    int droppedFrames();
    void setSwapWait(int sw);
    int fromTimerToNextSyncMicros(const PerformanceTimer& timer);
    void vsyncSlotFinished();
    void getAvailableVSyncTypes(QList<QPair<int, QString>>* list);
    void setupSync(WGLWidget* glw, int index);
    void waitUntilSwap(WGLWidget* glw);
    mixxx::Duration sinceLastSwap() const;
    int getSyncIntervalTimeMicros() const {
        return m_syncIntervalTimeMicros;
    }
    void updatePLL();
  signals:
    void vsyncSwapAndRender();
    void vsyncRender();
    void vsyncSwap();

  private:
    void runFree();
    void runPLL();
    void runTimer();

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
    mixxx::Duration m_sinceLastSwap;
    // phase locked loop
    std::mutex m_pllMutex;
    PerformanceTimer m_pllTimer;
    double m_pllPhaseOut;
    double m_pllDeltaOut;
    double m_pllLogging;
};
