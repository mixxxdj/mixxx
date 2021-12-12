#pragma once

#include <QApplication>
#include <QMetaObject>

class CompressingProxy : public QObject {
    Q_OBJECT
  private:
    bool isLatestEventInQueue();

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
