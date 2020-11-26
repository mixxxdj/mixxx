#pragma once
#include <QDateTime>
#include <QObject>
#include <QString>
#include <memory>

namespace mixxx {

enum class NotificationFlag {
    None = 0,
    Sticky = 1,
    Default = None,
};
Q_DECLARE_FLAGS(NotificationFlags, NotificationFlag);
Q_DECLARE_OPERATORS_FOR_FLAGS(NotificationFlags);

class Notification : public QObject {
    Q_OBJECT

  public:
    explicit Notification(const QString& text, NotificationFlags flags = NotificationFlag::Default);
    ~Notification(){};

    NotificationFlags flags() const {
        return m_flags;
    }

    const QString& text() const {
        return m_text;
    }

    void setTimeoutSecs(int timeout) {
        m_timeoutSecs = timeout;
    }

    int timeoutSecs() const {
        return m_timeoutSecs;
    }

    void setLastUpdated(const QDateTime& lastUpdated) {
        m_lastUpdated = lastUpdated;
    }

    QDateTime lastUpdated() const {
        return m_lastUpdated;
    }

  private:
    const NotificationFlags m_flags;
    const QString m_text;
    int m_timeoutSecs;
    QDateTime m_lastUpdated;
};

typedef Notification* NotificationPointer;

} // namespace mixxx
