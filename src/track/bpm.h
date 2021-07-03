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
    static constexpr double kValueMax = 500.0; // higher bound (inclusive)

    constexpr Bpm()
            : Bpm(kValueUndefined) {
    }
    explicit constexpr Bpm(double value)
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

    bool isValid() const {
        return isValidValue(m_value);
    }
    double value() const {
        VERIFY_OR_DEBUG_ASSERT(isValid()) {
            return mixxx::Bpm::kValueUndefined;
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
        switch (cmp) {
        case Comparison::Integer:
            return Bpm::valueToInteger(value()) == Bpm::valueToInteger(bpm.value());
        case Comparison::String:
            return Bpm::valueToString(value()) == Bpm::valueToString(bpm.value());
        case Comparison::Default:
        default:
            return value() == bpm.value();
        }
    }

    QString displayText() const {
        return displayValueText(m_value);
    }

    Bpm& operator+=(double increment) {
        DEBUG_ASSERT(isValid());
        if (isValid()) {
            m_value += increment;
        }
        return *this;
    }

    Bpm& operator-=(double decrement) {
        DEBUG_ASSERT(isValid());
        if (isValid()) {
            m_value -= decrement;
        }
        return *this;
    }

    Bpm& operator*=(double multiple) {
        DEBUG_ASSERT(isValid());
        if (isValid()) {
            m_value *= multiple;
        }
        return *this;
    }

    Bpm& operator/=(double divisor) {
        DEBUG_ASSERT(isValid());
        if (isValid()) {
            m_value /= divisor;
        }
        return *this;
    }

private:
    double m_value;
};

/// Bpm can be added to a double
inline Bpm operator+(Bpm bpm, double bpmDiff) {
    VERIFY_OR_DEBUG_ASSERT(bpm.isValid()) {
        return Bpm();
    }

    return Bpm(bpm.value() + bpmDiff);
}

/// Bpm can be subtracted from a double
inline Bpm operator-(Bpm bpm, double bpmDiff) {
    VERIFY_OR_DEBUG_ASSERT(bpm.isValid()) {
        return Bpm();
    }

    return Bpm(bpm.value() - bpmDiff);
}

/// Two Bpm values can be subtracted to get a double
inline double operator-(Bpm bpm1, Bpm bpm2) {
    VERIFY_OR_DEBUG_ASSERT(bpm1.isValid() && bpm2.isValid()) {
        return mixxx::Bpm::kValueUndefined;
    }

    return bpm1.value() - bpm2.value();
}

// Adding two Bpm is not allowed, because it makes no sense semantically.

/// Bpm can be multiplied or divided by a double
inline Bpm operator*(Bpm bpm, double multiple) {
    VERIFY_OR_DEBUG_ASSERT(bpm.isValid()) {
        return Bpm();
    }

    return Bpm(bpm.value() * multiple);
}

inline Bpm operator/(Bpm bpm, double divisor) {
    VERIFY_OR_DEBUG_ASSERT(bpm.isValid()) {
        return Bpm();
    }

    return Bpm(bpm.value() / divisor);
}

inline bool operator<(Bpm bpm1, Bpm bpm2) {
    VERIFY_OR_DEBUG_ASSERT(bpm1.isValid() && bpm2.isValid()) {
        return false;
    }
    return bpm1.value() < bpm2.value();
}

inline bool operator<=(Bpm bpm1, Bpm bpm2) {
    VERIFY_OR_DEBUG_ASSERT(bpm1.isValid() && bpm2.isValid()) {
        return !bpm1.isValid() && !bpm2.isValid();
    }
    return bpm1.value() <= bpm2.value();
}

inline bool operator>(Bpm bpm1, Bpm bpm2) {
    VERIFY_OR_DEBUG_ASSERT(bpm1.isValid() && bpm2.isValid()) {
        return false;
    }
    return bpm1.value() > bpm2.value();
}

inline bool operator>=(Bpm bpm1, Bpm bpm2) {
    VERIFY_OR_DEBUG_ASSERT(bpm1.isValid() && bpm2.isValid()) {
        return !bpm1.isValid() && !bpm2.isValid();
    }
    return bpm1.value() >= bpm2.value();
}

inline bool operator==(Bpm bpm1, Bpm bpm2) {
    if (!bpm1.isValid() || !bpm2.isValid()) {
        return !bpm1.isValid() && !bpm2.isValid();
    }
    return bpm1.value() == bpm2.value();
}

inline bool operator!=(Bpm bpm1, Bpm bpm2) {
    return !(bpm1 == bpm2);
}

inline QDebug operator<<(QDebug dbg, Bpm arg) {
    if (arg.isValid()) {
        dbg << arg.value();
    } else {
        dbg.nospace() << "<Invalid FramePos>";
    }
    return dbg;
}
}

Q_DECLARE_TYPEINFO(mixxx::Bpm, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(mixxx::Bpm)
