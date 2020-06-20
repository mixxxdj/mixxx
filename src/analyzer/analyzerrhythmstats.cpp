#include "analyzer/analyzerrhythmstats.h"

#include <QList>
#include <QMap>
#include <algorithm>

#include "util/fpclassify.h"

void MovingMode::update(double newValue, double oldValue) {
    m_tempoFrequency[newValue] += 1;
    if (!util_isnan(oldValue)) {
        m_tempoFrequency[oldValue] -= 1;
    }
}
// this is an incomplete implementation that do NOT
// handle cases where the mode is not unique.
double MovingMode::compute() {
    QMapIterator<double, int> tempo(m_tempoFrequency);
    int max = 0;
    double mode = 0.0;
    while (tempo.hasNext()) {
        tempo.next();
        if (max < tempo.value()) {
            mode = tempo.key();
            max = tempo.value();
        }
    }
    return mode;
}

void MovingMedian::update(double newValue, double oldValue) {
    auto insertPosition = std::lower_bound(m_sortedValues.begin(), m_sortedValues.end(), newValue);
    m_sortedValues.insert(insertPosition, newValue);
    if (!util_isnan(oldValue)) {
        m_sortedValues.removeAt(m_sortedValues.indexOf(oldValue));
    }
}
double MovingMedian::compute() {
    return BeatStatistics::computeSampleMedian(m_sortedValues);
}

double BeatStatistics::computeSampleMedian(QList<double> sortedItems) {
    if (sortedItems.empty()) {
        return 0.0;
    }
    // When there are an even number of elements, the sample median is the mean
    // of the middle 2 elements.
    if (sortedItems.size() % 2 == 0) {
        int item_position = sortedItems.size() / 2;
        double item_value1 = sortedItems.at(item_position - 1);
        double item_value2 = sortedItems.at(item_position);
        return (item_value1 + item_value2) / 2.0;
    }
    // When there are an odd number of elements, find the {(n+1)/2}th item in
    // the sorted list.
    int item_position = (sortedItems.size() + 1) / 2;
    return sortedItems.at(item_position - 1);
}
