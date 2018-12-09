#include <QThread>
#include <QGLFormat>
#include <QTime>
#include <QtDebug>
#include <QTime>

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

VSyncThread::VSyncThread(QObject* pParent)
        : QThread(pParent),
          m_bDoRendering(true),
          m_vSyncTypeChanged(false),
          m_syncIntervalTimeMicros(33333),  // 30 FPS
          m_waitToSwapMicros(0),
          m_vSyncMode(ST_TIMER),
          m_syncOk(false),
          m_droppedFrames(0),
          m_swapWait(0),
          m_displayFrameRate(60.0),
          m_vSyncPerRendering(1) {
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

    m_waitToSwapMicros = m_syncIntervalTimeMicros;
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
            m_waitToSwapMicros = 1000;
            usleep(1000);
        } else { // if (m_vSyncMode == ST_TIMER) {

            Event::start("VsyncThread vsync render");
            emit(vsyncRender()); // renders the new waveform.

            // wait until rendering was scheduled. It might be delayed due a
            // pending swap (depends one driver vSync settings)
            m_semaVsyncSlot.acquire();
            Event::end("VsyncThread vsync render");

            // qDebug() << "ST_TIMER                      " << lastMicros << restMicros;
            int remainingForSwap = m_waitToSwapMicros - static_cast<int>(
                m_timer.elapsed().toIntegerMicros());
            // waiting for interval by sleep
            if (remainingForSwap > 100) {
                Event::start("VsyncThread usleep for VSync");
                usleep(remainingForSwap);
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
            int lastSwapTime = static_cast<int>(m_timer.restart().toIntegerMicros());
            if (remainingForSwap < 0) {
                // Our swapping call was already delayed
                // The real swap might happens on the following VSync, depending on driver settings
                m_droppedFrames++; // Count as Real Time Error
                droppedFrames.increment();
            }
            // try to stay in right intervals
            m_waitToSwapMicros = m_syncIntervalTimeMicros +
                    ((m_waitToSwapMicros - lastSwapTime) % m_syncIntervalTimeMicros);
        }
    }
}


// static
void VSyncThread::swapGl(QGLWidget* glw, int index) {
    Q_UNUSED(index);
    // No need for glw->makeCurrent() here.
    //qDebug() << "swapGl" << m_timer.elapsed().formatNanosWithUnit();
#if defined(__APPLE__)
    glw->swapBuffers();
#elif defined(__WINDOWS__)
    glw->swapBuffers();
#else
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#ifdef QT_OPENGL_ES_2
    glw->swapBuffers();
#else
    const QX11Info *xinfo = qt_x11Info(glw);
    glXSwapBuffers(xinfo->display(), glw->winId());
#endif
#else
    glw->swapBuffers();
#endif // QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#endif
}

int VSyncThread::elapsed() {
    return static_cast<int>(m_timer.elapsed().toIntegerMicros());
}

void VSyncThread::setSyncIntervalTimeMicros(int syncTime) {
    m_syncIntervalTimeMicros = syncTime;
    m_vSyncPerRendering = round(m_displayFrameRate * m_syncIntervalTimeMicros / 1000);
}

void VSyncThread::setVSyncType(int type) {
    if (type >= (int)VSyncThread::ST_COUNT) {
        type = VSyncThread::ST_TIMER;
    }
    m_vSyncMode = (enum VSyncMode)type;
    m_droppedFrames = 0;
    m_vSyncTypeChanged = true;
}

int VSyncThread::toNextSyncMicros() {
    int rest = m_waitToSwapMicros - static_cast<int>(m_timer.elapsed().toIntegerMicros());
    // int math is fine here, because we do not expect times > 4.2 s
    if (rest < 0) {
        rest %= m_syncIntervalTimeMicros;
        rest += m_syncIntervalTimeMicros;
    }
    return rest;
}

int VSyncThread::fromTimerToNextSyncMicros(const PerformanceTimer& timer) {
    int difference = static_cast<int>(m_timer.difference(timer).toIntegerMicros());
    // int math is fine here, because we do not expect times > 4.2 s
    return difference + m_waitToSwapMicros;
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
