#pragma once

#include <QApplication>
#include <QMetaObject>

class CompressingProxy : public QObject {
    Q_OBJECT
  private:
    enum class StateOfProcessQueuedEvent {
        LastEvent,
        OutdatedEvent,
        NoEvent
    };
    StateOfProcessQueuedEvent processQueuedEvents();

    bool m_recursiveSearchForLastEventOngoing;
    int m_recursionDepth;

  public slots:
    void slotValueChanged(double value, QObject* obj);

  signals:
    void signalValueChanged(double, QObject*);

  public:
    // No default constructor, since the proxy must be a child of the object with the Qt event queue
    explicit CompressingProxy(QObject* parent);
};
