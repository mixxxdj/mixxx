#include "vsyncthread.h"

#include "moc_vsyncthread.cpp"
#include "util/math.h"
#include "util/performancetimer.h"

namespace {

constexpr int kNumStableDeltasRequired = 20;

VSyncThread::VSyncMode defaultVSyncMode() {
#ifdef __APPLE__
    return VSyncThread::ST_PLL;
#else
    return VSyncThread::ST_TIMER;
#endif
}

} // namespace

VSyncThread::VSyncThread(QObject* pParent, VSyncThread::VSyncMode vSyncMode)
        : QThread(pParent),
          m_bDoRendering(true),
          m_syncIntervalTimeMicros(33333), // 30 FPS
          m_waitToSwapMicros(0),
          m_vSyncMode(vSyncMode == VSyncThread::ST_DEFAULT ? defaultVSyncMode() : vSyncMode),
          m_syncOk(false),
          m_droppedFrames(0),
          m_swapWait(0),
          m_displayFrameRate(60.0),
          m_vSyncPerRendering(1),
          m_pllInitCnt(0),
          m_pllInitSum(0.0),
          m_pllPhaseOut(0.0),
          m_pllDeltaOut(16666.6),
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
        m_syncIntervalTimeMicros = 1000;
        m_waitToSwapMicros = m_syncIntervalTimeMicros;
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
        usleep(m_waitToSwapMicros);
    }
}

void VSyncThread::runPLL() {
    assert(m_vSyncMode == ST_PLL);
    qint64 offsetAdjustedAt = 0;
    qint64 offset = 0;
    qint64 nextSwapMicros = 0;
    qint64 multiplierAdjustedPllPhaseOut = 0;
    while (m_bDoRendering) {
        // Use a phase-locked-loop on the QOpenGLWindow::frameSwapped signal
        // to determine when the vsync occurs

        qint64 pllPhaseOut;
        qint64 pllDeltaOut;
        qint64 multiplierAdjustedPllDeltaOut;
        qint64 now;

        {
            std::scoped_lock lock(m_pllMutex);
            // last estimated vsync
            pllPhaseOut = std::llround(m_pllPhaseOut);
            // estimated frame interval
            pllDeltaOut = std::llround(m_pllDeltaOut);
            now = m_pllTimer.elapsed().toIntegerMicros();

            // Calculate the nearest integer number of PLL intervals that
            // correspond with the interval based on the frame rate from the
            // user settings. E.g. if the PLL is running at 60 fps, and the user
            // settings is between 25 and 40 fps, we do run effectively at 30
            // fps (multiplier is 2)
            //
            // Note this also is applied when running at 120 fps (ProMotion)
            const auto multiplier = std::max<qint64>(1,
                    (m_syncIntervalTimeMicros + pllDeltaOut / 2) / pllDeltaOut);

            // Update the multiplierAdjustedPllPhaseOut to pllPhaseOut, if pllPhaseOut has increased
            // multiplier * pllDeltaOut intervals.
            if (multiplierAdjustedPllPhaseOut == 0 ||
                    ((pllPhaseOut - multiplierAdjustedPllPhaseOut +
                             pllDeltaOut / 2) /
                            pllDeltaOut) %
                                    multiplier ==
                            0) {
                multiplierAdjustedPllPhaseOut = pllPhaseOut;
            }

            multiplierAdjustedPllDeltaOut = pllDeltaOut * multiplier;
        }

        if (multiplierAdjustedPllPhaseOut > nextSwapMicros) {
            // We received a new pll phase
            nextSwapMicros = multiplierAdjustedPllPhaseOut;
        } else {
            // We didn't receive a new pll phase out, so freewheel to estimated
            // next with the current delta.
            nextSwapMicros += multiplierAdjustedPllDeltaOut;
        }

        qint64 sleepUntilSwap = (nextSwapMicros + offset - now) % multiplierAdjustedPllDeltaOut;
        if (sleepUntilSwap < 0) {
            sleepUntilSwap += multiplierAdjustedPllDeltaOut;
        }
        usleep(sleepUntilSwap);

        m_sinceLastSwap = m_timer.restart();
        m_waitToSwapMicros = multiplierAdjustedPllDeltaOut;

        // Signal to swap the gl widgets (waveforms, spinnies, vumeters)
        // and render them for the next swap
        if (!pllInitializing() || m_pllPendingUpdate) {
            emit vsyncSwapAndRender();
            m_semaVsyncSlot.acquire();
            m_pllPendingUpdate = false;
        }

        if (m_sinceLastSwap.toIntegerMicros() > multiplierAdjustedPllDeltaOut * 3 / 2) {
            // Too much time passed since last swap: consider frame dropped.
            // Automatically adjust the time offset between the PLL and our signal,
            // ideally settling on an offset with no or little frame drops.
            const auto sinceLastOffsetAdjust = now - offsetAdjustedAt;
            // Don't adjust too often (max once every 100 ms)
            if (sinceLastOffsetAdjust > 100000) {
                // And don't adjust (immediately) if we have been running
                // without drops for over 1 second
                if (sinceLastOffsetAdjust < 1000000) {
                    offset = (offset + pllDeltaOut / 8) % multiplierAdjustedPllDeltaOut;
                }
                offsetAdjustedAt = now;
            }
            m_droppedFrames++;
            qDebug() << "DROP";
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
    if (m_vSyncMode != ST_FREE) {
        m_syncIntervalTimeMicros = syncTime;
        m_vSyncPerRendering = static_cast<int>(
                round(m_displayFrameRate * m_syncIntervalTimeMicros / 1000));
    }
}

std::chrono::microseconds VSyncThread::fromTimerToNextSync(const PerformanceTimer& timer) {
    int difference = static_cast<int>(m_timer.difference(timer).toIntegerMicros());
    // int math is fine here, because we do not expect times > 4.2 s
    int toNextSync = difference + m_waitToSwapMicros;
    while (toNextSync < 0) {
        // this function is called during rendering. A negative value indicates
        // an attempt to render an outdated frame. Render the next frame instead
        toNextSync += m_syncIntervalTimeMicros;
    }
    return std::chrono::microseconds(toNextSync);
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
            case VSyncThread::ST_MESA_VBLANK_MODE_1_DEPRECATED:
                name = tr("MESA vblank_mode = 1");
                break;
            case VSyncThread::ST_SGI_VIDEO_SYNC_DEPRECATED:
                name = tr("Wait for Video sync");
                break;
            case VSyncThread::ST_OML_SYNC_CONTROL_DEPRECATED:
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

bool VSyncThread::pllInitializing() const {
    return m_pllInitCnt < kNumStableDeltasRequired;
}

void VSyncThread::updatePLL() {
    std::scoped_lock lock(m_pllMutex);

    m_pllPendingUpdate = false;

    // Phase-lock-looped to estimate the vsync based on the
    // QOpenGLWindow::frameSwapped signal

    const double pllPhaseIn = m_pllTimer.elapsed().toDoubleMicros();

    if (m_pllInitCnt < kNumStableDeltasRequired) {
        // Before activating the phase-lock-looped, we need an initial
        // delta and phase, which we calculate by taking the average
        // delta over a number of stable deltas.
        const double delta = pllPhaseIn - m_pllPhaseOut;
        m_pllPhaseOut = pllPhaseIn;
        m_pllInitSum += delta;
        m_pllInitCnt++;
        m_pllInitAvg = m_pllInitSum / static_cast<double>(m_pllInitCnt);
        if (std::abs(delta - m_pllInitAvg) > 2000.0) {
            // The current delta is too different from the current
            // average so we reset the init process.
            m_pllInitSum = 0.0;
            m_pllInitCnt = 0;
        }
        if (m_pllInitCnt == kNumStableDeltasRequired) {
            m_pllDeltaOut = m_pllInitAvg;
        }
        return;
    }

    // inspired by https://liquidsdr.org/blog/pll-simple-howto/
    const double alpha = 0.01;               // the page above uses 0.05, but a more narrow
                                             // filter seems to work better here
    const double beta = 0.5 * alpha * alpha; // increment adjustment factor

    m_pllPhaseOut += m_pllDeltaOut;

    double pllPhaseError = pllPhaseIn - m_pllPhaseOut;

    if (pllPhaseError > 0) {
        // when advanced more than a frame, jump to the nearest frame
        m_pllPhaseOut += std::round(pllPhaseError / m_pllDeltaOut) * m_pllDeltaOut;
        pllPhaseError = pllPhaseIn - m_pllPhaseOut;
    }

    // apply loop filter and correct output phase and delta
    m_pllPhaseOut += alpha * pllPhaseError; // adjust phase
    m_pllDeltaOut += beta * pllPhaseError;  // adjust delta

    if (pllPhaseIn > m_pllLogging) {
        if (m_pllLogging == 0) {
            m_pllLogging = pllPhaseIn;
        } else {
            qDebug() << "phase-locked-loop:" << std::llround(m_pllPhaseOut)
                     << m_pllDeltaOut << pllPhaseError;
        }
        // log every 10 seconds
        m_pllLogging += 10000000.0;
    }

    m_pllPendingUpdate = true;
}
