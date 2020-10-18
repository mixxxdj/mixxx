#ifndef MIXXX_BPM_H
#define MIXXX_BPM_H

#include <QtDebug>

#include "util/math.h"

namespace mixxx {

// DTO for storing BPM information.
class Bpm final {
public:
    static constexpr double kValueUndefined = 0.0;
    static constexpr double kValueMin = 0.0; // lower bound (exclusive)
    static constexpr double kValueMax = 300.0; // higher bound (inclusive)

    Bpm()
        : Bpm(kValueUndefined) {
    }
    explicit Bpm(double value)
        : m_value(value) {
    }

    static double normalizeValue(double value);

    // Adjusts floating-point values to match their string representation
    // in file tags to account for rounding errors and false positives
    // when checking for modifications.
    // NOTE(2020-01-08, uklotzde): Since bpm values are stored with
    // integer precision in ID3 tags, bpm values are only considered
    // as modified if their rounded integer values differ. But even
    // then this pre-normalization step should not be skipped to prevent
    // fluttering values for other tag formats.
    void normalizeBeforeExport() {
        m_value = normalizeValue(m_value);
    }

    static bool isValidValue(double value) {
        return kValueMin < value;
    }

    bool hasValue() const {
        return isValidValue(m_value);
    }
    double getValue() const {
        return m_value;
    }
    void setValue(double value) {
        m_value = value;
    }
    void resetValue() {
        m_value = kValueUndefined;
    }

    static double valueFromString(const QString& str, bool* pValid = nullptr);
    static QString valueToString(double value);
    static int valueToInteger(double value) {
        return static_cast<int>(std::round(value));
    }

    enum class Comparison {
        Default, // full precision
        Integer, // rounded
        String, // stringified
    };

    bool compareEq(
            const Bpm& bpm,
            Comparison cmp = Comparison::Default) const {
        switch (cmp) {
        case Comparison::Integer:
            return Bpm::valueToInteger(getValue()) == Bpm::valueToInteger(bpm.getValue());
        case Comparison::String:
            return Bpm::valueToString(getValue()) == Bpm::valueToString(bpm.getValue());
        case Comparison::Default:
        default:
            return getValue() == bpm.getValue();
        }
    }

private:
    double m_value;
};

inline
bool operator==(const Bpm& lhs, const Bpm& rhs) {
    return lhs.compareEq(rhs);
}

inline
bool operator!=(const Bpm& lhs, const Bpm& rhs) {
    return !(lhs == rhs);
}

inline
QDebug operator<<(QDebug dbg, const Bpm& arg) {
    return dbg << arg.getValue();
}

}

Q_DECLARE_TYPEINFO(mixxx::Bpm, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(mixxx::Bpm)

#endif // MIXXX_BPM_H
