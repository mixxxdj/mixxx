#include <QStringList>

#include "track/tracknumbers.h"
#include "util/assert.h"

/*static*/ const QString TrackNumbers::kDefaultSeparator("/");

//static
std::pair<TrackNumbers, TrackNumbers::ParseResult> TrackNumbers::fromString(
        const QString& str,
        const QString& separator) {
    TrackNumbers trackNumbers;
    if (str.trimmed().isEmpty()) {
        return std::make_pair(trackNumbers, ParseResult::EMPTY);
    } else {
        const QStringList splitted(str.split(separator));
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

QString TrackNumbers::toString(
        const QString& separator) const {
    QString result(getCurrentText());
    const QString totalText(getTotalText());
    if (!totalText.isEmpty()) {
        // Padding with '0' to match the size of the total track number
        if (result.size() < totalText.size()) {
            const QString padding(totalText.size() - result.size(), '0');
            result = padding + result;
        }
        DEBUG_ASSERT(!result.isEmpty());
        result += separator;
        result += totalText;
    }
    return result;
}
