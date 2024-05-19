#include "engine/bufferscalers/rubberbandworker.h"

#include "engine/engine.h"
#include "moc_rubberbandworker.cpp"
#include "util/assert.h"
#include "util/compatibility/qmutex.h"

using RubberBand::RubberBandStretcher;

RubberBandWorker::RubberBandWorker()
        : QThread(),
          m_currentJob(Job{nullptr, nullptr, 0, false}),
          m_completed(false) {
}

void RubberBandWorker::schedule(RubberBand::RubberBandStretcher* stretcher,
        const float* const* input,
        size_t samples,
        bool final) {
    auto locker = lockMutex(&m_waitLock);
    m_currentJob.instance = stretcher;
    m_currentJob.input = input;
    m_currentJob.samples = samples;
    m_currentJob.final = final;
    m_completed = false;
    m_waitCondition.wakeOne();
}

void RubberBandWorker::waitReady() {
    auto locker = lockMutex(&m_waitLock);
    while (!m_completed) {
        m_waitCondition.wait(&m_waitLock);
    }
}

void RubberBandWorker::stop() {
    requestInterruption();
    m_waitCondition.wakeOne();
    wait();
}
void RubberBandWorker::run() {
    auto locker = lockMutex(&m_waitLock);
    while (!isInterruptionRequested()) {
        if (!m_completed && m_currentJob.instance && m_currentJob.input && m_currentJob.samples) {
            m_currentJob.instance->process(m_currentJob.input,
                    m_currentJob.samples,
                    m_currentJob.final);
            m_completed = true;
            DEBUG_ASSERT(m_assigned.test(std::memory_order_relaxed));
            m_assigned.clear(std::memory_order_release);
            m_waitCondition.wakeOne();
        }
        m_waitCondition.wait(&m_waitLock);
    }
    quit();
}
