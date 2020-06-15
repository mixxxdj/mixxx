#pragma once

class QDebug;

namespace mixxx {
class Frame {
  public:
    Frame()
            : m_dFrame(0) {
    }

    explicit Frame(double dFrame)
            : m_dFrame(dFrame) {
    }

    ~Frame() = default;

    void setValue(double dFrame) {
        m_dFrame = dFrame;
    }

    double getValue() const {
        return m_dFrame;
    }

    Frame& operator+=(const Frame& arg) {
        m_dFrame += arg.getValue();
        return *this;
    }

    Frame& operator-=(const Frame& arg) {
        m_dFrame -= arg.getValue();
        return *this;
    }

  private:
    double m_dFrame;
};

inline const Frame operator+(const Frame& frame1, const Frame& frame2) {
    return Frame(frame1.getValue() + frame2.getValue());
}

inline const Frame operator-(const Frame& frame1, const Frame& frame2) {
    return Frame(frame1.getValue() - frame2.getValue());
}

inline bool operator<(const Frame& frame1, const Frame& frame2) {
    return frame1.getValue() < frame2.getValue();
}

inline bool operator<=(const Frame& frame1, const Frame& frame2) {
    return frame1.getValue() <= frame2.getValue();
}

inline bool operator>(const Frame& frame1, const Frame& frame2) {
    return frame1.getValue() > frame2.getValue();
}

inline bool operator>=(const Frame& frame1, const Frame& frame2) {
    return frame1.getValue() >= frame2.getValue();
}

inline bool operator==(const Frame& frame1, const Frame& frame2) {
    return frame1.getValue() == frame2.getValue();
}

inline bool operator!=(const Frame& frame1, const Frame& frame2) {
    return frame1.getValue() != frame2.getValue();
}

inline QDebug operator<<(QDebug dbg, const Frame& arg) {
    dbg << arg.getValue();
    return dbg;
}
} // namespace mixxx
