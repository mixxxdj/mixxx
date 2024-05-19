#include "engine/bufferscalers/rubberbandworkerpool.h"

#include <rubberband/RubberBandStretcher.h>

#include "engine/bufferscalers/rubberbandworker.h"
#include "engine/engine.h"
#include "util/assert.h"

RubberBandWorkerPool::RubberBandWorkerPool() {
    DEBUG_ASSERT(0 == mixxx::kEngineChannelCount % RubberBandWorker::kChannelPerWorker);

    // We allocate one runner less than the total of maximum supported channel,
    // so the engine thread will also perform a stretching operation, instead of
    // waiting all workers to complete. During performance testing, this ahas
    // show better results
    for (int w = 1; w < mixxx::kEngineChannelCount / RubberBandWorker::kChannelPerWorker; w++) {
        m_workers.emplace_back(
                // We cannot use make_unique here because RubberBandWorker ctor
                // is protected to prevent direct usage.
                new RubberBandWorker);
        m_workers.back()->start(QThread::HighPriority);
    }
}

RubberBandWorkerPool::~RubberBandWorkerPool() {
    for (auto& member : m_workers) {
        member->stop();
    }
    m_workers.clear();
}

RubberBandWorker* RubberBandWorkerPool::submit(
        RubberBand::RubberBandStretcher* stretcher,
        const float* const* input,
        size_t samples,
        bool final) {
    for (auto& member : m_workers) {
        if (!member->m_assigned.test_and_set(std::memory_order_acquire)) {
            member->schedule(stretcher, input, samples, final);
            return member.get();
        }
    }
    return nullptr;
}
