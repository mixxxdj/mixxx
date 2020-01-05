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
    // in file tags to account for rounding errors.
    // NOTE(2019-02-19, uklotzde): The pre-normalization cannot prevent
    // repeated export of metadata for files with ID3 tags that are only
    // able to store the BPM value with integer precision! In case of a
    // fractional value the ID3 metadata is always detected as modified
    // and will be exported regardless if it has actually been modified
    // or not.
    // TL;DR: If metadata export is enabled ID3 tags will be rewritten
    // for all files with a fractional bpm values even if their metadata
    // has not been modified.
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
        return std::round(value);
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
