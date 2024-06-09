#include "engine/bufferscalers/rubberbandworkerpool.h"

#include <rubberband/RubberBandStretcher.h>

#include "engine/engine.h"
#include "util/assert.h"

RubberBandWorkerPool::RubberBandWorkerPool(UserSettingsPointer pConfig)
        : QThreadPool() {
    bool multiThreadedOnStereo = pConfig &&
            pConfig->getValue(ConfigKey(QStringLiteral("[App]"),
                                      QStringLiteral("keylock_multithreading")),
                    false);
    m_channelPerWorker = multiThreadedOnStereo
            ? mixxx::audio::ChannelCount::mono()
            : mixxx::audio::ChannelCount::stereo();
    DEBUG_ASSERT(mixxx::kMaxEngineChannelInputCount % m_channelPerWorker == 0);

    setThreadPriority(QThread::HighPriority);
    setMaxThreadCount(mixxx::kMaxEngineChannelInputCount / m_channelPerWorker - 1);

    // We allocate one runner less than the total of maximum supported channel,
    // so the engine thread will also perform a stretching operation, instead of
    // waiting all workers to complete. During performance testing, this ahas
    // show better results
    for (int w = 0; w < maxThreadCount(); w++) {
        reserveThread();
    }
}
