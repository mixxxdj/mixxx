#pragma once
#include <QObject>
#include <QString>
#include <memory>

namespace mixxx {

enum class NotificationFlag {
    None = 0,
    AutoTimeout = 1,
    Default = AutoTimeout,
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

  private:
    const NotificationFlags m_flags;
    QString m_text;
};

class NotificationPointer : public std::shared_ptr<Notification> {
  public:
    NotificationPointer() = default;
    explicit NotificationPointer(Notification* pNotification)
            : std::shared_ptr<Notification>(pNotification, deleteLater) {
    }

  private:
    static void deleteLater(Notification* pNotification) {
        if (pNotification) {
            pNotification->deleteLater();
        }
    }
};

} // namespace mixxx
