#include "util/descriptivestatistics.h"

double DescriptiveStatistics::median(const QList<double>& sortedItems) {
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

// this is an incomplete implementation that do NOT
// handle cases where the mode is not unique.
// but is good enough for our proporses
double DescriptiveStatistics::mode(const QHash<double, int>& frequencyOfValues) {
    QHashIterator<double, int> valuesAndFrequency(frequencyOfValues);
    int max = 0;
    double mode = 0;
    while (valuesAndFrequency.hasNext()) {
        valuesAndFrequency.next();
        if (max < valuesAndFrequency.value()) {
            mode = valuesAndFrequency.key();
            max = valuesAndFrequency.value();
        }
    }
    return mode;
}
