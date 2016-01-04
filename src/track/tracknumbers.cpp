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
        const QString& actualText,
        const QString& totalText,
        TrackNumbers* pParsed) {
    const QString actualTrimmed(actualText.trimmed());
    const QString totalTrimmed(totalText.trimmed());
    ParseResult parseResult =
            (actualTrimmed.isEmpty() && totalTrimmed.isEmpty()) ? ParseResult::EMPTY : ParseResult::VALID;
    int actualValue = kValueUndefined;
    if (!actualTrimmed.isEmpty()) {
        if (!parseValueFromString(actualTrimmed, &actualValue)) {
            parseResult = ParseResult::INVALID;
        }
    }
    if (nullptr != pParsed) {
        pParsed->setActual(actualValue);
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
    QString actualText;
    QString totalText;
    TrackNumbers::splitString(str, &actualText, &totalText);
    return parseFromStrings(actualText, totalText, pParsed);
}

//static
void TrackNumbers::splitString(
        const QString& str,
        QString* pActualText,
        QString* pTotalText) {
    const int splitIndex = str.indexOf(kSeparator);
    if (0 <= splitIndex) {
        if (nullptr != pActualText) {
            *pActualText = str.left(splitIndex);
        }
        if (nullptr != pTotalText) {
            *pTotalText = str.right(str.length() - (splitIndex + 1));
        }
    } else {
        if (nullptr != pActualText) {
            *pActualText = str;
        }
        if (nullptr != pTotalText) {
            pTotalText->clear();
        }
    }
}

//static
QString TrackNumbers::joinStrings(
        const QString& actualText,
        const QString& totalText) {
    if (totalText.isEmpty()) {
        return actualText;
    } else {
        return actualText + kSeparator + totalText;
    }
}

void TrackNumbers::toStrings(
        QString* pActualText,
        QString* pTotalText) const {
    QString actualText;
    if (hasActual() && isActualValid()) {
        actualText = QString::number(getActual());
    }
    QString totalText;
    if (hasTotal() && isTotalValid()) {
        totalText = QString::number(getTotal());
    }
    if (!totalText.isEmpty()) {
        // Padding with '0' of the actual track number string
        // to match the string length of the total track number,
        // e.g. actual=3 and total=12 becomes actualText="03"
        // and totalText="12".
        if (actualText.size() < totalText.size()) {
            const QString padding(totalText.size() - actualText.size(), '0');
            actualText = padding + actualText;
        }
    }
    if (nullptr != pActualText) {
        *pActualText = actualText;
    }
    if (nullptr != pTotalText) {
        *pTotalText = totalText;
    }
}

QString TrackNumbers::toString() const {
    QString actualText;
    QString totalText;
    toStrings(&actualText, &totalText);
    return joinStrings(actualText, totalText);
}
