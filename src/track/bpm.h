#ifndef MIXXX_BPM_H
#define MIXXX_BPM_H

#include <QtDebug>

#include "util/math.h"

namespace mixxx {

// DTO for storing BPM information.
class Bpm final {
public:
    // TODO(uklotzde): Replace 'const' with 'constexpr'
    // (and copy initialization from .cpp file) after switching to
    // Visual Studio 2015 on Windows.
    static const double kValueUndefined;
    static const double kValueMin; // lower bound (exclusive)

    Bpm()
        : Bpm(kValueUndefined) {
    }
    explicit Bpm(double value)
        : m_value(value) {
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
    void normalizeValue();

    static double valueFromString(const QString& str, bool* pValid = nullptr);
    static QString valueToString(double value);
    static int valueToInteger(double value) {
        return std::round(value);
    }

private:
    double m_value;
};

inline
bool operator==(const Bpm& lhs, const Bpm& rhs) {
    return lhs.getValue() == rhs.getValue();
}

inline
bool operator!=(const Bpm& lhs, const Bpm& rhs) {
    return !(lhs == rhs);
}

}

Q_DECLARE_METATYPE(mixxx::Bpm)

#endif // MIXXX_BPM_H
