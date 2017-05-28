#ifndef MIXXX_UTIL_LOGGER_H
#define MIXXX_UTIL_LOGGER_H


#include <QByteArray>
#include <QLatin1String>
#include <QtDebug>

#include "util/logging.h"


namespace mixxx {

class Logger final {
public:
    Logger() = default;
    explicit Logger(const char* logContext);
    explicit Logger(const QLatin1String& logContext);

    QDebug log(QDebug stream) const {
        return stream << m_preambleChars.data();
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
