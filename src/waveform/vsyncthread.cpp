#include "vsyncthread.h"

#include "moc_vsyncthread.cpp"
#include "util/math.h"
#include "util/performancetimer.h"

VSyncThread::VSyncThread(QObject* pParent)
        : QThread(pParent),
          m_bDoRendering(true),
          m_vSyncTypeChanged(false),
          m_syncIntervalTimeMicros(33333), // 30 FPS
          m_waitToSwapMicros(0),
          m_vSyncMode(ST_TIMER),
          m_syncOk(false),
          m_droppedFrames(0),
          m_swapWait(0),
          m_displayFrameRate(60.0),
          m_vSyncPerRendering(1),
          m_pllPhaseOut(0.0),
          m_pllDeltaOut(16666.6), // 60 FPS initial delta
          m_pllLogging(0.0) {
    m_pllTimer.start();
}

VSyncThread::~VSyncThread() {
    m_bDoRendering = false;
    m_semaVsyncSlot.release(m_vSyncMode == ST_PLL ? 1 : 2); // Two slots, one for PLL
    wait();
    //delete m_glw;
}

void VSyncThread::run() {
    QThread::currentThread()->setObjectName("VSyncThread");

    m_waitToSwapMicros = m_syncIntervalTimeMicros;
    m_timer.start();

    //qDebug() << "VSyncThread::run()";
    switch (m_vSyncMode) {
    case ST_FREE:
        runFree();
        break;
    case ST_PLL:
        runPLL();
        break;
    case ST_TIMER:
        runTimer();
        break;
    default:
        assert(false);
        break;
    }
}

void VSyncThread::runFree() {
    assert(m_vSyncMode == ST_FREE);
    while (m_bDoRendering) {
        // for benchmark only!

        // renders the waveform, Possible delayed due to anti tearing
        emit vsyncRender();
        m_semaVsyncSlot.acquire();

        emit vsyncSwap(); // swaps the new waveform to front
        m_semaVsyncSlot.acquire();

        m_sinceLastSwap = m_timer.restart();
        m_waitToSwapMicros = 1000;
        usleep(1000);
    }
}

void VSyncThread::runPLL() {
    assert(m_vSyncMode == ST_PLL);
    qint64 offset = 0;
    qint64 nextSwapMicros = 0;
    while (m_bDoRendering) {
        // Use a phase-locked-loop on the QOpenGLWindow::frameSwapped signal
        // to determine when the vsync occurs

        qint64 pllPhaseOut;
        qint64 pllDeltaOut;
        qint64 now;

        {
            std::scoped_lock lock(m_pllMutex);
            // last estimated vsync
            pllPhaseOut = std::llround(m_pllPhaseOut);
            // estimated frame interval
            pllDeltaOut = std::llround(m_pllDeltaOut);
            now = m_pllTimer.elapsed().toIntegerMicros();
        }
        if (pllPhaseOut > nextSwapMicros) {
            nextSwapMicros = pllPhaseOut;
        }
        if (nextSwapMicros == pllPhaseOut) {
            nextSwapMicros += pllDeltaOut;
        }

        // sleep an integer number of frames extra to approximate the
        // selected framerate (eg 10,15,20,30)
        const auto skippedFrames = (m_syncIntervalTimeMicros - pllDeltaOut / 2) / pllDeltaOut;
        qint64 sleepForSkippedFrames = skippedFrames * pllDeltaOut;

        qint64 sleepUntilSwap = (nextSwapMicros + offset - now) % pllDeltaOut;
        if (sleepUntilSwap < 0) {
            sleepUntilSwap += pllDeltaOut;
        }
        usleep(sleepUntilSwap + sleepForSkippedFrames);

        m_sinceLastSwap = m_timer.restart();
        m_waitToSwapMicros = pllDeltaOut + sleepForSkippedFrames;

        // Signal to swap the gl widgets (waveforms, spinnies, vumeters)
        // and render them for the next swap
        emit vsyncSwapAndRender();
        m_semaVsyncSlot.acquire();
        if (m_sinceLastSwap.toIntegerMicros() > sleepForSkippedFrames + pllDeltaOut * 3 / 2) {
            m_droppedFrames++;
            // Adjusting the offset on each frame drop ends up at
            // an offset with no frame drops
            offset = (offset + 2000) % pllDeltaOut;
        }
    }
}

void VSyncThread::runTimer() {
    assert(m_vSyncMode == ST_TIMER);

    while (m_bDoRendering) {
        emit vsyncRender(); // renders the new waveform.

        // wait until rendering was scheduled. It might be delayed due a
        // pending swap (depends one driver vSync settings)
        m_semaVsyncSlot.acquire();

        // qDebug() << "ST_TIMER                      " << lastMicros << restMicros;
        int remainingForSwap = m_waitToSwapMicros -
                static_cast<int>(m_timer.elapsed().toIntegerMicros());
        // waiting for interval by sleep
        if (remainingForSwap > 100) {
            usleep(remainingForSwap);
        }

        // swaps the new waveform to front in case of gl-wf
        emit vsyncSwap();

        // wait until swap occurred. It might be delayed due to driver vSync
        // settings.
        m_semaVsyncSlot.acquire();

        // <- Assume we are VSynced here ->
        m_sinceLastSwap = m_timer.restart();
        int lastSwapTime = static_cast<int>(m_sinceLastSwap.toIntegerMicros());
        if (remainingForSwap < 0) {
            // Our swapping call was already delayed
            // The real swap might happens on the following VSync, depending on driver settings
            m_droppedFrames++; // Count as Real Time Error
        }
        // try to stay in right intervals
        m_waitToSwapMicros = m_syncIntervalTimeMicros +
                ((m_waitToSwapMicros - lastSwapTime) % m_syncIntervalTimeMicros);
    }
}

int VSyncThread::elapsed() {
    return static_cast<int>(m_timer.elapsed().toIntegerMicros());
}

void VSyncThread::setSyncIntervalTimeMicros(int syncTime) {
    m_syncIntervalTimeMicros = syncTime;
    m_vSyncPerRendering = static_cast<int>(
            round(m_displayFrameRate * m_syncIntervalTimeMicros / 1000));
}

void VSyncThread::setVSyncType(int type) {
    // qDebug() << "setting vsync type" << type;

    if (type >= (int)VSyncThread::ST_COUNT) {
        type = VSyncThread::ST_TIMER;
    }
    m_vSyncMode = (enum VSyncMode)type;
    m_droppedFrames = 0;
    m_vSyncTypeChanged = true;
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

void VSyncThread::getAvailableVSyncTypes(QList<QPair<int, QString>>* pList) {
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
            case VSyncThread::ST_PLL:
                name = tr("frameSwapped-signal driven phase locked loop");
                break;
            default:
                break;
            }
            QPair<int, QString > pair = QPair<int, QString >(i, name);
            pList->append(pair);
        }
    }
}

mixxx::Duration VSyncThread::sinceLastSwap() const {
    return m_sinceLastSwap;
}

void VSyncThread::updatePLL() {
    std::scoped_lock lock(m_pllMutex);

    // Phase-lock-looped to estimate the vsync based on the
    // QOpenGLWindow::frameSwapped signal

    // inspired by https://liquidsdr.org/blog/pll-simple-howto/
    const double alpha = 0.01;               // the page above uses 0.05, but a more narrow
                                             // filter seems to work better here
    const double beta = 0.5 * alpha * alpha; // increment adjustment factor

    const double pllPhaseIn = m_pllTimer.elapsed().toDoubleMicros();

    m_pllPhaseOut += m_pllDeltaOut;

    double pllPhaseError = pllPhaseIn - m_pllPhaseOut;

    if (pllPhaseError > 0) {
        // when advanced more than a frame, jump to the current frame
        m_pllPhaseOut += std::floor(pllPhaseError / m_pllDeltaOut) * m_pllDeltaOut;
        pllPhaseError = pllPhaseIn - m_pllPhaseOut;
    }

    // apply loop filter and correct output phase and delta
    m_pllPhaseOut += alpha * pllPhaseError; // adjust phase
    m_pllDeltaOut += beta * pllPhaseError;  // adjust delta

    if (pllPhaseIn > m_pllLogging) {
        if (m_pllLogging == 0) {
            m_pllLogging = pllPhaseIn;
        } else {
            qDebug() << "phase-locked-loop:" << m_pllPhaseOut << m_pllDeltaOut;
        }
        // log every 10 seconds
        m_pllLogging += 10000000.0;
    }
}
