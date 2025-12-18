#include "util/ratelimitedlogger.h"

#include <QMutexLocker>

namespace mixxx {

RateLimitedLogger::RateLimitedLogger(qint64 intervalMillis)
        : m_intervalMillis(intervalMillis) {
}

void RateLimitedLogger::log(
        const QString& key, const std::function<void(int suppressedCount)>& logger) const {
    QMutexLocker locker(&m_mutex);
    Entry& entry = m_entries[key];
    if (!entry.timer.isValid()) {
        entry.timer.start();
        entry.suppressed = 0;
        logger(0);
        return;
    }

    if (entry.timer.hasExpired(m_intervalMillis)) {
        const int suppressed = entry.suppressed;
        entry.timer.restart();
        entry.suppressed = 0;
        logger(suppressed);
        return;
    }

    ++entry.suppressed;
}

} // namespace mixxx
