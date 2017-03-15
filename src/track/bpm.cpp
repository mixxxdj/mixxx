#include "track/bpm.h"

namespace mixxx {

/*static*/ constexpr double Bpm::kValueUndefined;
/*static*/ constexpr double Bpm::kValueMin;

double Bpm::valueFromString(const QString& str, bool* pValid) {
    if (pValid) {
        *pValid = false;
    }
    if (str.trimmed().isEmpty()) {
        return kValueUndefined;
    }
    bool valueValid = false;
    double value = str.toDouble(&valueValid);
    if (valueValid) {
        if (kValueUndefined == value) {
            // special case
            if (pValid) {
                *pValid = true;
            }
            return value;
        }
        if (isValidValue(value)) {
            if (pValid) {
                *pValid = true;
            }
            return value;
        } else {
            qDebug() << "Invalid BPM value:" << str << "->" << value;
        }
    } else {
        qDebug() << "Failed to parse BPM:" << str;
    }
    return kValueUndefined;
}

QString Bpm::valueToString(double value) {
    if (isValidValue(value)) {
        //TODO: Shouldn't this be formatted in some way, instead of letting it output in scientific notation?
        return QString::number(value);
    } else {
        return QString();
    }
}

void Bpm::normalizeValue() {
    if (isValidValue(m_value)) {
        const double normalizedValue = valueFromString(valueToString(m_value));
        // NOTE(uklotzde): Subsequently formatting and parsing the
        // normalized value should not alter it anymore!
        DEBUG_ASSERT(normalizedValue == valueFromString(valueToString(normalizedValue)));
        m_value = normalizedValue;
    }
}

} //namespace mixxx
