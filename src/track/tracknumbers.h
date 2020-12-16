#pragma once

#include <QString>
#include <QMetaType>


// DTO for storing the actual and total number of tracks for an album.
// Both numbers are 1-based and 0 indicates an undefined value.
class TrackNumbers final {
public:
    static constexpr int kValueUndefined = 0;
    static constexpr int kValueMin = 1; // lower bound (inclusive)

    // Separates the total number of tracks from the actual
    // track number in the textual format.
    static const QString kSeparator;

    static bool isUndefinedValue(int value) {
        return kValueUndefined == value;
    }

    static bool isValidValue(int value) {
        return isUndefinedValue(value) || (kValueMin <= value);
    }

    // Parses a value from a string. Returns true if the value
    // has been parsed successfully and (optionally) stores the
    // result in pValue. The caller is still responsible to check
    // if the resulting value is valid!
    static bool parseValueFromString(
            const QString& str,
            int* pValue = nullptr);

    explicit TrackNumbers(int actualValue = kValueUndefined, int totalValue = kValueUndefined)
        : m_actualValue(actualValue),
          m_totalValue(totalValue) {
    }

    bool hasActual() const {
        return !isUndefinedValue(m_actualValue);
    }
    bool isActualValid() const {
        return isValidValue(m_actualValue);
    }
    int getActual() const {
        return m_actualValue;
    }
    void setActual(int actualValue) {
        m_actualValue = actualValue;
    }

    bool hasTotal() const {
        return !isUndefinedValue(m_totalValue);
    }
    bool isTotalValid() const {
        return isValidValue(m_totalValue);
    }
    int getTotal() const {
        return m_totalValue;
    }
    void setTotal(int totalValue) {
        m_totalValue = totalValue;
    }

    bool isValid() const {
        return isActualValid() && isTotalValid();
    }

    enum class ParseResult {
        EMPTY,
        VALID,
        INVALID
    };

    // Creates a new instance from string(s).
    // If the caller is only interested in the ParseResult
    // a nullptr can be passed for the corresponding output
    // parameter pParsed.
    static ParseResult parseFromStrings(
            const QString& actualText,
            const QString& totalText,
            TrackNumbers* pParsed = nullptr);
    static ParseResult parseFromString(
            const QString& str,
            TrackNumbers* pParsed = nullptr);

    // Formats this instance as string(s).
    // Both output parameters pActualText and pTotalText
    // are optional and the caller might pass a nullptr.
    void toStrings(
            QString* pActualText,
            QString* pTotalText) const;
    QString toString() const;

    // Splits a string into actual and total part. Both
    // output parameters pActualText and pTotalText are
    // optional and the caller might pass a nullptr.
    static void splitString(
            const QString& str,
            QString* pActualText = nullptr,
            QString* pTotalText = nullptr);
    // Joins the actual and total strings
    static QString joinAsString(
            const QString& actualText,
            const QString& totalText);

private:
    int m_actualValue;
    int m_totalValue;
};

inline
bool operator==(const TrackNumbers& lhs, const TrackNumbers& rhs) {
    return (lhs.getActual() == rhs.getActual()) &&
            (lhs.getTotal() == rhs.getTotal());
}

inline
bool operator!=(const TrackNumbers& lhs, const TrackNumbers& rhs) {
    return !(lhs == rhs);
}

Q_DECLARE_TYPEINFO(TrackNumbers, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(TrackNumbers)
