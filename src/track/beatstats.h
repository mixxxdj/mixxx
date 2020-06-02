#pragma once

#include <QQueue>

// These classes are used to compute statistics descriptors
// of a series of tempo values and are called from beatutils

class WindowedStatistics {

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

  public:

    WindowedStatistics(int period) {
        assert(period > 0);
        m_period = period;
    }
    virtual ~WindowedStatistics(){};

    double operator()(double newValue) {
        double oldValue = updateWindow(newValue);
        update(newValue, oldValue);
        return compute();
    }
    double operator()(void) {
        return compute();
    }
    void operator++(int) {
        m_period += 1;
    }
};
class BeatStatistics {
  public:
    static double computeSampleMedian(QList<double> sortedItems);
};

class MovingMode : public WindowedStatistics {
    QMap<double, int> m_tempoFrequency;
    void update(double, double) override;
    double compute() override;
    public:
      MovingMode(int period)
              : WindowedStatistics(period){};
      ~MovingMode(){};
};

class MovingMedian : public WindowedStatistics {
    QList<double> m_sortedValues;
    void update(double, double) override;
    double compute() override;
    public:
      MovingMedian(int period)
              : WindowedStatistics(period){};
      ~MovingMedian(){};
};