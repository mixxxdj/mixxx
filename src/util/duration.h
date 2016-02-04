#ifndef UTIL_DURATION_H
#define UTIL_DURATION_H

#include <QMetaType>
#include <QString>
#include <QtDebug>
#include <QtGlobal>
#include <QTextStreamFunction>

#include "util/assert.h"

namespace mixxx {
namespace {

const qint64 kMillisPerSecond = 1e3;
const qint64 kMicrosPerSecond = 1e6;
const qint64 kNanosPerSecond = 1e9;
const qint64 kNanosPerMilli = 1e6;
const qint64 kNanosPerMicro = 1e3;

}  // namespace

class DurationBase {
  public:
    enum Units {
        HEX,
        SECONDS,
        MILLIS,
        MICROS,
        NANOS
    };

    // Returns the duration as an integer number of seconds (rounded-down).
    inline qint64 toIntegerSeconds() const {
        return m_durationNanos / kNanosPerSecond;
    }

    // Returns the duration as a floating point number of seconds.
    inline double toDoubleSeconds() const {
        return static_cast<double>(m_durationNanos) / kNanosPerSecond;
    }

    // Returns the duration as an integer number of milliseconds (rounded-down).
    inline qint64 toIntegerMillis() const {
        return m_durationNanos / kNanosPerMilli;
    }

    // Returns the duration as a floating point number of milliseconds.
    inline qint64 toDoubleMillis() const {
        return static_cast<double>(m_durationNanos) / kNanosPerMilli;
    }

    // Returns the duration as an integer number of microseconds (rounded-down).
    inline qint64 toIntegerMicros() const {
        return m_durationNanos / kNanosPerMicro;
    }

    // Returns the duration as a floating point number of microseconds.
    inline qint64 toDoubleMicros() const {
        return static_cast<double>(m_durationNanos) / kNanosPerMicro;
    }

    // Returns the duration as an integer number of nanoseconds. The duration is
    // represented internally as nanoseconds so no rounding occurs.
    inline qint64 toIntegerNanos() const {
        return m_durationNanos;
    }

    // Returns the duration as an integer number of nanoseconds.
    inline qint64 toDoubleNanos() const {
        return static_cast<double>(m_durationNanos);
    }

  protected:
    DurationBase(qint64 durationNanos)
        : m_durationNanos(durationNanos) {
    }

    qint64 m_durationNanos;
};

class DurationDebug : public DurationBase {
  public:
    DurationDebug(const DurationBase& duration, Units unit)
        : DurationBase(duration),
          m_unit(unit) {
    }

    friend QDebug operator<<(QDebug debug, const DurationDebug& dd) {
        switch (dd.m_unit) {
        case HEX:
        {
            QByteArray ret("0x0000000000000000");
            QByteArray hex = QByteArray::number(dd.m_durationNanos, 16);
            ret.replace(18 - hex.size(), hex.size(), hex);
            return debug << ret;
        }
        case SECONDS:
            return debug << dd.toIntegerSeconds() << "s";
        case MILLIS:
            return debug << dd.toIntegerMillis() << "ms";
        case MICROS:
            return debug << dd.toIntegerMicros() << "us";
        case NANOS:
            return debug << dd.m_durationNanos << "ns";
        default:
            DEBUG_ASSERT(!"Unit unknown");
            return debug << dd.m_durationNanos << "ns";
        }
    }

  private:
    Units m_unit;
};

// Represents a duration in a type-safe manner. Provides conversion methods to
// convert between physical units. Durations can be negative.
class Duration : public DurationBase {
  public:
    // Returns a Duration object representing a duration of 'seconds'.
    static Duration fromSeconds(qint64 seconds) {
        return Duration(seconds * kNanosPerSecond);
    }

    // Returns a Duration object representing a duration of 'millis'.
    static Duration fromMillis(qint64 millis) {
        return Duration(millis * kNanosPerMilli);
    }

    // Returns a Duration object representing a duration of 'micros'.
    static Duration fromMicros(qint64 micros) {
        return Duration(micros * kNanosPerMicro);
    }

    // Returns a Duration object representing a duration of 'nanos'.
    static Duration fromNanos(qint64 nanos) {
        return Duration(nanos);
    }

    Duration()
        : DurationBase(0) {
    }

    const Duration operator+(const Duration& other) const {
        Duration result = *this;
        result += other;
        return result;
    }

    Duration& operator+=(const Duration& other) {
        m_durationNanos += other.m_durationNanos;
        return *this;
    }

    const Duration operator-(const Duration& other) const {
        Duration result = *this;
        result -= other;
        return result;
    }

    Duration& operator-=(const Duration& other) {
        m_durationNanos -= other.m_durationNanos;
        return *this;
    }

    friend const Duration operator*(const Duration& duration, int scalar) {
        Duration result = duration;
        result.m_durationNanos *= scalar;
        return result;
    }

    friend const Duration operator*(int scalar, const Duration& duration) {
        return duration * scalar;
    }

    Duration& operator*=(int scalar) {
        m_durationNanos *= scalar;
        return *this;
    }

    friend bool operator==(const Duration& lhs, const Duration& rhs) {
        return lhs.m_durationNanos == rhs.m_durationNanos;
    }

    friend bool operator!=(const Duration& lhs, const Duration& rhs) {
        return !(lhs == rhs);
    }

    friend bool operator<(const Duration& lhs, const Duration& rhs) {
        return lhs.m_durationNanos < rhs.m_durationNanos;
    }

    friend bool operator>(const Duration& lhs, const Duration& rhs) {
        return lhs.m_durationNanos > rhs.m_durationNanos;
    }

    friend bool operator<=(const Duration& lhs, const Duration& rhs) {
        return lhs.m_durationNanos <= rhs.m_durationNanos;
    }

    friend bool operator>=(const Duration& lhs, const Duration& rhs) {
        return lhs.m_durationNanos >= rhs.m_durationNanos;
    }

    friend QDebug operator<<(QDebug debug, const Duration& duration) {
        return debug << duration.m_durationNanos << "ns";
    }

    // Formats the duration as a two's-complement hexadecimal string.
    QString formatHex() const {
        // Format as fixed-width (8 digits).
        return QString("0x%1").arg(m_durationNanos, 16, 16, QLatin1Char('0'));
    }

    // Formats the duration as a two's-complement hexadecimal string.
    inline DurationDebug debugHex() const {
        return debug(HEX);
    }

    QString formatNanosWithUnit() const {
        return QString("%1 ns").arg(toIntegerNanos());
    }

    DurationDebug debugNanosWithUnit() const {
        return debug(NANOS);
    }

    QString formatMicrosWithUnit() const {
        return QString("%1 us").arg(toIntegerMicros());
    }

    DurationDebug debugMicrosWithUnit() const {
        return debug(MICROS);
    }

    QString formatMillisWithUnit() const {
        return QString("%1 ms").arg(toIntegerMillis());
    }

    DurationDebug debugMillisWithUnit() const {
        return debug(MILLIS);
    }

    QString formatSecondsWithUnit() const {
        return QString("%1 s").arg(toIntegerSeconds());
    }

    DurationDebug debugSecondsWithUnit() const {
        return debug(SECONDS);
    }

    DurationDebug debug(Units unit) const {
        return DurationDebug(*this, unit);
    }

  private:
    Duration(qint64 durationNanos)
            : DurationBase(durationNanos) {
    }
};

}  // namespace mixxx

Q_DECLARE_METATYPE(mixxx::Duration)

#endif /* UTIL_DURATION_H */
