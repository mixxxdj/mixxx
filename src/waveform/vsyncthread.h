#pragma once

#include <QPair>
#include <QSemaphore>
#include <QThread>
#include <mutex>

#include "util/performancetimer.h"
#include "waveform/isynctimeprovider.h"

class WGLWidget;

class VSyncThread : public QThread, public VSyncTimeProvider {
    Q_OBJECT
  public:
    enum VSyncMode {
        ST_DEFAULT = 0,
        ST_MESA_VBLANK_MODE_1_DEPRECATED, // 1
        ST_SGI_VIDEO_SYNC_DEPRECATED,     // 2
        ST_OML_SYNC_CONTROL_DEPRECATED,   // 3
        ST_FREE,                          // 4
        ST_PLL,                           // 5
        ST_TIMER,                         // 6
        ST_COUNT                          // Dummy Type at last, counting possible types
    };

    VSyncThread(QObject* pParent, VSyncMode vSyncMode);
    ~VSyncThread();

    void run() override;

    bool waitForVideoSync(WGLWidget* glw);
    int elapsed();
    void setSyncIntervalTimeMicros(int usSyncTimer);
    int droppedFrames();
    void setSwapWait(int sw);
    // VSyncTimerProvider
    std::chrono::microseconds fromTimerToNextSync(const PerformanceTimer& timer) override;
    void vsyncSlotFinished();
    void getAvailableVSyncTypes(QList<QPair<int, QString>>* list);
    void setupSync(WGLWidget* glw, int index);
    void waitUntilSwap(WGLWidget* glw);
    mixxx::Duration sinceLastSwap() const;
    // VSyncTimerProvider
    std::chrono::microseconds getSyncInterval() const override {
        return std::chrono::microseconds(m_syncIntervalTimeMicros);
    }
    void updatePLL();
    bool pllInitializing() const;
    VSyncMode vsyncMode() const {
        return m_vSyncMode;
    }
  signals:
    void vsyncSwapAndRender();
    void vsyncRender();
    void vsyncSwap();
  private:
    void runFree();
    void runPLL();
    void runTimer();

    bool m_bDoRendering;
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
    std::atomic<int> m_pllInitCnt;
    std::atomic<bool> m_pllPendingUpdate;
    double m_pllInitSum;
    double m_pllInitAvg;
    double m_pllPhaseOut;
    double m_pllDeltaOut;
    double m_pllLogging;
};
