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

  public:
    constexpr SignalInfo() = default;
    SignalInfo(
            ChannelCount channelCount,
            SampleRate sampleRate)
            : m_channelCount(channelCount),
              m_sampleRate(sampleRate) {
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

    // adjusted for PreMix + stems
    // SINT samples2frames(SINT samples) const {
    //    DEBUG_ASSERT(getChannelCount().isValid());
    //    DEBUG_ASSERT(0 == (samples % getChannelCount()));
    //    return samples / getChannelCount();
    //}
    SINT samples2frames(SINT samples) const {
        DEBUG_ASSERT(getChannelCount().isValid());
        // now 10
        const SINT channels = getChannelCount().value();
        // trim to multiple
        samples -= samples % channels;
        return samples / channels;
    }

    // Conversion: #samples / sample offset -> #frames / frame offset
    double samples2framesFractional(double samples) const {
        DEBUG_ASSERT(getChannelCount().isValid());
        return samples / getChannelCount();
    }

    // Conversion: #frames / frame offset -> #samples / sample offset
    SINT frames2samples(SINT frames) const {
        DEBUG_ASSERT(getChannelCount().isValid());
        return frames * getChannelCount();
    }

    // Conversion: #frames / frame offset -> second offset
    double frames2secsFractional(double frames) const {
        DEBUG_ASSERT(getSampleRate().isValid());
        return frames / getSampleRate();
    }

    // Conversion: #frames / frame offset -> second offset
    double frames2secs(SINT frames) const {
        return frames2secsFractional(static_cast<double>(frames));
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
