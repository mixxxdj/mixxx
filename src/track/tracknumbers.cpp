#include <QStringList>

#include "track/tracknumbers.h"
#include "util/assert.h"

/*static*/ const QString TrackNumbers::kSeparator("/");

//static
std::pair<TrackNumbers, TrackNumbers::ParseResult> TrackNumbers::fromString(
        const QString& str) {
    TrackNumbers trackNumbers;
    if (str.trimmed().isEmpty()) {
        return std::make_pair(trackNumbers, ParseResult::EMPTY);
    } else {
        const QStringList splitted(str.split(kSeparator));
        DEBUG_ASSERT(splitted.size() > 0);
        ParseResult parseResult;
        if ((splitted.size() == 1) || (splitted.size() == 2)) {
            parseResult = ParseResult::VALID;
            bool currentValid = false;
            int current = splitted[0].toInt(&currentValid);
            if (currentValid) {
                trackNumbers.setCurrent(current);
            } else {
                parseResult = ParseResult::INVALID;
            }
            if (splitted.size() > 1) {
                bool totalValid = false;
                int total = splitted[1].toInt(&totalValid);
                if (totalValid) {
                    trackNumbers.setTotal(total);
                } else {
                    parseResult = ParseResult::INVALID;
                }
            }
            if (!trackNumbers.isValid()) {
                parseResult = ParseResult::INVALID;
            }
        } else {
            parseResult = ParseResult::INVALID;
        }
        return std::make_pair(trackNumbers, parseResult);
    }
}

//static
std::pair<QString, QString> TrackNumbers::splitString(
        const QString& str) {
    const int splitIndex = str.indexOf(kSeparator);
    if (0 <= splitIndex) {
        return std::make_pair(
                str.left(splitIndex),
                str.right(str.length() - (splitIndex + 1)));
    } else {
        return std::make_pair(str, QString());
    }
}

//static
QString TrackNumbers::joinStrings(
        const QString& currentText,
        const QString& totalText) {
    if (totalText.isEmpty()) {
        return currentText;
    } else {
        return currentText + kSeparator + totalText;
    }
}

QString TrackNumbers::getCurrentText() const {
    if (hasCurrent() && isCurrentValid()) {
        return QString::number(getCurrent());
    } else {
        return QString();
    }
}

QString TrackNumbers::getTotalText() const {
    if (hasTotal() && isTotalValid()) {
        return QString::number(getTotal());
    } else {
        return QString();
    }
}

std::pair<QString, QString> TrackNumbers::toSplitString() const {
    QString first(getCurrentText());
    const QString second(getTotalText());
    if (!second.isEmpty()) {
        // Padding with '0' to match the size of the total track number
        if (first.size() < second.size()) {
            const QString padding(second.size() - first.size(), '0');
            first = padding + first;
        }
    }
    return std::make_pair(first, second);
}

QString TrackNumbers::toString() const {
    const auto splitted(toSplitString());
    return joinStrings(splitted.first, splitted.second);
}
