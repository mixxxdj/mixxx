#pragma once

#include <QApplication>
#include <QMetaObject>

#include "preferences/configobject.h"
#include "util/runtimeloggingcategory.h"

class CompressingProxy : public QObject {
    Q_OBJECT
  private:
    enum class StateOfProcessQueuedEvent {
        LastEvent,
        OutdatedEvent,
        NoEvent
    };
    StateOfProcessQueuedEvent processQueuedEvents();

    // Members needed for warning message
    const ConfigKey m_key;
    const RuntimeLoggingCategory m_logger;

    // Functional members
    bool m_recursiveSearchForLastEventOngoing;
    int m_recursionDepth;

  public slots:
    void slotValueChanged(double value, QObject* obj);

  signals:
    void signalValueChanged(double, QObject*);

  public:
    // No default constructor, since the proxy must be a child of the object with the Qt event queue
    explicit CompressingProxy(const ConfigKey& key,
            const RuntimeLoggingCategory& logger,
            QObject* pParent = nullptr);
};
