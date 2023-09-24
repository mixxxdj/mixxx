#include <QList>
#include <QString>

namespace mixxx {

/// parses a comma-separated list of positive ints and range if ints (eg `n - m`)
/// and returns a sorted list of all the ints described.
/// inverse counterpart of `stringifyRangeList`
QList<int> parseRangeList(const QString& input);

/// take a list of positive integers and stringify them into a neat
/// user friendly representation (eg {1, 2, 3} => "1 - 3").
/// inverse counterpart of `parseRangeList`.
/// assumes rangeList is well-formed
/// (sorted, not containing duplicates, and positive integers only)
QString stringifyRangeList(const QList<int>& rangeList);

} // namespace mixxx
