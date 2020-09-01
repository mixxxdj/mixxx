#pragma once

#include "audio/signalinfo.h"
#include "util/duration.h"

namespace mixxx {

namespace audio {

// Properties that characterize a (compressed) PCM audio stream.
//
// Currently we assume that every stream has a finite duration
// that is known upfront!
class StreamInfo final {
    // Properties
    PROPERTY_SET_BYVAL_GET_BYREF(SignalInfo, signalInfo, SignalInfo)
    PROPERTY_SET_BYVAL_GET_BYREF(Bitrate, bitrate, Bitrate)
    PROPERTY_SET_BYVAL_GET_BYREF(Duration, duration, Duration)

  public:
    constexpr StreamInfo() = default;
    constexpr explicit StreamInfo(
            const SignalInfo& signalInfo)
            : m_signalInfo(signalInfo) {
    }
    constexpr StreamInfo(
            const SignalInfo& signalInfo,
            Bitrate bitrate,
            Duration duration)
            : m_signalInfo(signalInfo),
              m_bitrate(bitrate),
              m_duration(duration) {
    }
    StreamInfo(StreamInfo&&) = default;
    StreamInfo(const StreamInfo&) = default;
    /*non-virtual*/ ~StreamInfo() = default;

    constexpr bool isValid() const {
        return getSignalInfo().isValid() &&
                getBitrate().isValid() &&
                (getDuration() > Duration::empty());
    }

    StreamInfo& operator=(StreamInfo&&) = default;
    StreamInfo& operator=(const StreamInfo&) = default;
};

bool operator==(
        const StreamInfo& lhs,
        const StreamInfo& rhs);

inline bool operator!=(
        const StreamInfo& lhs,
        const StreamInfo& rhs) {
    return !(lhs == rhs);
}

QDebug operator<<(QDebug dbg, const StreamInfo& arg);

} // namespace audio

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::audio::StreamInfo)
