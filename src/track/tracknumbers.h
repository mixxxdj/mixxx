#ifndef TRACKNUMBERS_H
#define TRACKNUMBERS_H

#include <QString>
#include <QMetaType>

#include <utility> // std::pair


// DTO for storing the current and total number of tracks for an album.
// Both numbers are 1-based and 0 indicates an undefined value.
class TrackNumbers final {
public:
    // TODO(uklotzde): Replace 'const' with 'constexpr'
    static const int kValueUndefined = 0;
    static const int kValueMin = 1; // lower bound (inclusive)

    // Separates the total number of tracks from the current
    // track number in the textual format.
    static const QString kDefaultSeparator;

    static bool isUndefinedValue(int value) {
        return kValueUndefined == value;
    }

    static bool isValidValue(int value) {
        return isUndefinedValue(value) || (kValueMin <= value);
    }

    explicit TrackNumbers(int current = kValueUndefined, int total = kValueUndefined)
        : m_current(current),
          m_total(total) {
    }

    bool hasCurrent() const {
        return !isUndefinedValue(m_current);
    }
    bool isCurrentValid() const {
        return isValidValue(m_current);
    }
    int getCurrent() const {
        return m_current;
    }
    QString getCurrentText() const;
    void setCurrent(int current) {
        m_current = current;
    }

    bool hasTotal() const {
        return !isUndefinedValue(m_total);
    }
    bool isTotalValid() const {
        return isValidValue(m_total);
    }
    int getTotal() const {
        return m_total;
    }
    QString getTotalText() const;
    void setTotal(int total) {
        m_total = total;
    }

    bool isValid() const {
        return isCurrentValid() && isTotalValid();
    }

    enum class ParseResult {
        EMPTY,
        VALID,
        INVALID
    };

    static std::pair<TrackNumbers, ParseResult> fromString(
            const QString& str,
            const QString& separator = kDefaultSeparator);

    QString toString(
            const QString& separator = kDefaultSeparator) const;

private:
    int m_current;
    int m_total;
};

inline
bool operator==(const TrackNumbers& lhs, const TrackNumbers& rhs) {
    return (lhs.getCurrent() == rhs.getCurrent()) &&
            (lhs.getTotal() == rhs.getTotal());
}

inline
bool operator!=(const TrackNumbers& lhs, const TrackNumbers& rhs) {
    return !(lhs == rhs);
}

Q_DECLARE_METATYPE(TrackNumbers)

#endif // TRACKNUMBERS_H
