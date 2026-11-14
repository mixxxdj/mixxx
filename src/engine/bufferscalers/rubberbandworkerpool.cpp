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

    int numCore = QThread::idealThreadCount();
    int numRBTasks = qMin(numCore, mixxx::kMaxEngineChannelInputCount / m_channelPerWorker);

    qDebug() << "RubberBand will use" << numRBTasks << "tasks to scale the audio signal";

    setThreadPriority(QThread::HighPriority);
    // The RB pool will only be used to scale n-1 buffer sample, so the engine
    // thread takes care of the last buffer and doesn't have to be idle.
    setMaxThreadCount(numRBTasks - 1);
}
