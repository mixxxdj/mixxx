#ifndef MIXXX_UTIL_LOGGER_H
#define MIXXX_UTIL_LOGGER_H


#include <QByteArray>
#include <QLatin1String>
#include <QtDebug>

#include "util/logging.h"
#include "util/performancetimer.h"


namespace mixxx {

class Logger final {
public:
    Logger() = default;
    explicit Logger(const char* logContext);
    explicit Logger(const QLatin1String& logContext);

    QDebug log(QDebug stream) const {
        return stream << m_preambleChars.data();
    }

    QDebug trace() const {
        // Trace logs just provide more details than debug logs
        return log(qDebug());
    }

    bool traceEnabled() const {
        return Logging::traceEnabled();
    }

    // Trace the elapsed time of some timed action in microseconds.
    template<typename T>
    void tracePerformance(const T& timed, const PerformanceTimer& timer) const {
        if (traceEnabled()) {
            trace() << timed << "took"
                    << timer.elapsed().toIntegerMicros() << "us";
        }
    }
    void tracePerformance(const QLatin1String& timed, const PerformanceTimer& timer) const {
        tracePerformance(timed.latin1(), timer);
    }
    void tracePerformance(const QString& timed, const PerformanceTimer& timer) const {
        tracePerformance(timed.toLocal8Bit().data(), timer);
    }

    QDebug debug() const {
        return log(qDebug());
    }

    bool debugEnabled() const {
        return Logging::debugEnabled();
    }

    QDebug info() const {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        return log(qInfo());
#else
        // Qt4 does not support log level Info, use Debug instead
        return debug();
#endif
    }

    bool infoEnabled() const {
        return Logging::infoEnabled();
    }

    QDebug warning() const {
        return log(qWarning());
    }

    QDebug critical() const {
        return log(qCritical());
    }

private:
    QByteArray m_preambleChars;
};

}  // namespace mixxx


#endif /* MIXXX_UTIL_LOGGER_H */
