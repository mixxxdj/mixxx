#ifndef UTIL_DURATION_H
#define UTIL_DURATION_H

#include <QtGlobal>
#include <QMetaType>
#include <QString>

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
    inline qint64 toSeconds() const {
        return m_timestamp_nanos / kNanosPerSecond;
    }

    // Returns the duration as an integer number of milliseconds (rounded-down).
    inline qint64 toMillis() const {
        return m_timestamp_nanos / kNanosPerMilli;
    }

    // Returns the duration as an integer number of microseconds (rounded-down).
    inline qint64 toMicros() const {
        return m_timestamp_nanos / kNanosPerMicro;
    }

    // Returns the duration as an integer number of nanoseconds.
    inline qint64 toNanos() const {
        return m_timestamp_nanos;
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

    bool operator==(const Duration& other) const {
        return m_timestamp_nanos == other.m_timestamp_nanos;
    }

    bool operator!=(const Duration& other) const {
        return !(*this == other);
    }

    // Formats the duration as a two's-complement hexadecimal string.
    QString formatHex() const {
        // Format as fixed-width (8 digits).
        return "0x" + QString("%1").arg(m_timestamp_nanos, 16, 16,
                                        QLatin1Char('0'));
    }

    QString formatNanosWithUnit() const {
        return QString("%1ns").arg(toNanos());
    }

    QString formatMicrosWithUnit() const {
        return QString("%1us").arg(toMicros());
    }

    QString formatMillisWithUnit() const {
        return QString("%1ms").arg(toMillis());
    }

    QString formatSecondsWithUnit() const {
        return QString("%1s").arg(toSeconds());
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
