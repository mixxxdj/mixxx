#include <QtDebug>

#include "renderthread.h"
#include "util/performancetimer.h"
#include "util/event.h"
#include "util/counter.h"
#include "util/math.h"
#include "util/time.h"

RenderThread::RenderThread(QObject* pParent)
        : QThread(pParent),
          m_bDoRendering(true),
          m_syncIntervalTimeMicros(33333),  // 30 FPS
          m_renderMode(ST_TIMER),
          m_droppedFrames(0) {
}

RenderThread::~RenderThread() {
    m_bDoRendering = false;
    m_semaRenderSlot.release(2); // Two slots
    wait();
}

void RenderThread::stop() {
    m_bDoRendering = false;
}


void RenderThread::run() {
    Counter droppedFrames("RenderThread real time error");
    QThread::currentThread()->setObjectName("RenderThread");

    m_timer.start();

    while (m_bDoRendering) {
        if (m_renderMode == ST_FREE) {
            // for benchmark only!

            Event::start("RenderThread render");
            // renders the waveform, Possible delayed due to anti tearing
            emit(render());
            m_semaRenderSlot.acquire();
            Event::end("RenderThread render");

            m_timer.restart();
            usleep(1000);
        } else { // if (m_renderMode == ST_TIMER) {
            m_timer.restart();

            Event::start("RenderThread render");
            emit(render()); // renders the new waveform.

            // wait until rendering was scheduled. It might be delayed due a
            // pending swap (depends one driver render settings)
            m_semaRenderSlot.acquire();
            Event::end("RenderThread render");

            int elapsed = m_timer.restart().toIntegerMicros();
            int sleepTimeMicros = m_syncIntervalTimeMicros - elapsed;
            //qDebug() << "RenderThread sleepTimeMicros" << sleepTimeMicros;
            if (sleepTimeMicros > 100) {
                usleep(sleepTimeMicros);
            } else if (sleepTimeMicros < 0) {
                m_droppedFrames++;
            }
        }
    }
}

int RenderThread::elapsed() {
    return static_cast<int>(m_timer.elapsed().toIntegerMicros());
}

void RenderThread::setSyncIntervalTimeMicros(int syncTime) {
    m_syncIntervalTimeMicros = syncTime;
}

void RenderThread::setRenderType(int type) {
    if (type >= (int)RenderThread::ST_COUNT) {
        type = RenderThread::ST_TIMER;
    }
    m_renderMode = (enum RenderMode)type;
    m_droppedFrames = 0;
}

int RenderThread::droppedFrames() {
    return m_droppedFrames;
}

void RenderThread::renderSlotFinished() {
    m_semaRenderSlot.release();
}
