#include "util/windowedstatistics.h"

#include <algorithm>

#include "util/descriptivestatistics.h"

void MovingMode::update(double newValue, double oldValue) {
    // Our update window method returns a nan
    // case the window is not filled yet
    if (!isnan(oldValue)) {
        m_frequencyOfValues[oldValue] -= 1;
    }
    m_frequencyOfValues[newValue] += 1;
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
