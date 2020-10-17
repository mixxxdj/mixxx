#pragma once

#include <QDebug>
#include <limits>

namespace mixxx {
/// FrameDiff_t can be used to store the difference in position between
/// two frames and to store the length of a segment of track in terms of frames.
typedef double FrameDiff_t;

/// FramePos defines the position of a frame in a track
/// with respect to a fixed origin, i.e. start of the track.
class FramePos final {
  public:
    typedef double value_t;
    static constexpr value_t kStartValue = 0;
    static constexpr value_t kInvalidValue = std::numeric_limits<FramePos::value_t>::quiet_NaN();

    constexpr FramePos()
            : m_dFramePos(kStartValue) {
    }

    constexpr explicit FramePos(value_t dFramePos)
            : m_dFramePos(dFramePos) {
    }

    void setValue(value_t dFramePos) {
        m_dFramePos = dFramePos;
    }

    value_t getValue() const {
        return m_dFramePos;
    }

    FramePos& operator+=(FrameDiff_t increment) {
        m_dFramePos += increment;
        return *this;
    }

    FramePos& operator-=(FrameDiff_t decrement) {
        m_dFramePos -= decrement;
        return *this;
    }

    FramePos& operator*=(double multiple) {
        m_dFramePos *= multiple;
        return *this;
    }

    FramePos& operator/=(double divisor) {
        m_dFramePos /= divisor;
        return *this;
    }

  private:
    value_t m_dFramePos;
};

// FramePos can be added to and subtracted from a FrameDiff_t
inline FramePos operator+(FramePos framePos, FrameDiff_t frameDiff) {
    return FramePos(framePos.getValue() + frameDiff);
}

inline FramePos operator-(FramePos framePos, FrameDiff_t frameDiff) {
    return FramePos(framePos.getValue() - frameDiff);
}

inline FrameDiff_t operator-(FramePos framePos1, FramePos framePos2) {
    return framePos1.getValue() - framePos2.getValue();
}

// Adding two FramePos is not allowed since every FramePos shares a common
// reference or origin i.e. the start of the track.

// FramePos can be multiplied or divided by a double
inline FramePos operator*(FramePos framePos, double multiple) {
    return FramePos(framePos.getValue() * multiple);
}

inline FramePos operator/(FramePos framePos, double divisor) {
    return FramePos(framePos.getValue() / divisor);
}

inline bool operator<(FramePos frame1, FramePos frame2) {
    return frame1.getValue() < frame2.getValue();
}

inline bool operator<=(FramePos frame1, FramePos frame2) {
    return frame1.getValue() <= frame2.getValue();
}

inline bool operator>(FramePos frame1, FramePos frame2) {
    return frame1.getValue() > frame2.getValue();
}

inline bool operator>=(FramePos frame1, FramePos frame2) {
    return frame1.getValue() >= frame2.getValue();
}

inline bool operator==(FramePos frame1, FramePos frame2) {
    return frame1.getValue() == frame2.getValue();
}

inline bool operator!=(FramePos frame1, FramePos frame2) {
    return !(frame1.getValue() == frame2.getValue());
}

inline QDebug operator<<(QDebug dbg, FramePos arg) {
    dbg << arg.getValue();
    return dbg;
}

constexpr FramePos kInvalidFramePos = FramePos(FramePos::kInvalidValue);
constexpr FramePos kStartFramePos = FramePos(FramePos::kStartValue);
} // namespace mixxx

Q_DECLARE_TYPEINFO(mixxx::FramePos, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(mixxx::FramePos);
