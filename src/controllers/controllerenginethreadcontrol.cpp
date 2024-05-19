#include "controllers/controllerenginethreadcontrol.h"

#include <QCoreApplication>

#include "moc_controllerenginethreadcontrol.cpp"
#include "util/assert.h"
#include "util/logger.h"
#include "util/mutex.h"
#include "util/thread_affinity.h"

namespace {
const mixxx::Logger kLogger("ControllerEngineThreadControl");
constexpr int kMaxPauseDurationMilliseconds = 1000;
} // namespace

ControllerEngineThreadControl::ControllerEngineThreadControl(QObject* parent)
        : QObject(parent) {
}
bool ControllerEngineThreadControl::pause() {
    VERIFY_OR_DEBUG_ASSERT_THIS_QOBJECT_THREAD_ANTI_AFFINITY() {
        return false;
    }
    const auto lock = lockMutex(&m_pauseMutex);
    m_pauseCount++;

    if (m_canPause && !m_isPaused) {
        emit pauseRequested();
    }

    while (m_canPause && !m_isPaused) {
        if (!m_isPausedCondition.wait(&m_pauseMutex, kMaxPauseDurationMilliseconds)) {
            kLogger.warning() << "Pause request timed out!";
            m_pauseCount--;
            return false;
        }
    }
    return !m_canPause || m_isPaused;
}
void ControllerEngineThreadControl::resume() {
    VERIFY_OR_DEBUG_ASSERT_THIS_QOBJECT_THREAD_ANTI_AFFINITY() {
        return;
    }
    const auto lock = lockMutex(&m_pauseMutex);
    if (m_pauseCount > 0) {
        m_pauseCount--;
    }
    m_isPaused = m_pauseCount > 0;
    m_isPausedCondition.wakeOne();
}
void ControllerEngineThreadControl::setCanPause(bool canPause) {
    DEBUG_ASSERT_THIS_QOBJECT_THREAD_AFFINITY();
    auto lock = lockMutex(&m_pauseMutex);
    m_canPause = canPause;

    if (m_canPause) {
        connect(this,
                &ControllerEngineThreadControl::pauseRequested,
                this,
                &ControllerEngineThreadControl::doPause,
                Qt::UniqueConnection);
    } else {
        // New signals may have been queued emitted requesting for pause, so we
        // manually process the event loop now to clear and handle those, before
        // disabling pausing. Without this, thread requesting pause will stay
        // stuck waiting on the condvar.
        lock.unlock();
        QCoreApplication::processEvents();
        lock.relock();

        disconnect(this,
                &ControllerEngineThreadControl::pauseRequested,
                this,
                &ControllerEngineThreadControl::doPause);

        m_isPaused = false;
        m_pauseCount = 0;
        m_isPausedCondition.wakeOne();
    }
}
void ControllerEngineThreadControl::doPause() {
    VERIFY_OR_DEBUG_ASSERT_THIS_QOBJECT_THREAD_AFFINITY() {
        return;
    }
    const auto lock = lockMutex(&m_pauseMutex);
    m_isPaused = m_pauseCount > 0;
    m_isPausedCondition.wakeOne();

    while (m_canPause && m_isPaused) {
        VERIFY_OR_DEBUG_ASSERT(m_isPausedCondition.wait(
                &m_pauseMutex, kMaxPauseDurationMilliseconds)) {
            kLogger.warning() << "Engine pause timed out!";
            m_isPaused = false;
            m_pauseCount = 0;
        };
    }
    m_isPausedCondition.wakeAll();
}
