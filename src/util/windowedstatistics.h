#pragma once

#include <QHash>
#include <QList>
#include <cmath>
#include <memory>

#include "util/assert.h"
#include "util/circularbuffer.h"
#include "util/fpclassify.h"

/// These classes are used to compute statistics descriptors
/// of a series of tempo values and are called from beatutils
class WindowedStatistics {
  public:
    WindowedStatistics(int windowSize) {
        DEBUG_ASSERT(windowSize > 0);
        m_windowSize = windowSize;
        m_window = std::make_unique<CircularBuffer<double>>(windowSize);
        m_currentCount = 0;
    }
    virtual ~WindowedStatistics() = default;

    double pushAndEvaluate(double newValue) {
        if (isnan(newValue)) {
            return newValue;
        }
        double oldValue = updateWindow(newValue);
        update(newValue, oldValue);
        return compute();
    }

    void clear() {
        m_window->clear();
    }

    int lag() {
        // expected latency
        return (m_currentCount - 1) / 2;
    }

    int windowCurrentSize() {
        return m_currentCount;
    }

  private:
    double updateWindow(double newValue) {
        // case not full we do not want to remove the
        // first element on child class update method
        double front = std::nan("");
        if (m_window->isFull()) {
            m_window->read(&front, 1);
        } else {
            m_currentCount += 1;
        }
        m_window->write(&newValue, 1);
        return front;
    }

    virtual double compute() = 0;
    virtual void update(double newValue, double oldValue) = 0;

    std::unique_ptr<CircularBuffer<double>> m_window;
    int m_windowSize;
    int m_currentCount;
};

class MovingMedian : public WindowedStatistics {
  public:
    MovingMedian(int windowSize)
            : WindowedStatistics(windowSize) {
        m_sortedValues.reserve(windowSize);
    };
    ~MovingMedian() = default;

  private:
    
    void update(double, double) override;
    double compute() override;

    QList<double> m_sortedValues;
};

class MovingMode : public WindowedStatistics {
  public:
    MovingMode(int windowSize)
            : WindowedStatistics(windowSize) {
        m_frequencyOfValues.reserve(windowSize);
    };
    ~MovingMode() = default;

  private:

    void update(double, double) override;
    double compute() override;

    QHash<int, int> m_frequencyOfValues;
};
