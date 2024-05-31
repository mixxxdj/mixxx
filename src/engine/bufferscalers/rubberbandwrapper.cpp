#include "engine/bufferscalers/rubberbandwrapper.h"

#include "engine/bufferscalers/rubberbandworkerpool.h"
#include "engine/engine.h"
#include "util/assert.h"

using RubberBand::RubberBandStretcher;

#define RUBBERBANDV3 (RUBBERBAND_API_MAJOR_VERSION >= 2 && RUBBERBAND_API_MINOR_VERSION >= 7)

namespace {

mixxx::audio::ChannelCount getChannelPerWorker() {
    RubberBandWorkerPool* pPool = RubberBandWorkerPool::instance();
    return pPool ? pPool->channelPerWorker() : mixxx::kEngineChannelCount;
}
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
    for (auto& stretcher : m_pInstances) {
        require = qMax(require, stretcher->getSamplesRequired());
    }
    return require;
}
int RubberBandWrapper::available() const {
    int available = std::numeric_limits<int>::max();
    for (auto& stretcher : m_pInstances) {
        available = qMin(available, stretcher->available());
    }
    return available == std::numeric_limits<int>::max() ? 0 : available;
}
size_t RubberBandWrapper::retrieve(float* const* output, size_t samples) const {
    if (m_pInstances.size() == 1) {
        return m_pInstances[0]->retrieve(output, samples);
    } else {
        size_t ret = 0;
        for (const auto& stretcher : m_pInstances) {
            size_t thisRet = stretcher->retrieve(output, samples);
            DEBUG_ASSERT(!ret || thisRet == ret);
            ret = qMax(thisRet, ret);
            output += getChannelPerWorker();
        }
        return ret;
    }
}
size_t RubberBandWrapper::getInputIncrement() const {
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return -1;
    }
    return m_pInstances[0]->getInputIncrement();
}
size_t RubberBandWrapper::getLatency() const {
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return -1;
    }
    return m_pInstances[0]->getLatency();
}
double RubberBandWrapper::getPitchScale() const {
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return -1;
    }
    return m_pInstances[0]->getPitchScale();
}
// See
// https://github.com/breakfastquay/rubberband/commit/72654b04ea4f0707e214377515119e933efbdd6c
// for how these two functions were implemented within librubberband itself
size_t RubberBandWrapper::getPreferredStartPad() const {
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return -1;
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
        return -1;
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
            if (!pPool->tryStart(pInstance.get())) {
                pInstance->run();
            }
            input += pPool->channelPerWorker();
        }
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

    auto channelPerWorker = getChannelPerWorker();
    qDebug() << "RubberBandWrapper::setup" << channelPerWorker;
    VERIFY_OR_DEBUG_ASSERT(0 == chCount % channelPerWorker) {
        m_pInstances.emplace_back(
                std::make_unique<RubberBandTask>(
                        sampleRate, chCount, opt));
        return;
    }

    for (int c = 0; c < chCount; c += channelPerWorker) {
        m_pInstances.emplace_back(
                std::make_unique<RubberBandTask>(
                        sampleRate, channelPerWorker, opt));
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
