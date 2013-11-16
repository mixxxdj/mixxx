
#include <QThread>
#include <QGLWidget>
#include <QGLFormat>
#include <QTime>
#include <qdebug.h>
#include <QTime>

#include "mathstuff.h"
#include "vsyncthread.h"
#include "util/performancetimer.h"

#if defined(__APPLE__)

#elif defined(__WINDOWS__)

#else
   extern const QX11Info *qt_x11Info(const QPaintDevice *pd);
#endif

VSyncThread::VSyncThread(QWidget* parent)
        : QThread(parent),
          m_vSyncTypeChanged(false),
          m_usSyncTime(33333),
          m_vSyncMode(ST_TIMER),
          m_syncOk(false),
          m_rtErrorCnt(0),
          m_swapWait(0),
          m_displayFrameRate(60.0),
          m_interval(1) {
    doRendering = true;
}

VSyncThread::~VSyncThread() {
    doRendering = false;
    m_sema.release(2); // Two slots
    wait();
    //delete m_glw;
}

void VSyncThread::stop()
{
    doRendering = false;
}


void VSyncThread::run() {
    QThread::currentThread()->setObjectName("VSyncThread");

    int usRest;
    int usLast;

    m_usWait = m_usSyncTime;
    m_timer.start();

    while (doRendering) {
        if (m_vSyncMode == ST_FREE) {
            // for benchmark only!
            emit(vsync1()); // renders the waveform, Possible delayed due to anti tearing
            m_sema.acquire();
            emit(vsync2()); // swaps the new waveform to front
            m_sema.acquire();
            m_timer.restart();
            m_usWait = 1000;
            usleep(1000);
        } else { // if (m_vSyncMode == ST_TIMER) {
            usRest = m_usWait - m_timer.elapsed() / 1000;
            // waiting for interval by sleep
            if (usRest > 100) {
                usleep(usRest);
            }

            emit(vsync2()); // swaps the new waveform to front in case of gl-wf
            m_sema.acquire(); // wait until swap was scheduled. It might be delayed due to driver vSync settings  
            // <- Assume we are VSynced here ->
            usLast = m_timer.restart() / 1000;
            if (usRest < 0) {
                // Our swapping call was already delayed
                // The real swap might happens on the following VSync, depending on driver settings 
                m_rtErrorCnt++; // Count as Real Time Error
            }
            // try to stay in right intervals
            usRest = m_usWait - usLast;
            m_usWait = m_usSyncTime + (usRest % m_usSyncTime);
            emit(vsync1()); // renders the new waveform.
            m_sema.acquire(); // wait until rendreing was scheduled. It might be delayed due a pending swap (depends one driver vSync settings) 
 			// qDebug() << "ST_TIMER                      " << usLast << usRest;
        }
    }
}


void VSyncThread::postRender(QGLWidget* glw, int index) {
    Q_UNUSED(index);
    // No need for glw->makeCurrent() here.
    //qDebug() << "postRender" << m_timer.elapsed();
#if defined(__APPLE__)
    glw->swapBuffers();
#elif defined(__WINDOWS__)
    glw->swapBuffers();
#else
    const QX11Info *xinfo = qt_x11Info(glw);
    glXSwapBuffers(xinfo->display(), glw->winId());
#endif
}

int VSyncThread::elapsed() {
    return m_timer.elapsed() / 1000;
}

void VSyncThread::setUsSyncTime(int syncTime) {
    m_usSyncTime = syncTime;
    m_interval = round(m_displayFrameRate * m_usSyncTime / 1000);
}

void VSyncThread::setVSyncType(int type) {
    if (type >= (int)VSyncThread::ST_COUNT) {
        type = VSyncThread::ST_TIMER;
    }
    m_vSyncMode = (enum VSyncMode)type;
    m_rtErrorCnt = 0;
    m_vSyncTypeChanged = true;
}

int VSyncThread::usToNextSync() {
    int usRest = m_usWait - m_timer.elapsed() / 1000;
    if (usRest < 0) {
        usRest %= m_usSyncTime;
        usRest += m_usSyncTime;
    }
    return usRest;
}

int VSyncThread::usFromTimerToNextSync(PerformanceTimer* timer) {
    int difference = m_timer.difference(timer) / 1000;
    return difference + m_usWait;
}

int VSyncThread::rtErrorCnt() {
    return m_rtErrorCnt;
}

void VSyncThread::vsyncSlotFinished() {
    m_sema.release();
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
