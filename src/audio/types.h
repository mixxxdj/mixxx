#pragma once

#include <QtDebug>
#include <cstdint>
#include <limits>

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
  public:
    typedef uint8_t value_t;

  private:
    // The default value is invalid and indicates a missing or unknown value.
    static constexpr value_t kValueDefault = 0;
    static constexpr value_t kValueMin = 1;   // lower bound (inclusive)

    static value_t valueFromInt(int value) {
        VERIFY_OR_DEBUG_ASSERT(value >= std::numeric_limits<value_t>::min() &&
                value <= std::numeric_limits<value_t>::max()) {
            return kValueDefault;
        }
        return static_cast<value_t>(value);
    }

    static value_t valueFromLayout(ChannelLayout layout) {
        switch (layout) {
        case ChannelLayout::Mono:
            return 1;
        case ChannelLayout::DualMono:
            return 2;
        case ChannelLayout::Stereo:
            return 2;
        }
        DEBUG_ASSERT(!"unreachable code");
        return kValueDefault;
    }

  public:
    static constexpr ChannelCount min() {
        return ChannelCount(kValueMin);
    }
    static constexpr ChannelCount max() {
        return ChannelCount(std::numeric_limits<value_t>::max());
    }

    static ChannelCount fromLayout(ChannelLayout layout) {
        return ChannelCount(valueFromLayout(layout));
    }

    static ChannelCount fromInt(int value) {
        return ChannelCount(valueFromInt(value));
    }

    static ChannelCount fromDouble(double value) {
        const auto channelCount = ChannelCount(static_cast<value_t>(value));
        // The channel count should always be an integer value
        // and this conversion is supposed to be lossless.
        DEBUG_ASSERT(channelCount.toDouble() == value);
        return channelCount;
    }

    static constexpr ChannelCount mono() {
        return ChannelCount(static_cast<value_t>(1));
    }

    static constexpr ChannelCount stereo() {
        return ChannelCount(static_cast<value_t>(2));
    }

    static constexpr ChannelCount stem() {
        // return ChannelCount(static_cast<value_t>(8)); // 4 stereo channels
        return ChannelCount(static_cast<value_t>(10)); // 5 stereo channels including premix
    }

    explicit constexpr ChannelCount(
            value_t value = kValueDefault)
            : m_value(value) {
    }

    // A limits checking c-tor from int channel used in many
    // external libraries
    explicit ChannelCount(int value)
            : m_value(valueFromInt(value)) {
    }

    explicit ChannelCount(
            ChannelLayout layout)
            : m_value(valueFromLayout(layout)) {
    }

    constexpr bool isValid() const {
        return kValueMin <= m_value;
    }

    constexpr value_t value() const {
        return m_value;
    }
    /*implicit*/ constexpr operator value_t() const {
        return value();
    }

    // Helper cast for COs
    constexpr double toDouble() const {
        return static_cast<double>(value());
    }

  private:
    value_t m_value;
};

class SampleRate {
  public:
    typedef uint32_t value_t;

  private:
    // The default value is invalid and indicates a missing or unknown value.
    static constexpr value_t kValueDefault = 0;

  public:
    static constexpr value_t kValueMin = 8000;   // lower bound (inclusive, = minimum MP3 sample rate)
    static constexpr value_t kValueMax = 192000; // upper bound (inclusive)

    static constexpr SampleRate min() {
        return SampleRate(kValueMin);
    }
    static constexpr SampleRate max() {
        return SampleRate(kValueMax);
    }

    static constexpr const char* unit() {
        return "Hz";
    }

    explicit constexpr SampleRate(
            value_t value = kValueDefault)
            : m_value(value) {
    }

    constexpr bool isValid() const {
        return kValueMin <= m_value && m_value <= kValueMax;
    }

    void operator=(const value_t& value) {
        m_value = value;
    }

    constexpr value_t value() const {
        return m_value;
    }
    /*implicit*/ constexpr operator value_t() const {
        return value();
    }

    static SampleRate fromDouble(double value) {
        const auto sampleRate = SampleRate(static_cast<value_t>(value));
        // The sample rate should always be an integer value
        // and this conversion is supposed to be lossless.
        DEBUG_ASSERT(sampleRate.toDouble() == value);
        return sampleRate;
    }

    constexpr double toDouble() const {
        return static_cast<double>(value());
    }

  private:
    value_t m_value;
};

QDebug operator<<(QDebug dbg, SampleRate arg);

/// Division of a SampleRate by another SampleRate returns a ratio as double.
inline double operator/(SampleRate sampleRate1, SampleRate sampleRate2) {
    return sampleRate1.toDouble() / sampleRate2.toDouble();
}

// The bitrate is measured in kbit/s (kbps) and provides information
// about the level of compression for lossily encoded audio streams.
//
// The value can only be interpreted in the context of the corresponding
// codec. It is supposed to reflect the average bitrate in case of a
// variable bitrate encoding and serves as a rough estimate of the
// expected quality.
class Bitrate {
  public:
    typedef uint32_t value_t;

  private:
    // The default value is invalid and indicates a missing or unknown value.
    static constexpr value_t kValueDefault = 0;

  public:
    static constexpr const char* unit() {
        return "kbit/s";
    }

    explicit constexpr Bitrate(
            value_t value = kValueDefault)
            : m_value(value) {
    }

    constexpr bool isValid() const {
        return m_value > kValueDefault;
    }

    constexpr value_t value() const {
        return m_value;
    }
    /*implicit*/ constexpr operator value_t() const {
        return value();
    }

  private:
    value_t m_value;
};

QDebug operator<<(QDebug dbg, Bitrate arg);

} // namespace audio

} // namespace mixxx

Q_DECLARE_TYPEINFO(mixxx::audio::ChannelCount, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(mixxx::audio::ChannelCount)

Q_DECLARE_TYPEINFO(mixxx::audio::ChannelLayout, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(mixxx::audio::ChannelLayout)

Q_DECLARE_TYPEINFO(mixxx::audio::OptionalChannelLayout, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(mixxx::audio::OptionalChannelLayout)

Q_DECLARE_TYPEINFO(mixxx::audio::SampleRate, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(mixxx::audio::SampleRate)

Q_DECLARE_TYPEINFO(mixxx::audio::Bitrate, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(mixxx::audio::Bitrate)
