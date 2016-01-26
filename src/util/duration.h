#ifndef UTIL_DURATION_H
#define UTIL_DURATION_H

#include <QMetaType>
#include <QString>
#include <QtDebug>
#include <QtGlobal>

namespace mixxx {
namespace {

const qint64 kMillisPerSecond = 1e3;
const qint64 kMicrosPerSecond = 1e6;
const qint64 kNanosPerSecond = 1e9;
const qint64 kNanosPerMilli = 1e6;
const qint64 kNanosPerMicro = 1e3;

}  // namespace

// Represents a duration in a type-safe manner. Provides conversion methods to
// convert between physical units. Durations can be negative.
class Duration {
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

    Duration() : m_timestamp_nanos(0) {}

    // Returns the duration as an integer number of seconds (rounded-down).
    inline qint64 toIntegerSeconds() const {
        return m_timestamp_nanos / kNanosPerSecond;
    }

    // Returns the duration as a floating point number of seconds.
    inline double toDoubleSeconds() const {
        return static_cast<double>(m_timestamp_nanos) / kNanosPerSecond;
    }

    // Returns the duration as an integer number of milliseconds (rounded-down).
    inline qint64 toIntegerMillis() const {
        return m_timestamp_nanos / kNanosPerMilli;
    }

    // Returns the duration as a floating point number of milliseconds.
    inline qint64 toDoubleMillis() const {
        return static_cast<double>(m_timestamp_nanos) / kNanosPerMilli;
    }

    // Returns the duration as an integer number of microseconds (rounded-down).
    inline qint64 toIntegerMicros() const {
        return m_timestamp_nanos / kNanosPerMicro;
    }

    // Returns the duration as a floating point number of microseconds.
    inline qint64 toDoubleMicros() const {
        return static_cast<double>(m_timestamp_nanos) / kNanosPerMicro;
    }

    // Returns the duration as an integer number of nanoseconds. The duration is
    // represented internally as nanoseconds so no rounding occurs.
    inline qint64 toIntegerNanos() const {
        return m_timestamp_nanos;
    }

    // Returns the duration as an integer number of nanoseconds.
    inline qint64 toDoubleNanos() const {
        return static_cast<double>(m_timestamp_nanos);
    }

    const Duration operator+(const Duration& other) const {
        Duration result = *this;
        result += other;
        return result;
    }

    Duration& operator+=(const Duration& other) {
        m_timestamp_nanos += other.m_timestamp_nanos;
        return *this;
    }

    const Duration operator-(const Duration& other) const {
        Duration result = *this;
        result -= other;
        return result;
    }

    Duration& operator-=(const Duration& other) {
        m_timestamp_nanos -= other.m_timestamp_nanos;
        return *this;
    }

    friend const Duration operator*(const Duration& duration, int scalar) {
        Duration result = duration;
        result.m_timestamp_nanos *= scalar;
        return result;
    }

    friend const Duration operator*(int scalar, const Duration& duration) {
        return duration * scalar;
    }

    Duration& operator*=(int scalar) {
        m_timestamp_nanos *= scalar;
        return *this;
    }

    friend bool operator==(const Duration& lhs, const Duration& rhs) {
        return lhs.m_timestamp_nanos == rhs.m_timestamp_nanos;
    }

    friend bool operator!=(const Duration& lhs, const Duration& rhs) {
        return !(lhs == rhs);
    }

    friend bool operator<(const Duration& lhs, const Duration& rhs) {
        return lhs.m_timestamp_nanos < rhs.m_timestamp_nanos;
    }

    friend bool operator>(const Duration& lhs, const Duration& rhs) {
        return lhs.m_timestamp_nanos > rhs.m_timestamp_nanos;
    }

    friend bool operator<=(const Duration& lhs, const Duration& rhs) {
        return lhs.m_timestamp_nanos <= rhs.m_timestamp_nanos;
    }

    friend bool operator>=(const Duration& lhs, const Duration& rhs) {
        return lhs.m_timestamp_nanos >= rhs.m_timestamp_nanos;
    }

    friend QDebug operator<<(QDebug debug, const Duration& duration) {
        return debug << duration.formatNanosWithUnit();
    }

    // Formats the duration as a two's-complement hexadecimal string.
    QString formatHex() const {
        // Format as fixed-width (8 digits).
        return "0x" + QString("%1").arg(m_timestamp_nanos, 16, 16,
                                        QLatin1Char('0'));
    }

    QString formatNanosWithUnit() const {
        return QString("%1 ns").arg(toIntegerNanos());
    }

    QString formatMicrosWithUnit() const {
        return QString("%1 us").arg(toIntegerMicros());
    }

    QString formatMillisWithUnit() const {
        return QString("%1 ms").arg(toIntegerMillis());
    }

    QString formatSecondsWithUnit() const {
        return QString("%1 s").arg(toIntegerSeconds());
    }

  private:
    Duration(qint64 nanoseconds)
            : m_timestamp_nanos(nanoseconds) {
    }

    qint64 m_timestamp_nanos;
};

}  // namespace mixxx

Q_DECLARE_METATYPE(mixxx::Duration)

#endif /* UTIL_DURATION_H */
