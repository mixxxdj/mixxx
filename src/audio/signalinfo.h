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
    PROPERTY_SET_BYVAL_GET_BYREF(ChannelCount, channelCount, ChannelCount)
    PROPERTY_SET_BYVAL_GET_BYREF(SampleRate, sampleRate, SampleRate)
    PROPERTY_SET_BYVAL_GET_BYREF(OptionalSampleLayout, sampleLayout, SampleLayout)

  public:
    constexpr SignalInfo() = default;
    constexpr explicit SignalInfo(
            OptionalSampleLayout sampleLayout)
            : m_sampleLayout(sampleLayout) {
    }
    SignalInfo(
            ChannelCount channelCount,
            SampleRate sampleRate,
            OptionalSampleLayout sampleLayout = std::nullopt)
            : m_channelCount(channelCount),
              m_sampleRate(sampleRate),
              m_sampleLayout(sampleLayout) {
    }
    SignalInfo(SignalInfo&&) = default;
    SignalInfo(const SignalInfo&) = default;
    /*non-virtual*/ ~SignalInfo() = default;

    constexpr bool isValid() const {
        return getChannelCount().isValid() &&
                getSampleLayout() &&
                getSampleRate().isValid();
    }

    SignalInfo& operator=(SignalInfo&&) = default;
    SignalInfo& operator=(const SignalInfo&) = default;

    // Conversion: #samples / sample offset -> #frames / frame offset
    template<typename T>
    inline T samples2frames(T samples) const {
        DEBUG_ASSERT(getChannelCount().isValid());
        DEBUG_ASSERT(0 == (samples % getChannelCount()));
        return samples / getChannelCount();
    }

    // Conversion: #frames / frame offset -> #samples / sample offset
    template<typename T>
    inline T frames2samples(T frames) const {
        DEBUG_ASSERT(getChannelCount().isValid());
        return frames * getChannelCount();
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
