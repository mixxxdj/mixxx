#ifndef PERFORMANCETIMER_H
#define PERFORMANCETIMER_H

#include <QtGlobal>

class AbstractPerformanceTimer {
    virtual void start() = 0;
    virtual quint64 elapsed() = 0;
    virtual quint64 restart() = 0;
};

#ifdef __APPLE__
class PerformanceTimer : public AbstractPerformanceTimer {
  public:
    PerformanceTimer();
    virtual ~PerformanceTimer();

    void start();
    quint64 restart();
    quint64 elapsed();

  private:
    bool m_running;
    quint64 m_start;
};
#else
#include <QTime>

// This is the crappy, millisecond-precision fallback if we don't have a
// high-resolution timer for the platform.
class PerformanceTimer : public AbstractPerformanceTimer {
  public:
    PerformanceTimer() {}
    virtual ~PerformanceTimer() {}

    void start() {
        m_time.start();
    }
    quint64 restart() {
        // QTime is millisecond precision.
        return m_time.restart() * 1e6;
    }
    quint64 elapsed() {
        return m_time.elapsed() * 1e6;
    }
  private:
    QTime m_time;
};
#endif


#endif /* PERFORMANCETIMER_H */
