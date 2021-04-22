#include "rangelist.h"

#include <util/assert.h>

#include <QRegularExpression>
#include <algorithm>

namespace {

const QString kGroupSeparator = QStringLiteral(", ");
const QString kRangeSeparator = QStringLiteral(" - ");
// when changing groupSeparator or rangeSeparator, rangeListMatchingRegex must
// be adjusted as well.
const QRegularExpression kRangeListMatchingRegex(
        QStringLiteral("(?:(\\d+)(?:\\s*-\\s*(\\d+))?)[\\s,]*"));

} // namespace

namespace mixxx {

QList<int> parseRangeList(const QString& input) {
    QList<int> intList;
    auto matchGroups = kRangeListMatchingRegex.globalMatch(input);
    while (matchGroups.hasNext()) {
        const auto group = matchGroups.next();
        const QString rangeStart = group.captured(1);
        const QString rangeEnd = group.captured(2);
        bool startOk, endOk;
        int startIndex = rangeStart.toInt(&startOk);
        if (!startOk) {
            continue;
        }
        int endIndex = rangeEnd.toInt(&endOk);
        if (!endOk) {
            endIndex = startIndex;
        }
        for (int currentIndex = startIndex; currentIndex <= endIndex; currentIndex++) {
            intList.append(currentIndex);
        }
    }

    std::sort(intList.begin(), intList.end());
    const auto end = std::unique(intList.begin(), intList.end());
    intList.erase(end, intList.end());

    return intList;
}

QString stringifyRangeList(const QList<int>& rangeList) {
    DEBUG_ASSERT(std::is_sorted(rangeList.cbegin(), rangeList.cend()));
    DEBUG_ASSERT([&rangeList]() {
        // Immediately-invoked function expression

        // assumes list is sorted!
        auto first_duplicate = std::adjacent_find(rangeList.cbegin(), rangeList.cend());
        return first_duplicate == rangeList.cend();
    }());
    DEBUG_ASSERT(std::all_of(rangeList.cbegin(), rangeList.cend(), [](int val) {
        return val >= 0;
    }));

    QString stringifiedRangeList;

    for (int i = 0; i < rangeList.size();) {
        int rangeStartIndex = i;
        int rangeStartValue = rangeList.at(i);

        while (i < rangeList.size() && rangeList.at(i) == rangeStartValue + (i - rangeStartIndex)) {
            i++;
        }

        int rangeEndIndex = i - 1;

        stringifiedRangeList += QString::number(rangeStartValue);

        switch (rangeEndIndex - rangeStartIndex) {
        case 0:
            // not a range
            break;
        case 1:
            // treat ranges of (i..i+1) as separate groups: "i, i+1"
            stringifiedRangeList += kGroupSeparator + QString::number(rangeList.at(rangeEndIndex));
            break;
        default:
            // range where the end is >=2 than the start
            stringifiedRangeList += kRangeSeparator + QString::number(rangeList.at(rangeEndIndex));
            break;
        }

        if (i < rangeList.size()) {
            stringifiedRangeList += kGroupSeparator;
        }
    }
    return stringifiedRangeList;
}

} // namespace mixxx
