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

double MovingMode::compute() {
    return BeatStatistics::mode(m_tempoFrequency);
}

void MovingMedian::update(double newValue, double oldValue) {
    auto insertPosition = std::lower_bound(m_sortedValues.begin(), m_sortedValues.end(), newValue);
    m_sortedValues.insert(insertPosition, newValue);
    if (!util_isnan(oldValue)) {
        m_sortedValues.removeAt(m_sortedValues.indexOf(oldValue));
    }
}
double MovingMedian::compute() {
    return BeatStatistics::median(m_sortedValues);
}

// this is an incomplete implementation that do NOT
// handle cases where the mode is not unique.
// but is good enough for our proporses
double BeatStatistics::mode(QMap<double, int> tempoFrequency) {
    QMapIterator<double, int> tempos(tempoFrequency);
    int max = 0;
    double mode = 0.0;
    while (tempos.hasNext()) {
        tempos.next();
        if (max < tempos.value()) {
            mode = tempos.key();
            max = tempos.value();
        }
    }
    return mode;
}

double BeatStatistics::median(QList<double> sortedItems) {
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

double BeatStatistics::stddev(QVector<double> const& tempos) {
    double mean = std::accumulate(tempos.begin(), tempos.end(), 0.0) / tempos.size();
    double sq_sum = std::inner_product(
            tempos.begin(),
            tempos.end(),
            tempos.begin(),
            0.0,
            [](double const& x, double const& y) { return x + y; },
            [mean](double const& x, double const& y) {
                return (x - mean) * (y - mean);
            });
    return std::sqrt(sq_sum / (tempos.size() - 1));
}
