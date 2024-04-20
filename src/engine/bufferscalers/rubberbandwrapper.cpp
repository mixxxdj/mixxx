#include "engine/bufferscalers/rubberbandwrapper.h"

#include "util/assert.h"

using RubberBand::RubberBandStretcher;

#define RUBBERBANDV3 (RUBBERBAND_API_MAJOR_VERSION >= 2 && RUBBERBAND_API_MINOR_VERSION >= 7)

int RubberBandWrapper::getEngineVersion() const {
#if RUBBERBANDV3
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return -1;
    }
    return m_pInstance->getEngineVersion();
#else
    return 2;
#endif
}
void RubberBandWrapper::setTimeRatio(double ratio) {
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return;
    }
    m_pInstance->setTimeRatio(ratio);
}
size_t RubberBandWrapper::getSamplesRequired() const {
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return -1;
    }
    return m_pInstance->getSamplesRequired();
}
int RubberBandWrapper::available() const {
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return -1;
    }
    return m_pInstance->available();
}
size_t RubberBandWrapper::retrieve(float* const* output, size_t samples) const {
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return -1;
    }
    return m_pInstance->retrieve(output, samples);
}
size_t RubberBandWrapper::getInputIncrement() const {
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return -1;
    }
    return m_pInstance->getInputIncrement();
}
size_t RubberBandWrapper::getLatency() const {
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return -1;
    }
    return m_pInstance->getLatency();
}
double RubberBandWrapper::getPitchScale() const {
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return -1;
    }
    return m_pInstance->getPitchScale();
}
// See
// https://github.com/breakfastquay/rubberband/commit/72654b04ea4f0707e214377515119e933efbdd6c
// for how these two functions were implemented within librubberband itself
size_t RubberBandWrapper::getPreferredStartPad() const {
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return -1;
    }
#if RUBBERBANDV3
    return m_pInstance->getPreferredStartPad();
#else
    // `getPreferredStartPad()` returns `window_size / 2`, while with
    // `getLatency()` both time stretching engines return `window_size / 2 /
    // pitch_scale`
    return static_cast<size_t>(std::ceil(
            m_pInstance->getLatency() * m_pInstance->getPitchScale()));
#endif
}
size_t RubberBandWrapper::getStartDelay() const {
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return -1;
    }
#if RUBBERBANDV3
    return m_pInstance->getStartDelay();
#else
    // In newer Rubber Band versions `getLatency()` is a deprecated alias for
    // `getStartDelay()`, so they should behave the same. In the commit linked
    // above the behavior was different for the R3 stretcher, but that was only
    // during the initial betas of Rubberband 3.0 so we shouldn't have to worry
    // about that.
    return m_pInstance->getLatency();
#endif
}
void RubberBandWrapper::process(const float* const* input, size_t samples, bool final) {
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return;
    }
    return m_pInstance->process(input, samples, final);
}
void RubberBandWrapper::reset() {
    m_pInstance->reset();
}
void RubberBandWrapper::clear() {
    m_pInstance.reset();
}
void RubberBandWrapper::setup(mixxx::audio::SampleRate sampleRate,
        mixxx::audio::ChannelCount chCount,
        const RubberBandStretcher::Options& opt) {
    // The instance should have been cleared, or not set before
    VERIFY_OR_DEBUG_ASSERT(!m_pInstance) {
        m_pInstance.reset();
    }

    m_pInstance = std::make_unique<RubberBand::RubberBandStretcher>(
            sampleRate, chCount, opt);
}
void RubberBandWrapper::setPitchScale(double scale) {
    m_pInstance->setPitchScale(scale);
}

bool RubberBandWrapper::isValid() const {
    return !!m_pInstance;
}
