#pragma once

#include <QMetaType>
#include <QString>
#include <QtDebug>
#include <QtGlobal>
#include <QTextStreamFunction>

#include "util/assert.h"

namespace mixxx {

class DurationBase {

  public:

    enum Units {
        SECONDS,
        MILLIS,
        MICROS,
        NANOS
    };

    // Returns the duration as an integer number of seconds (rounded-down).
    constexpr qint64 toIntegerSeconds() const {
        return m_durationNanos / kNanosPerSecond;
    }

    // Returns the duration as a floating point number of seconds.
    constexpr double toDoubleSeconds() const {
        return static_cast<double>(m_durationNanos) / kNanosPerSecond;
    }

    // Returns the duration as an integer number of milliseconds (rounded-down).
    constexpr qint64 toIntegerMillis() const {
        return m_durationNanos / kNanosPerMilli;
    }

    // Returns the duration as a floating point number of milliseconds.
    constexpr double toDoubleMillis() const {
        return static_cast<double>(m_durationNanos) / kNanosPerMilli;
    }

    // Returns the duration as an integer number of microseconds (rounded-down).
    constexpr qint64 toIntegerMicros() const {
        return m_durationNanos / kNanosPerMicro;
    }

    // Returns the duration as a floating point number of microseconds.
    constexpr double toDoubleMicros() const {
        return static_cast<double>(m_durationNanos) / kNanosPerMicro;
    }

    // Returns the duration as an integer number of nanoseconds. The duration is
    // represented internally as nanoseconds so no rounding occurs.
    constexpr qint64 toIntegerNanos() const {
        return m_durationNanos;
    }

    // Returns the duration as an integer number of nanoseconds.
    constexpr double toDoubleNanos() const {
        return static_cast<double>(m_durationNanos);
    }

    enum class Precision {
        SECONDS,
        CENTISECONDS,
        MILLISECONDS
    };

    // The standard way of formatting a floating-point duration in seconds.
    // Used for display of track duration, etc.
    static QString formatTime(
            double dSeconds,
            Precision precision = Precision::SECONDS);
    // Alternative format for duration based on seconds
    static QString formatSeconds(
            double dSeconds,
            Precision precision = Precision::SECONDS);
    static QString formatSecondsLong(
            double dSeconds,
            Precision precision = Precision::SECONDS);
    static QString formatKiloSeconds(
            double dSeconds,
            Precision precision = Precision::SECONDS);

    static constexpr qint64 kMillisPerSecond = 1000;
    static constexpr qint64 kMicrosPerSecond = kMillisPerSecond * 1000;
    static constexpr qint64 kNanosPerSecond  = kMicrosPerSecond * 1000;
    static constexpr qint64 kNanosPerMilli   = kNanosPerSecond / 1000;
    static constexpr qint64 kNanosPerMicro   = kNanosPerMilli / 1000;
    static const QString kInvalidDurationString;
    static QChar kKiloGroupSeparator;
    static QChar kHectoGroupSeparator;
    static QChar kDecimalSeparator;

  protected:
    explicit constexpr DurationBase(qint64 durationNanos)
        : m_durationNanos(durationNanos) {
    }

    qint64 m_durationNanos;
};

class DurationDebug : public DurationBase {
  public:
    constexpr DurationDebug(const DurationBase& duration, Units unit)
        : DurationBase(duration),
          m_unit(unit) {
    }

    friend QDebug operator<<(QDebug debug, const DurationDebug& dd) {
        switch (dd.m_unit) {
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
    template<typename T>
    static constexpr Duration fromSeconds(T seconds) {
        return Duration(static_cast<qint64>(seconds * kNanosPerSecond));
    }

    // Returns a Duration object representing a duration of 'millis'.
    static constexpr Duration fromMillis(qint64 millis) {
        return Duration(millis * kNanosPerMilli);
    }

    // Returns a Duration object representing a duration of 'micros'.
    static constexpr Duration fromMicros(qint64 micros) {
        return Duration(micros * kNanosPerMicro);
    }

    // Returns a Duration object representing a duration of 'nanos'.
    static constexpr Duration fromNanos(qint64 nanos) {
        return Duration(nanos);
    }

    static constexpr Duration empty() {
        return Duration();
    }

    constexpr Duration()
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
    explicit constexpr Duration(qint64 durationNanos)
            : DurationBase(durationNanos) {
    }
};

}  // namespace mixxx

Q_DECLARE_TYPEINFO(mixxx::Duration, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(mixxx::Duration)
