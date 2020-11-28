#pragma once
#include <QDateTime>
#include <QDialogButtonBox>
#include <QObject>
#include <QString>
#include <functional>
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
    struct NotificationButton {
        QString label;
        QObject* receiver;
        std::function<void()> func;
    };

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

    QVector<NotificationButton> buttons() {
        return m_buttons;
    }

    void addButton(const QString& buttonLabel, QObject* receiver, std::function<void()> func) {
        NotificationButton button;
        button.label = buttonLabel;
        button.receiver = receiver;
        button.func = func;
        m_buttons.append(button);
    }

  signals:
    void closed();

  private:
    const NotificationFlags m_flags;
    const QString m_text;
    int m_timeoutSecs;
    QDateTime m_lastUpdated;
    QVector<NotificationButton> m_buttons;
};

typedef Notification* NotificationPointer;

} // namespace mixxx
