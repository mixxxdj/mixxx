#pragma once

#include <QApplication>
#include <QMetaObject>

enum class stateOfprocessQueuedEvent { last_event,
    outdated_event };

class CompressingProxy : public QObject {
    Q_OBJECT
  private:
    stateOfprocessQueuedEvent processQueuedEvents();

    bool m_recursiveSearchForLastEventOngoing;

  public slots:
    void slotValueChanged(double value, QObject* obj);

  signals:
    void signalValueChanged(double, QObject*);

  public:
    // No default constructor, since the proxy must be a child of the
    // target object.
    explicit CompressingProxy(QObject* parent);
};
