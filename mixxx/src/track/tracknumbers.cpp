#include <QStringList>

#include "track/tracknumbers.h"
#include "util/assert.h"

/*static*/ constexpr int TrackNumbers::kValueUndefined;
/*static*/ constexpr int TrackNumbers::kValueMin;

/*static*/ const QString TrackNumbers::kSeparator("/");

//static
bool TrackNumbers::parseValueFromString(
        const QString& str,
        int* pValue) {
    bool valid = false;
    int value = str.toInt(&valid);
    if (valid) {
        if (nullptr != pValue) {
            *pValue = value;
        }
    }
    return valid;
}

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
        if (!parseValueFromString(actualTrimmed, &actualValue) ||
                !isValidValue(actualValue)) {
            parseResult = ParseResult::INVALID;
        }
    }
    if (nullptr != pParsed) {
        pParsed->setActual(actualValue);
    }
    int totalValue = kValueUndefined;
    if (!totalTrimmed.isEmpty()) {
        if (!parseValueFromString(totalTrimmed, &totalValue) ||
                !isValidValue(totalValue)) {
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
    // NOTE(uklotzde): The input string must be passed by value and
    // not by const-ref! Otherwise pointer aliasing may occur if one
    // of the output parameters points to the same string!
    //
    // QString trackNumber = ...; // contains the string to be split
    // QString trackTotal;        // initially empty
    // TrackNumbers::splitString(trackNumber, &trackNumber, &trackTotal);
    //
    // It's perfectly legal if a caller passes a string for splitting
    // that in turn receives the actual track number.
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
QString TrackNumbers::joinAsString(
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
    return joinAsString(actualText, totalText);
}
