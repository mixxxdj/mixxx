#pragma once

#include <QElapsedTimer>
#include <QHash>
#include <QMutex>
#include <QString>
#include <functional>

namespace mixxx {

/// Utility for rate limiting repeated log messages.
class RateLimitedLogger {
  public:
    explicit RateLimitedLogger(qint64 intervalMillis = 2000);

    void log(const QString& key, const std::function<void(int suppressedCount)>& logger) const;

  private:
    struct Entry {
        QElapsedTimer timer;
        int suppressed{0};
    };

    const qint64 m_intervalMillis;
    mutable QHash<QString, Entry> m_entries;
    mutable QMutex m_mutex;
};

} // namespace mixxx
