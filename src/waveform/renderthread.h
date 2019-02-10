#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

#include <QThread>
#include <QSemaphore>
#include <QPair>

#include "util/performancetimer.h"

class RenderThread : public QThread {
    Q_OBJECT
  public:
    enum RenderMode {
        ST_TIMER = 0,
        ST_FREE,
        ST_COUNT // Dummy Type at last, counting possible types
    };

    RenderThread(QObject* pParent);
    ~RenderThread();

    void run();
    void stop();

    int elapsed();
    void setSyncIntervalTimeMicros(int usSyncTimer);
    void setRenderType(int mode);
    int droppedFrames();
    void renderSlotFinished();

  signals:
    void render();

  private:
    bool m_bDoRendering;
    int m_syncIntervalTimeMicros;
    enum RenderMode m_renderMode;
    int m_droppedFrames;
    PerformanceTimer m_timer;
    QSemaphore m_semaRenderSlot;
};


#endif // RENDERTHREAD_H
