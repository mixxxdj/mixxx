#pragma once

template <typename T>
class RampingValue {
  public:
    RampingValue(const T& initial, const T& final, int steps) {
        m_value = initial;
        m_increment = (final - initial) / steps;
    }

    T getNext() {
        return m_value += m_increment;
    }

  private:
    T m_value;
    T m_increment;
};
