#pragma once

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

  private:
    double m_dFrame;
};

inline const Frame operator+(const Frame& frame1, const Frame& frame2) {
    return Frame(frame1.getValue() + frame2.getValue());
}

inline const Frame operator+(const Frame& frame1, const double& dFrame2) {
    return Frame(frame1.getValue() + dFrame2);
}

inline Frame& operator+=(Frame& frame1, Frame& frame2) {
    frame1.setValue(frame1.getValue() + frame2.getValue());
    return frame1;
}

inline Frame& operator+=(Frame& frame1, double dFrame2) {
    frame1.setValue(frame1.getValue() + dFrame2);
    return frame1;
}

} // namespace mixxx
