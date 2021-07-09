#pragma once

#include <QDebug>
#include <cmath>
#include <limits>

#include "engine/engine.h"
#include "util/fpclassify.h"

namespace mixxx {
namespace audio {
/// FrameDiff_t can be used to store the difference in position between
/// two frames and to store the length of a segment of track in terms of frames.
typedef double FrameDiff_t;

/// FramePos defines the position of a frame in a track
/// with respect to a fixed origin, i.e. start of the track.
///
/// Note that all invalid frame positions are considered equal.
class FramePos final {
  public:
    typedef double value_t;
    static constexpr value_t kStartValue = 0;
    static constexpr value_t kInvalidValue = std::numeric_limits<FramePos::value_t>::quiet_NaN();
    static constexpr double kLegacyInvalidEnginePosition = -1.0;

    constexpr FramePos()
            : m_framePosition(kInvalidValue) {
    }

    constexpr explicit FramePos(value_t framePosition)
            : m_framePosition(framePosition) {
    }

    /// Return a `FramePos` from a given engine sample position. To catch
    /// "invalid" positions (e.g. when parsing values from control objects),
    /// use `FramePos::fromEngineSamplePosMaybeInvalid` instead.
    static constexpr FramePos fromEngineSamplePos(double engineSamplePos) {
        return FramePos(engineSamplePos / mixxx::kEngineChannelCount);
    }

    /// Return an engine sample position. The `FramePos` is expected to be
    /// valid. If invalid positions are possible (e.g. for control object
    /// values), use `FramePos::toEngineSamplePosMaybeInvalid` instead.
    double toEngineSamplePos() const {
        DEBUG_ASSERT(isValid());
        return value() * mixxx::kEngineChannelCount;
    }

    /// Return a `FramePos` from a given engine sample position. Sample
    /// positions that equal `kLegacyInvalidEnginePosition` are considered
    /// invalid and result in an invalid `FramePos` instead.
    ///
    /// In general, using this method should be avoided and is only necessary
    /// for compatiblity with our control objects and legacy parts of the code
    /// base. Using a different code path based on the output of `isValid()` is
    /// preferable.
    static constexpr FramePos fromEngineSamplePosMaybeInvalid(double engineSamplePos) {
        if (engineSamplePos == kLegacyInvalidEnginePosition) {
            return {};
        }
        return fromEngineSamplePos(engineSamplePos);
    }

    /// Return an engine sample position. If the `FramePos` is invalid,
    /// `kLegacyInvalidEnginePosition` is returned instad.
    ///
    /// In general, using this method should be avoided and is only necessary
    /// for compatiblity with our control objects and legacy parts of the code
    /// base. Using a different code path based on the output of `isValid()` is
    /// preferable.
    double toEngineSamplePosMaybeInvalid() const {
        if (!isValid()) {
            return kLegacyInvalidEnginePosition;
        }
        return toEngineSamplePos();
    }

    /// Return true if the frame position is valid. Any finite value is
    /// considered valid, i.e. any value except NaN and negative/positive
    /// infinity.
    bool isValid() const {
        return util_isfinite(m_framePosition);
    }

    void setValue(value_t framePosition) {
        m_framePosition = framePosition;
    }

    /// Return the underlying primitive value for this frame position.
    value_t value() const {
        VERIFY_OR_DEBUG_ASSERT(isValid()) {
            return FramePos::kInvalidValue;
        }
        return m_framePosition;
    }

    /// Return true if the frame position has a fractional part, i.e. if it is
    /// not located at a full frame boundary.
    bool isFractional() const {
        DEBUG_ASSERT(isValid());
        value_t integerPart;
        return std::modf(value(), &integerPart) != 0;
    }

    /// Return position rounded to the next lower full frame position, without
    /// the fractional part.
    [[nodiscard]] FramePos toLowerFrameBoundary() const {
        return FramePos(std::floor(value()));
    }

    FramePos& operator+=(FrameDiff_t increment) {
        DEBUG_ASSERT(isValid());
        m_framePosition += increment;
        return *this;
    }

    FramePos& operator-=(FrameDiff_t decrement) {
        DEBUG_ASSERT(isValid());
        m_framePosition -= decrement;
        return *this;
    }

    FramePos& operator*=(double multiple) {
        DEBUG_ASSERT(isValid());
        m_framePosition *= multiple;
        return *this;
    }

    FramePos& operator/=(double divisor) {
        DEBUG_ASSERT(isValid());
        m_framePosition /= divisor;
        return *this;
    }

  private:
    value_t m_framePosition;
};

/// FramePos can be added to a FrameDiff_t
inline FramePos operator+(FramePos framePos, FrameDiff_t frameDiff) {
    return FramePos(framePos.value() + frameDiff);
}

/// FramePos can be subtracted from a FrameDiff_t
inline FramePos operator-(FramePos framePos, FrameDiff_t frameDiff) {
    return FramePos(framePos.value() - frameDiff);
}

/// Two FramePos can be subtracted to get a FrameDiff_t
inline FrameDiff_t operator-(FramePos framePos1, FramePos framePos2) {
    return framePos1.value() - framePos2.value();
}

// Adding two FramePos is not allowed since every FramePos shares a common
// reference or origin i.e. the start of the track.

/// FramePos can be multiplied or divided by a double
inline FramePos operator*(FramePos framePos, double multiple) {
    return FramePos(framePos.value() * multiple);
}

inline FramePos operator/(FramePos framePos, double divisor) {
    return FramePos(framePos.value() / divisor);
}

inline bool operator<(FramePos frame1, FramePos frame2) {
    return frame1.value() < frame2.value();
}

inline bool operator<=(FramePos frame1, FramePos frame2) {
    return frame1.value() <= frame2.value();
}

inline bool operator>(FramePos frame1, FramePos frame2) {
    return frame1.value() > frame2.value();
}

inline bool operator>=(FramePos frame1, FramePos frame2) {
    return frame1.value() >= frame2.value();
}

inline bool operator==(FramePos frame1, FramePos frame2) {
    if (frame1.isValid() && frame2.isValid()) {
        return frame1.value() == frame2.value();
    }

    if (!frame1.isValid() && !frame2.isValid()) {
        return true;
    }

    return false;
}

inline bool operator!=(FramePos frame1, FramePos frame2) {
    return !(frame1 == frame2);
}

inline QDebug operator<<(QDebug dbg, FramePos arg) {
    if (arg.isValid()) {
        dbg.nospace() << "FramePos(" << arg.value() << ")";
    } else {
        dbg << "FramePos()";
    }
    return dbg;
}

constexpr FramePos kInvalidFramePos = FramePos(FramePos::kInvalidValue);
constexpr FramePos kStartFramePos = FramePos(FramePos::kStartValue);
} // namespace audio
} // namespace mixxx

Q_DECLARE_TYPEINFO(mixxx::audio::FramePos, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(mixxx::audio::FramePos);
