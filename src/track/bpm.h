#pragma once

#include <QtDebug>

#include "util/fpclassify.h"
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

    static QString displayValueText(double value);

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
        return util_isfinite(value) && kValueMin < value;
    }

    bool hasValue() const {
        return isValidValue(m_value);
    }
    double getValue() const {
        VERIFY_OR_DEBUG_ASSERT(hasValue()) {
            return kValueUndefined;
        }
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
        if (!hasValue() && !bpm.hasValue()) {
            // Both values are invalid and thus equal.
            return true;
        }

        if (hasValue() != bpm.hasValue()) {
            // One value is valid, one is not.
            return false;
        }

        // At this point both values are valid
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

    QString displayText() const {
        return displayValueText(m_value);
    }

    Bpm& operator+=(double increment) {
        DEBUG_ASSERT(hasValue());
        m_value += increment;
        return *this;
    }

    Bpm& operator-=(double decrement) {
        DEBUG_ASSERT(hasValue());
        m_value -= decrement;
        return *this;
    }

    Bpm& operator*=(double multiple) {
        DEBUG_ASSERT(hasValue());
        m_value *= multiple;
        return *this;
    }

    Bpm& operator/=(double divisor) {
        DEBUG_ASSERT(hasValue());
        m_value /= divisor;
        return *this;
    }

private:
    double m_value;
};

/// Bpm can be added to a double
inline Bpm operator+(Bpm bpm, double bpmDiff) {
    return Bpm(bpm.getValue() + bpmDiff);
}

/// Bpm can be subtracted from a double
inline Bpm operator-(Bpm bpm, double bpmDiff) {
    return Bpm(bpm.getValue() - bpmDiff);
}

/// Two Bpm values can be subtracted to get a double
inline double operator-(Bpm bpm1, Bpm bpm2) {
    return bpm1.getValue() - bpm2.getValue();
}

// Adding two Bpm is not allowed, because it makes no sense semantically.

/// Bpm can be multiplied or divided by a double
inline Bpm operator*(Bpm bpm, double multiple) {
    return Bpm(bpm.getValue() * multiple);
}

inline Bpm operator/(Bpm bpm, double divisor) {
    return Bpm(bpm.getValue() / divisor);
}

inline bool operator==(Bpm bpm1, Bpm bpm2) {
    if (!bpm1.hasValue() && !bpm2.hasValue()) {
        return true;
    }
    return bpm1.hasValue() && bpm2.hasValue() && bpm1.getValue() == bpm2.getValue();
}

inline bool operator!=(Bpm bpm1, Bpm bpm2) {
    return !(bpm1 == bpm2);
}

inline bool operator<(Bpm bpm1, Bpm bpm2) {
    return bpm1.getValue() < bpm2.getValue();
}

inline bool operator<=(Bpm bpm1, Bpm bpm2) {
    return (bpm1 == bpm2) || bpm1 < bpm2;
}

inline bool operator>(Bpm bpm1, Bpm bpm2) {
    return bpm2 < bpm1;
}

inline bool operator>=(Bpm bpm1, Bpm bpm2) {
    return bpm2 <= bpm1;
}

inline QDebug operator<<(QDebug dbg, Bpm arg) {
    if (arg.hasValue()) {
        dbg.nospace() << "Bpm(" << arg.getValue() << ")";
    } else {
        dbg << "Bpm(Invalid)";
    }
    return dbg;
}
}

Q_DECLARE_TYPEINFO(mixxx::Bpm, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(mixxx::Bpm)
