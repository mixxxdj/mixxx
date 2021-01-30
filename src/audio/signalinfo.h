#pragma once

#include "audio/types.h"
#include "util/assert.h"
#include "util/macros.h"
#include "util/optional.h"

namespace mixxx {

namespace audio {

// Properties that characterize an uncompressed PCM audio signal.
class SignalInfo final {
    // Properties
    MIXXX_DECL_PROPERTY(ChannelCount, channelCount, ChannelCount)
    MIXXX_DECL_PROPERTY(SampleRate, sampleRate, SampleRate)
    MIXXX_DECL_PROPERTY(OptionalSampleLayout, sampleLayout, SampleLayout)

  public:
    constexpr SignalInfo() = default;
    constexpr explicit SignalInfo(
            const OptionalSampleLayout& sampleLayout)
            : m_sampleLayout(sampleLayout) {
    }
    SignalInfo(
            ChannelCount channelCount,
            SampleRate sampleRate,
            const OptionalSampleLayout& sampleLayout = std::nullopt)
            : m_channelCount(channelCount),
              m_sampleRate(sampleRate),
              m_sampleLayout(sampleLayout) {
    }
    SignalInfo(SignalInfo&&) = default;
    SignalInfo(const SignalInfo&) = default;
    /*non-virtual*/ ~SignalInfo() = default;

    constexpr bool isValid() const {
        return getChannelCount().isValid() &&
                getSampleRate().isValid();
    }

    SignalInfo& operator=(SignalInfo&&) = default;
    SignalInfo& operator=(const SignalInfo&) = default;

    // Conversion: #samples / sample offset -> #frames / frame offset
    // Only works for integer sample offsets on frame boundaries!
    SINT samples2frames(SINT samples) const {
        DEBUG_ASSERT(getChannelCount().isValid());
        DEBUG_ASSERT(0 == (samples % getChannelCount()));
        return samples / getChannelCount();
    }

    // Conversion: #frames / frame offset -> #samples / sample offset
    SINT frames2samples(SINT frames) const {
        DEBUG_ASSERT(getChannelCount().isValid());
        return frames * getChannelCount();
    }

    // Conversion: #frames / frame offset -> second offset
    double frames2secs(SINT frames) const {
        DEBUG_ASSERT(getSampleRate().isValid());
        return static_cast<double>(frames) / getSampleRate();
    }

    // Conversion: second offset -> #frames / frame offset
    double secs2frames(double seconds) const {
        DEBUG_ASSERT(getSampleRate().isValid());
        return seconds * getSampleRate();
    }

    // Conversion: #frames / frame offset -> millisecond offset
    double frames2millis(SINT frames) const {
        return frames2secs(frames) * 1000;
    }

    // Conversion: millisecond offset -> #frames / frame offset
    double millis2frames(double milliseconds) const {
        return secs2frames(milliseconds / 1000);
    }
};

bool operator==(
        const SignalInfo& lhs,
        const SignalInfo& rhs);

inline bool operator!=(
        const SignalInfo& lhs,
        const SignalInfo& rhs) {
    return !(lhs == rhs);
}

QDebug operator<<(QDebug dbg, const SignalInfo& arg);

} // namespace audio

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::audio::SignalInfo)
