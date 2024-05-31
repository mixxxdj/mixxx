#include "engine/bufferscalers/rubberbandtask.h"

#include "engine/engine.h"
#include "util/assert.h"
#include "util/compatibility/qmutex.h"

RubberBandTask::RubberBandTask(
        size_t sampleRate, size_t channels, Options options)
        : RubberBand::RubberBandStretcher(sampleRate, channels, options),
          QRunnable(),
          m_completed(false),
          m_input(nullptr),
          m_samples(0),
          m_isFinal(false) {
    setAutoDelete(false);
}

void RubberBandTask::set(const float* const* input,
        size_t samples,
        bool isFinal) {
    auto locker = lockMutex(&m_waitLock);
    m_completed = false;
    m_input = input;
    m_samples = samples;
    m_isFinal = isFinal;
}

void RubberBandTask::waitReady() {
    auto locker = lockMutex(&m_waitLock);
    VERIFY_OR_DEBUG_ASSERT(m_input && m_samples) {
        return;
    };
    while (!m_completed) {
        m_waitCondition.wait(&m_waitLock);
    }
}

void RubberBandTask::run() {
    auto locker = lockMutex(&m_waitLock);
    VERIFY_OR_DEBUG_ASSERT(!m_completed && m_input && m_samples) {
        return;
    };
    process(m_input,
            m_samples,
            m_isFinal);
    m_completed = true;
    m_waitCondition.wakeOne();
}
