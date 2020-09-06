#include <QHash>
#include <QList>

class DescriptiveStatistics {
  public:
    // The median is the middle value of a sorted sequence
    // If sequence is even the mean of both middle values.
    static double median(const QList<double>& sortedItems);
    // The mode is most repeated value in a sequence
    static double mode(const QHash<int, int>& frequencyOfValues);
};
