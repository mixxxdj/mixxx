#include <QThread>
#include <QGLWidget>
#include <QGLFormat>
#include <QTime>
#include <QtDebug>
#include <QTime>

#include "mixxx.h"
#include "vsyncthread.h"
#include "util/performancetimer.h"
#include "util/event.h"
#include "util/counter.h"
#include "util/math.h"
#include "waveform/guitick.h"

#if defined(__APPLE__)
#elif defined(__WINDOWS__)
#else
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
   extern const QX11Info *qt_x11Info(const QPaintDevice *pd);
#endif
#endif

VSyncThread::VSyncThread(MixxxMainWindow* mixxxMainWindow)
        : QThread(mixxxMainWindow),
          m_bDoRendering(true),
          m_vSyncTypeChanged(false),
          m_usSyncIntervalTime(33333),
          m_vSyncMode(ST_TIMER),
          m_syncOk(false),
          m_droppedFrames(0),
          m_swapWait(0),
          m_displayFrameRate(60.0),
          m_vSyncPerRendering(1),
          m_pGuiTick(mixxxMainWindow->getGuiTick()) {
}

VSyncThread::~VSyncThread() {
    m_bDoRendering = false;
    m_semaVsyncSlot.release(2); // Two slots
    wait();
    //delete m_glw;
}

void VSyncThread::stop() {
    m_bDoRendering = false;
}


void VSyncThread::run() {
    Counter droppedFrames("VsyncThread real time error");
    QThread::currentThread()->setObjectName("VSyncThread");

    m_usWaitToSwap = m_usSyncIntervalTime;
    m_timer.start();

    while (m_bDoRendering) {
        if (m_vSyncMode == ST_FREE) {
            // for benchmark only!

            Event::start("VsyncThread vsync render");
            // renders the waveform, Possible delayed due to anti tearing
            emit(vsyncRender());
            m_semaVsyncSlot.acquire();
            Event::end("VsyncThread vsync render");

            Event::start("VsyncThread vsync swap");
            emit(vsyncSwap()); // swaps the new waveform to front
            m_semaVsyncSlot.acquire();
            Event::end("VsyncThread vsync swap");

            m_timer.restart();
            m_usWaitToSwap = 1000;
            usleep(1000);
        } else { // if (m_vSyncMode == ST_TIMER) {

            Event::start("VsyncThread vsync render");
            emit(vsyncRender()); // renders the new waveform.

            // wait until rendering was scheduled. It might be delayed due a
            // pending swap (depends one driver vSync settings)
            m_semaVsyncSlot.acquire();
            Event::end("VsyncThread vsync render");

            // qDebug() << "ST_TIMER                      " << usLast << usRest;
            int usRemainingForSwap = m_usWaitToSwap - (int)m_timer.elapsed() / 1000;
            // waiting for interval by sleep
            if (usRemainingForSwap > 100) {
                Event::start("VsyncThread usleep for VSync");
                usleep(usRemainingForSwap);
                Event::end("VsyncThread usleep for VSync");
            }

            Event::start("VsyncThread vsync swap");
            // swaps the new waveform to front in case of gl-wf
            emit(vsyncSwap());

            // wait until swap occurred. It might be delayed due to driver vSync
            // settings.
            m_semaVsyncSlot.acquire();
            Event::end("VsyncThread vsync swap");

            // <- Assume we are VSynced here ->
            int usLastSwapTime = (int)m_timer.restart() / 1000;
            if (usRemainingForSwap < 0) {
                // Our swapping call was already delayed
                // The real swap might happens on the following VSync, depending on driver settings
                m_droppedFrames++; // Count as Real Time Error
                droppedFrames.increment();
            }
            // try to stay in right intervals
            m_usWaitToSwap = m_usSyncIntervalTime +
                    ((m_usWaitToSwap - usLastSwapTime) % m_usSyncIntervalTime);
        }

        // Qt timers are not that useful in our case, because they
        // are handled with priority without respecting the callback
        m_pGuiTick->process();
    }
}


// static
void VSyncThread::swapGl(QGLWidget* glw, int index) {
    Q_UNUSED(index);
    // No need for glw->makeCurrent() here.
    //qDebug() << "swapGl" << m_timer.elapsed();
#if defined(__APPLE__)
    glw->swapBuffers();
#elif defined(__WINDOWS__)
    glw->swapBuffers();
#else
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    const QX11Info *xinfo = qt_x11Info(glw);
    glXSwapBuffers(xinfo->display(), glw->winId());
#else
    glw->swapBuffers();
#endif // QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#endif
}

int VSyncThread::elapsed() {
    return (int)m_timer.elapsed() / 1000;
}

void VSyncThread::setUsSyncIntervalTime(int syncTime) {
    m_usSyncIntervalTime = syncTime;
    m_vSyncPerRendering = round(m_displayFrameRate * m_usSyncIntervalTime / 1000);
}

void VSyncThread::setVSyncType(int type) {
    if (type >= (int)VSyncThread::ST_COUNT) {
        type = VSyncThread::ST_TIMER;
    }
    m_vSyncMode = (enum VSyncMode)type;
    m_droppedFrames = 0;
    m_vSyncTypeChanged = true;
}

int VSyncThread::usToNextSync() {
    int usRest = m_usWaitToSwap - ((int)m_timer.elapsed() / 1000);
    // int math is fine here, because we do not expect times > 4.2 s
    if (usRest < 0) {
        usRest %= m_usSyncIntervalTime;
        usRest += m_usSyncIntervalTime;
    }
    return usRest;
}

int VSyncThread::usFromTimerToNextSync(PerformanceTimer* timer) {
    int usDifference = (int)m_timer.difference(timer) / 1000;
    // int math is fine here, because we do not expect times > 4.2 s
    return usDifference + m_usWaitToSwap;
}

int VSyncThread::droppedFrames() {
    return m_droppedFrames;
}

void VSyncThread::vsyncSlotFinished() {
    m_semaVsyncSlot.release();
}

void VSyncThread::getAvailableVSyncTypes(QList<QPair<int, QString > >* pList) {
    for (int i = (int)VSyncThread::ST_TIMER; i < (int)VSyncThread::ST_COUNT; i++) {
        //if (isAvailable(type))  // TODO
        {
            enum VSyncMode mode = (enum VSyncMode)i;

            QString name;
            switch (mode) {
            case VSyncThread::ST_TIMER:
                name = tr("Timer (Fallback)");
                break;
            case VSyncThread::ST_MESA_VBLANK_MODE_1:
                name = tr("MESA vblank_mode = 1");
                break;
            case VSyncThread::ST_SGI_VIDEO_SYNC:
                name = tr("Wait for Video sync");
                break;
            case VSyncThread::ST_OML_SYNC_CONTROL:
                name = tr("Sync Control");
                break;
            case VSyncThread::ST_FREE:
                name = tr("Free + 1 ms (for benchmark only)");
                break;
            default:
                break;
            }
            QPair<int, QString > pair = QPair<int, QString >(i, name);
            pList->append(pair);
        }
    }
}
