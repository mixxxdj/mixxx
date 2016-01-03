#ifndef TRACKNUMBERS_H
#define TRACKNUMBERS_H

#include <QString>
#include <QMetaType>


// DTO for storing the actual and total number of tracks for an album.
// Both numbers are 1-based and 0 indicates an undefined value.
class TrackNumbers final {
public:
    // TODO(uklotzde): Replace 'const' with 'constexpr'
    static const int kValueUndefined = 0;
    static const int kValueMin = 1; // lower bound (inclusive)

    // Separates the total number of tracks from the actual
    // track number in the textual format.
    static const QString kSeparator;

    static bool isUndefinedValue(int value) {
        return kValueUndefined == value;
    }

    static bool isValidValue(int value) {
        return isUndefinedValue(value) || (kValueMin <= value);
    }

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

    static ParseResult parseFromStrings(
            const QString& actualText,
            const QString& totalText,
            TrackNumbers* pParsed);
    static ParseResult parseFromString(
            const QString& str,
            TrackNumbers* pParsed);

    void toStrings(
            QString* pActualText,
            QString* pTotalText) const;
    QString toString() const;

    // Splits a string into actual and total part
    static void splitString(
            const QString& str,
            QString* pActualText,
            QString* pTotalText);
    // Joins the actual and total strings
    static QString joinStrings(
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

Q_DECLARE_METATYPE(TrackNumbers)

#endif // TRACKNUMBERS_H
