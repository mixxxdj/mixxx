#pragma once

#include <QQueue>
#include <cmath>

#include "util/assert.h"

// These classes are used to compute statistics descriptors
// of a series of tempo values and are called from analyzerrhythm

class WindowedStatistics {
  public:
    WindowedStatistics(int period) {
        DEBUG_ASSERT(period > 0);
        m_period = period;
    }
    virtual ~WindowedStatistics() = default;

    double operator()(double newValue) {
        double oldValue = updateWindow(newValue);
        update(newValue, oldValue);
        return compute();
    }
    double operator()(void) {
        return compute();
    }
    int lag() {
        // expected latency
        return (m_window.size() - 1) / 2;
    }

  private:
    QQueue<double> m_window;
    int m_period;

    double updateWindow(double newValue) {
        m_window.enqueue(newValue);
        if (m_window.count() > m_period) {
            return m_window.dequeue();
        } else {
            return std::nan("");
        }
    }
    virtual double compute() = 0;
    virtual void update(double newValue, double oldValue) = 0;
};
class BeatStatistics {
  public:
    static double median(QList<double> sortedItems);
    static double mode(QMap<double, int> tempoFrequency);
    static double stddev(QVector<double> const& tempos);
};

class MovingMode : public WindowedStatistics {
    QMap<double, int> m_tempoFrequency;
    void update(double, double) override;
    double compute() override;

  public:
    MovingMode(int period)
            : WindowedStatistics(period){};
    ~MovingMode() = default;
};

class MovingMedian : public WindowedStatistics {
    QList<double> m_sortedValues;
    void update(double, double) override;
    double compute() override;

  public:
    MovingMedian(int period)
            : WindowedStatistics(period){};
    ~MovingMedian() = default;
};
