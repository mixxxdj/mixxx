#include "engine/bufferscalers/rubberbandwrapper.h"

#include "engine/bufferscalers/rubberbandworkerpool.h"
#include "engine/engine.h"
#include "util/assert.h"
#include "util/sample.h"

using RubberBand::RubberBandStretcher;

#define RUBBERBANDV3 (RUBBERBAND_API_MAJOR_VERSION >= 3 || \
        (RUBBERBAND_API_MAJOR_VERSION == 2 && RUBBERBAND_API_MINOR_VERSION >= 7))

namespace {

/// The function is used to compute the best number of channel per RB task,
/// depending of the number of channels and available worker. This allows
/// hardware if will less than 8 core to adjust the task distribution in the
/// most optimum way.
///
/// The following table provide the expected number of channel per task with
/// stereo processing for a given number of CPU core (the default behaviour)
///
///  | NbOfCore | Stereo | Stem |
///  |----------|--------|------|
///  | 1        | 2      | 8    |
///  | 2        | 2      | 4    |
///  | 3        | 2      | 2    |
///  | 4        | 2      | 2    |
///
/// The following table provide the expected number of channel per task when the
/// user has explicitly requested stereo channel to be processed as mono
/// channels for a given number of CPU core.
///
///  | NbOfCore | Stereo | Stem |
///  |----------|--------|------|
///  | 1        | 2      | 8    |
///  | 2        | 1      | 4    |
///  | 3        | 1      | 2    |
///  | 4        | 1      | 2    |
///  | 5        | 1      | 2    |
///  | 6        | 1      | 2    |
///  | 7        | 1      | 2    |
///  | 8        | 1      | 1    |

// changed for PreMix
// mixxx::audio::ChannelCount getChannelPerWorker(mixxx::audio::ChannelCount chCount) {
//    RubberBandWorkerPool* pPool = RubberBandWorkerPool::instance();
//
//    // There should always be a pool set, even if multi threading isn't enabled.
//    // This is because multi threading will always be used for stem when
//    // possible.
//    VERIFY_OR_DEBUG_ASSERT(pPool) {
//        return mixxx::kMaxEngineChannelInputCount;
//    }
//    auto channelPerWorker = pPool->channelPerWorker();
//    // The task count includes all the thread in the pool + the engine thread
//    auto maxThreadCount = pPool->maxThreadCount() + 1;
//    VERIFY_OR_DEBUG_ASSERT(chCount % channelPerWorker == 0) {
//        return mixxx::kEngineChannelOutputCount;
//    }
//    auto numTasks = chCount / channelPerWorker;
//    if (numTasks > maxThreadCount) {
//        VERIFY_OR_DEBUG_ASSERT(numTasks % maxThreadCount == 0) {
//            return mixxx::kEngineChannelOutputCount;
//        }
//        return mixxx::audio::ChannelCount(chCount / maxThreadCount);
//    }
//    return channelPerWorker;
//}

// mixxx::audio::ChannelCount getChannelPerWorker(mixxx::audio::ChannelCount chCount) {
//     RubberBandWorkerPool* pPool = RubberBandWorkerPool::instance();
//
//     // There should always be a pool set, even if multi threading isn't enabled.
//     // This is because multi threading will always be used for stem when
//     // possible.
//     VERIFY_OR_DEBUG_ASSERT(pPool) {
//         return mixxx::kMaxEngineChannelInputCount;
//     }
//     auto channelPerWorker = pPool->channelPerWorker();
//     // The task count includes all the thread in the pool + the engine thread
//     auto maxThreadCount = pPool->maxThreadCount() + 1;
//     VERIFY_OR_DEBUG_ASSERT(chCount % channelPerWorker == 0) {
//         return mixxx::kEngineChannelOutputCount;
//     }
//     auto numTasks = chCount / channelPerWorker;
//     if (numTasks > maxThreadCount) {
//         VERIFY_OR_DEBUG_ASSERT(numTasks % maxThreadCount == 0) {
//             return mixxx::kEngineChannelOutputCount;
//         }
//         return mixxx::audio::ChannelCount(chCount / maxThreadCount);
//     }
//     return channelPerWorker;
// }
} // namespace

int RubberBandWrapper::getEngineVersion() const {
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return -1;
    }
#if RUBBERBANDV3
    return m_pInstances[0]->getEngineVersion();
#else
    return 2;
#endif
}
void RubberBandWrapper::setTimeRatio(double ratio) {
    for (auto& stretcher : m_pInstances) {
        stretcher->setTimeRatio(ratio);
    }
}
size_t RubberBandWrapper::getSamplesRequired() const {
    size_t require = 0;
    for (const auto& stretcher : m_pInstances) {
        require = qMax(require, stretcher->getSamplesRequired());
    }
    return require;
}
int RubberBandWrapper::available() const {
    int available = std::numeric_limits<int>::max();
    for (const auto& stretcher : m_pInstances) {
        available = qMin(available, stretcher->available());
    }
    return available == std::numeric_limits<int>::max() ? 0 : available;
}
size_t RubberBandWrapper::retrieve(
        float* const* output, size_t samples, SINT channelBufferSize) const {
    // ensure we don't fetch more samples than we really have available.
    samples = std::min(static_cast<size_t>(available()), samples);
    VERIFY_OR_DEBUG_ASSERT(samples <= static_cast<size_t>(channelBufferSize)) {
        samples = channelBufferSize;
    }
    if (m_pInstances.size() == 1) {
        return m_pInstances[0]->retrieve(output, samples);
    } else {
        for (const auto& stretcher : m_pInstances) {
            size_t numSamplesRetrieved =
                    stretcher->retrieve(output, samples);
            // there is something very wrong if we got a different of amount
            // of samples than we requested
            // We clear the buffer to limit the damage, but the signal
            // interruption will still create undesirable audio artefacts.
            VERIFY_OR_DEBUG_ASSERT(numSamplesRetrieved == samples) {
                if (samples > numSamplesRetrieved) {
                    for (int ch = 0; ch < m_channelPerWorker; ch++) {
                        SampleUtil::clear(output[ch] + numSamplesRetrieved,
                                samples - numSamplesRetrieved);
                    }
                }
            }
            output += m_channelPerWorker;
        }
        return samples;
    }
}
size_t RubberBandWrapper::getInputIncrement() const {
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return {};
    }
    return m_pInstances[0]->getInputIncrement();
}
size_t RubberBandWrapper::getLatency() const {
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return {};
    }
    return m_pInstances[0]->getLatency();
}
double RubberBandWrapper::getPitchScale() const {
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return {};
    }
    return m_pInstances[0]->getPitchScale();
}
// See
// https://github.com/breakfastquay/rubberband/commit/72654b04ea4f0707e214377515119e933efbdd6c
// for how these two functions were implemented within librubberband itself
size_t RubberBandWrapper::getPreferredStartPad() const {
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return {};
    }
#if RUBBERBANDV3
    return m_pInstances[0]->getPreferredStartPad();
#else
    // `getPreferredStartPad()` returns `window_size / 2`, while with
    // `getLatency()` both time stretching engines return `window_size / 2 /
    // pitch_scale`
    return static_cast<size_t>(std::ceil(
            m_pInstances[0]->getLatency() * m_pInstances[0]->getPitchScale()));
#endif
}
size_t RubberBandWrapper::getStartDelay() const {
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return {};
    }
#if RUBBERBANDV3
    return m_pInstances[0]->getStartDelay();
#else
    // In newer Rubber Band versions `getLatency()` is a deprecated alias for
    // `getStartDelay()`, so they should behave the same. In the commit linked
    // above the behavior was different for the R3 stretcher, but that was only
    // during the initial betas of Rubberband 3.0 so we shouldn't have to worry
    // about that.
    return m_pInstances[0]->getLatency();
#endif
}
void RubberBandWrapper::process(const float* const* input, size_t samples, bool isFinal) {
    if (m_pInstances.size() == 1) {
        return m_pInstances[0]->process(input, samples, isFinal);
    } else {
        RubberBandWorkerPool* pPool = RubberBandWorkerPool::instance();
        for (auto& pInstance : m_pInstances) {
            pInstance->set(input, samples, isFinal);
            // We try to get the stretching job ran by the RBPool if there is a
            // worker slot available
            if (!pPool->tryStart(pInstance.get())) {
                // Otherwise, it means the main thread should take care of the stretching
                pInstance->run();
            }
            input += m_channelPerWorker;
        }
        // We always perform a wait, even for task that were ran in the main
        // thread, so it resets the semaphore
        for (auto& pInstance : m_pInstances) {
            pInstance->waitReady();
        }
    }
}
void RubberBandWrapper::reset() {
    for (auto& stretcher : m_pInstances) {
        stretcher->reset();
    }
}
void RubberBandWrapper::clear() {
    m_pInstances.clear();
}
void RubberBandWrapper::setup(mixxx::audio::SampleRate sampleRate,
        mixxx::audio::ChannelCount chCount,
        const RubberBandStretcher::Options& opt) {
    // The instance should have been cleared, or not set before
    VERIFY_OR_DEBUG_ASSERT(m_pInstances.size() == 0) {
        m_pInstances.clear();
    };

    // Premix: test to override getChannelPerWorker(chCount)
    // m_channelPerWorker = getChannelPerWorker(chCount);
    m_channelPerWorker = mixxx::audio::ChannelCount(2);

    qDebug() << "RubberBandWrapper::setup - using" << m_channelPerWorker << "channel(s) per task";
    VERIFY_OR_DEBUG_ASSERT(0 == chCount % m_channelPerWorker) {
        // If we have an uneven number of channel, which we can't evenly
        // distribute across the RubberBandPool workers, we fallback to using a
        // single instance to limit the audio imperfection that may come from
        // using RB with different parameters.
        m_pInstances.emplace_back(
                std::make_unique<RubberBandTask>(
                        sampleRate, chCount, opt));
        return;
    }

    m_pInstances.reserve(chCount / m_channelPerWorker);
    for (int c = 0; c < chCount; c += m_channelPerWorker) {
        m_pInstances.emplace_back(
                std::make_unique<RubberBandTask>(
                        sampleRate, m_channelPerWorker, opt));
    }
}
void RubberBandWrapper::setPitchScale(double scale) {
    for (auto& stretcher : m_pInstances) {
        stretcher->setPitchScale(scale);
    }
}

bool RubberBandWrapper::isValid() const {
    return !m_pInstances.empty();
}
