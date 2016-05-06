#include <QApplication>
#include <QtDebug>

#include "control/controlproxy.h"
#include "control/control.h"

ControlProxy::ControlProxy(QObject* pParent)
        : QObject(pParent),
          m_pControl(NULL) {
}

ControlProxy::ControlProxy(const QString& g, const QString& i, QObject* pParent)
        : QObject(pParent) {
    initialize(ConfigKey(g, i));
}

ControlProxy::ControlProxy(const char* g, const char* i, QObject* pParent)
        : QObject(pParent) {
    initialize(ConfigKey(g, i));
}

ControlProxy::ControlProxy(const ConfigKey& key, QObject* pParent)
        : QObject(pParent) {
    initialize(key);
}

void ControlProxy::initialize(const ConfigKey& key) {
    m_key = key;
    // Don't bother looking up the control if key is NULL. Prevents log spew.
    if (!key.isNull()) {
        m_pControl = ControlDoublePrivate::getControl(key);
    }
}

ControlProxy::~ControlProxy() {
    //qDebug() << "ControlProxy::~ControlProxy()";
}

bool ControlProxy::connectValueChanged(const QObject* receiver,
        const char* method, Qt::ConnectionType requestedConnectionType) {

    if (!m_pControl) {
        return false;
    }

    // We connect to the
    // ControlObjectPrivate only once and in a way that
    // the requested ConnectionType is working as desired.
    // We try to avoid direct connections if not requested
    // since you cannot safely delete an object with a pending
    // direct connection. This fixes bug Bug #1406124
    // requested: Auto -> COP = Auto / SCO = Auto
    // requested: Direct -> COP = Direct / SCO = Direct
    // requested: Queued -> COP = Queued / SCO = Auto
    // requested: BlockingQueued -> Assert(false)

    const char* copSlot;
    Qt::ConnectionType copConnection;
    Qt::ConnectionType scoConnection;
    switch(requestedConnectionType) {
    case Qt::AutoConnection:
        copSlot = SLOT(slotValueChangedAuto(double, QObject*));
        copConnection = Qt::AutoConnection;
        scoConnection = Qt::AutoConnection;
        break;
    case Qt::DirectConnection:
        copSlot = SLOT(slotValueChangedDirect(double, QObject*));
        copConnection = Qt::DirectConnection;
        scoConnection = Qt::DirectConnection;
        break;
    case Qt::QueuedConnection:
        copSlot = SLOT(slotValueChangedQueued(double, QObject*));
        copConnection = Qt::QueuedConnection;
        scoConnection = Qt::AutoConnection;
        break;
    case Qt::BlockingQueuedConnection:
        // We must not block the signal source by a blocking connection
        DEBUG_ASSERT(false);
        return false;
    default:
        DEBUG_ASSERT(false);
        return false;
    }

    if (!connect((QObject*)this, SIGNAL(valueChanged(double)),
                      receiver, method, scoConnection)) {
        return false;
    }

    // Connect to ControlObjectPrivate only if required. Do not allow
    // duplicate connections.

    // use only explicit direct connection if requested
    // the caller must not delete this until the all signals are
    // processed to avoid segfaults
    connect(m_pControl.data(), SIGNAL(valueChanged(double, QObject*)),
            this, copSlot,
            static_cast<Qt::ConnectionType>(copConnection | Qt::UniqueConnection));
    return true;
}

// connect to parent object
bool ControlProxy::connectValueChanged(
        const char* method, Qt::ConnectionType type) {
    DEBUG_ASSERT(parent() != NULL);
    return connectValueChanged(parent(), method, type);
}
