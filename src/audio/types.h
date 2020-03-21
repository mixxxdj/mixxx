#pragma once

#include <QtDebug>

#include "util/assert.h"
#include "util/optional.h"
#include "util/types.h"

// Various properties of digital PCM audio signals and streams.
//
// An audio signal or stream contains samples for multiple
// channels sampled at discrete times.
//
// The channel layout (optional) assigns meaning to the
// different channels of a signal.
//
// The sample layout defines how subsequent samples from
// different channels are represented and stored in memory.

namespace mixxx {

namespace audio {

enum class ChannelLayout {
    Mono,     // 1 channel
    DualMono, // 2 channels with identical signals
    Stereo,   // 2 independent channels left/right
    // ...to be continued...
};

typedef std::optional<ChannelLayout> OptionalChannelLayout;

QDebug operator<<(QDebug dbg, ChannelLayout arg);

class ChannelCount {
  private:
    static constexpr SINT kValueDefault = 0;

  public:
    static constexpr SINT kValueMin = 1;   // lower bound (inclusive)
    static constexpr SINT kValueMax = 255; // upper bound (inclusive, 8-bit unsigned integer)

    static constexpr ChannelCount min() {
        return ChannelCount(kValueMin);
    }
    static constexpr ChannelCount max() {
        return ChannelCount(kValueMax);
    }

    static ChannelCount fromLayout(ChannelLayout layout) {
        switch (layout) {
        case ChannelLayout::Mono:
            return ChannelCount(1);
        case ChannelLayout::DualMono:
            return ChannelCount(1);
        case ChannelLayout::Stereo:
            return ChannelCount(2);
        }
        DEBUG_ASSERT(!"unreachable code");
    }

    explicit constexpr ChannelCount(SINT value = kValueDefault)
            : m_value(value) {
    }
    explicit ChannelCount(ChannelLayout layout)
            : m_value(fromLayout(layout).m_value) {
    }

    constexpr bool isValid() const {
        return (kValueMin <= m_value) &&
                (m_value <= kValueMax);
    }

    /*implicit*/ constexpr operator SINT() const {
        return m_value;
    }

  private:
    SINT m_value;
};

// Defines the ordering of how samples from multiple channels are
// stored in contiguous buffers:
//    - Planar: Channel by channel
//    - Interleaved: Frame by frame
// The samples from all channels that are coincident in time are
// called a "frame" (or more specific "sample frame").
//
// Example: 10 stereo samples from left (L) and right (R) channel
// Planar layout:      LLLLLLLLLLRRRRRRRRRR
// Interleaved layout: LRLRLRLRLRLRLRLRLRLR
enum class SampleLayout {
    Planar,
    Interleaved
};

typedef std::optional<SampleLayout> OptionalSampleLayout;

QDebug operator<<(QDebug dbg, SampleLayout arg);

class SampleRate {
  private:
    static constexpr SINT kValueDefault = 0;

  public:
    static constexpr SINT kValueMin = 8000;   // lower bound (inclusive, = minimum MP3 sample rate)
    static constexpr SINT kValueMax = 192000; // upper bound (inclusive)

    static constexpr SampleRate min() {
        return SampleRate(kValueMin);
    }
    static constexpr SampleRate max() {
        return SampleRate(kValueMax);
    }

    static constexpr const char* unit() {
        return "Hz";
    }

    explicit constexpr SampleRate(SINT value = kValueDefault)
            : m_value(value) {
    }

    constexpr bool isValid() const {
        return (kValueMin <= m_value) &&
                (m_value <= kValueMax);
    }

    /*implicit*/ constexpr operator SINT() const {
        return m_value;
    }

  private:
    SINT m_value;
};

QDebug operator<<(QDebug dbg, SampleRate arg);

// The bitrate is measured in kbit/s (kbps) and provides information
// about the level of compression for lossily encoded audio streams.
// It depends on the metadata and decoder if a value for the bitrate
// is available, i.e. it might be invalid if it cannot be determined.
class Bitrate {
  private:
    static constexpr SINT kValueDefault = 0;

  public:
    static constexpr const char* unit() {
        return "kbps";
    }

    explicit constexpr Bitrate(SINT value = kValueDefault)
            : m_value(value) {
    }

    constexpr bool isValid() const {
        return m_value > kValueDefault;
    }

    /*implicit*/ operator SINT() const {
        DEBUG_ASSERT(m_value >= kValueDefault); // unsigned value
        return m_value;
    }

  private:
    SINT m_value;
};

QDebug operator<<(QDebug dbg, Bitrate arg);

} // namespace audio

} // namespace mixxx

Q_DECLARE_TYPEINFO(mixxx::audio::ChannelCount, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(mixxx::audio::ChannelCount)

Q_DECLARE_TYPEINFO(mixxx::audio::OptionalChannelLayout, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(mixxx::audio::OptionalChannelLayout)

Q_DECLARE_TYPEINFO(mixxx::audio::OptionalSampleLayout, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(mixxx::audio::OptionalSampleLayout)

Q_DECLARE_TYPEINFO(mixxx::audio::SampleRate, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(mixxx::audio::SampleRate)

Q_DECLARE_TYPEINFO(mixxx::audio::Bitrate, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(mixxx::audio::Bitrate)
