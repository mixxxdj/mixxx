#ifndef PERFORMANCETIMER_H
#define PERFORMANCETIMER_H

#include <QtGlobal>

class QTime;

class PerformanceTimer {
  public:
    PerformanceTimer();
    virtual ~PerformanceTimer();

    void start();
    quint64 restart();
    quint64 elapsed();

  private:
    quint64 count() const;
    quint64 countDeltaToNanoseconds(quint64 delta) const;

    bool m_running;
    quint64 m_start;
    quint64 m_freq_numerator;
    quint64 m_freq_denominator;

    // This is the crappy, millisecond-precision fallback if we don't have a
    // high-resolution timer for the platform.
    QTime* m_time;
};

#endif /* PERFORMANCETIMER_H */
