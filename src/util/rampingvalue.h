#pragma once

template <typename T>
class RampingValue {
  public:
    constexpr RampingValue(const T& initial, const T& final, int steps)
            : m_start(initial),
              m_increment((final - initial) / steps) {
    }
    /// this method is supposed to be used in hot audio-processing loops
    /// which benefit greatly from vectorization. For this to work, loop-
    /// iterations can't have any data-dependencies on each other. If `getNth`
    /// were to modify its instance in between iterations, a data-dependency
    /// would be introduced, and vectorization made impossible!
    [[nodiscard]] constexpr T getNth(int step) const {
        return m_start + m_increment * step;
    }

  private:
    T m_start;
    T m_increment;
};
