#include <QStringList>

#include "track/tracknumbers.h"
#include "util/assert.h"

/*static*/ const QString TrackNumbers::kSeparator("/");

namespace {
    bool parseValueFromString(
            const QString& str,
            int* pValue) {
        bool valid = false;
        int value = str.toInt(&valid);
        if (valid) {
            if (nullptr != pValue) {
                *pValue = value;
            }
            return TrackNumbers::isValidValue(value);
        } else {
            return false;
        }
    }
} // anonymous namespace

//static
TrackNumbers::ParseResult TrackNumbers::parseFromStrings(
        const QString& currentText,
        const QString& totalText,
        TrackNumbers* pParsed) {
    const QString currentTrimmed(currentText.trimmed());
    const QString totalTrimmed(totalText.trimmed());
    ParseResult parseResult =
            (currentTrimmed.isEmpty() && totalTrimmed.isEmpty()) ? ParseResult::EMPTY : ParseResult::VALID;
    int currentValue = kValueUndefined;
    if (!currentTrimmed.isEmpty()) {
        if (!parseValueFromString(currentTrimmed, &currentValue)) {
            parseResult = ParseResult::INVALID;
        }
    }
    if (nullptr != pParsed) {
        pParsed->setCurrent(currentValue);
    }
    int totalValue = kValueUndefined;
    if (!totalTrimmed.isEmpty()) {
        if (!parseValueFromString(totalTrimmed, &totalValue)) {
            parseResult = ParseResult::INVALID;
        }
    }
    if (nullptr != pParsed) {
        pParsed->setTotal(totalValue);
    }
    return parseResult;
}

//static
TrackNumbers::ParseResult TrackNumbers::parseFromString(
        const QString& str,
        TrackNumbers* pParsed) {
    const QStringList splitted(str.split(kSeparator));
    DEBUG_ASSERT(splitted.size() > 0);
    switch (splitted.size()) {
    case 1:
        return parseFromStrings(splitted[0], QString(), pParsed);
    case 2:
        return parseFromStrings(splitted[0], splitted[1], pParsed);
    default:
        return ParseResult::INVALID;
    }
}

//static
void TrackNumbers::splitString(
        const QString& str,
        QString* pCurrentText,
        QString* pTotalText) {
    const int splitIndex = str.indexOf(kSeparator);
    if (0 <= splitIndex) {
        if (nullptr != pCurrentText) {
            *pCurrentText = str.left(splitIndex);
        }
        if (nullptr != pTotalText) {
            *pTotalText = str.right(str.length() - (splitIndex + 1));
        }
    } else {
        if (nullptr != pCurrentText) {
            *pCurrentText = str;
        }
        if (nullptr != pTotalText) {
            pTotalText->clear();
        }
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

void TrackNumbers::toStrings(
        QString* pCurrentText,
        QString* pTotalText) const {
    QString currentText;
    if (hasCurrent() && isCurrentValid()) {
        currentText = QString::number(getCurrent());
    }
    QString totalText;
    if (hasTotal() && isTotalValid()) {
        totalText = QString::number(getTotal());
    }
    if (!totalText.isEmpty()) {
        // Padding with '0' of the current track number string
        // to match the string length of the total track number,
        // e.g. current=3 and total=12 becomes currentText="03"
        // and totalText="12".
        if (currentText.size() < totalText.size()) {
            const QString padding(totalText.size() - currentText.size(), '0');
            currentText = padding + currentText;
        }
    }
    if (nullptr != pCurrentText) {
        *pCurrentText = currentText;
    }
    if (nullptr != pTotalText) {
        *pTotalText = totalText;
    }
}

QString TrackNumbers::toString() const {
    QString currentText;
    QString totalText;
    toStrings(&currentText, &totalText);
    return joinStrings(currentText, totalText);
}
