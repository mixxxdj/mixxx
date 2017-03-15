#include <QApplication>
#include <QtDebug>

#include "controlobjectslave.h"
#include "control/control.h"

ControlObjectSlave::ControlObjectSlave(QObject* pParent)
        : QObject(pParent),
          m_pControl(NULL) {
}

ControlObjectSlave::ControlObjectSlave(const QString& g, const QString& i, QObject* pParent)
        : QObject(pParent) {
    initialize(ConfigKey(g, i));
}

ControlObjectSlave::ControlObjectSlave(const char* g, const char* i, QObject* pParent)
        : QObject(pParent) {
    initialize(ConfigKey(g, i));
}

ControlObjectSlave::ControlObjectSlave(const ConfigKey& key, QObject* pParent)
        : QObject(pParent) {
    initialize(key);
}

void ControlObjectSlave::initialize(const ConfigKey& key) {
    m_key = key;
    // Don't bother looking up the control if key is NULL. Prevents log spew.
    if (!key.isNull()) {
        m_pControl = ControlDoublePrivate::getControl(key);
    }
}

ControlObjectSlave::~ControlObjectSlave() {
}

bool ControlObjectSlave::connectValueChanged(const QObject* receiver,
        const char* method, Qt::ConnectionType requestedConnectionType) {

    // We connect to the
    // ControlObjectPrivate only once and in a way that
    // the requested ConnectionType is working as desired.
    // We try to avoid direct connections if not requested
    // since you cannot safely delete an object with a pending
    // direct connection. This fixes bug Bug #1406124
    // requested: Auto -> COP = Auto / SCO = Auto
    // requested: Direct -> COP = Direct / SCO = Direct
    // requested: Queued -> COP = Auto / SCO = Queued
    // requested: BlockingQueued -> Assert(false)


    bool ret = false;
    if (m_pControl) {
        // We must not block the signal source by a blocking connection
        DEBUG_ASSERT(requestedConnectionType != Qt::BlockingQueuedConnection);
        ret = connect((QObject*)this, SIGNAL(valueChanged(double)),
                      receiver, method, requestedConnectionType);
        if (ret) {
            // Connect to ControlObjectPrivate only if required. Do not allow
            // duplicate connections.
            if (requestedConnectionType == Qt::DirectConnection) {
                // use only explicit direct connection if requested
                // the caller must not delete this until the all signals are
                // processed to avoid segfaults
                connect(m_pControl.data(), SIGNAL(valueChanged(double, QObject*)),
                         this, SLOT(slotValueChangedDirect(double, QObject*)),
                         static_cast<Qt::ConnectionType>(Qt::DirectConnection |
                                                         Qt::UniqueConnection));
            } else {
                connect(m_pControl.data(), SIGNAL(valueChanged(double, QObject*)),
                        this, SLOT(slotValueChangedAuto(double, QObject*)),
                        static_cast<Qt::ConnectionType>(Qt::AutoConnection |
                                                        Qt::UniqueConnection));
            }
        }
    }
    return ret;
}

// connect to parent object
bool ControlObjectSlave::connectValueChanged(
        const char* method, Qt::ConnectionType type) {
    DEBUG_ASSERT(parent() != NULL);
    return connectValueChanged(parent(), method, type);
}
