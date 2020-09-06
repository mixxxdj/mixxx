#include "util/windowedstatistics.h"

#include <algorithm>

#include "util/descriptivestatistics.h"

void MovingMode::update(double newValue, double oldValue) {
    // Our update window method returns a nan
    // case the window is not filled yet
    if (!isnan(oldValue)) {
        countValue(oldValue, -1);
    }
    countValue(newValue, +1);
}

void MovingMode::countValue(double value, int count) {
    int stepNumber = static_cast<int>(std::round(value / m_stepSize));
    auto it = m_frequencyOfValues.find(stepNumber);
    if (it != m_frequencyOfValues.end()) {
        it.value() += count;
    } else {
        m_frequencyOfValues.insert(stepNumber, count);
    }
}

// this is an incomplete implementation that do NOT
// handle cases where the mode is not unique.
double MovingMode::compute() {
    return DescriptiveStatistics::mode(m_frequencyOfValues);
}

void MovingMedian::update(double newValue, double oldValue) {
    // Our update window method returns a nan
    // case the window is not filled yet
    if (!isnan(oldValue)) {
        m_sortedValues.removeAt(m_sortedValues.indexOf(oldValue));
    }
    auto insertPosition = std::lower_bound(m_sortedValues.begin(), m_sortedValues.end(), newValue);
    m_sortedValues.insert(insertPosition, newValue);
}

double MovingMedian::compute() {
    return DescriptiveStatistics::median(m_sortedValues);
}
