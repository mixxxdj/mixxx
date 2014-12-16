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
        const char* method, Qt::ConnectionType type) {
    bool ret = false;
    if (m_pControl) {
        ret = connect((QObject*)this, SIGNAL(valueChanged(double)),
                      receiver, method, type);
        if (ret) {
            // Connect to ControlObjectPrivate only if required. Do not allow
            // duplicate connections.
            connect(m_pControl.data(), SIGNAL(valueChanged(double, QObject*)),
                    this, SLOT(slotValueChanged(double, QObject*)),
                    static_cast<Qt::ConnectionType>(Qt::DirectConnection |
                                                    Qt::UniqueConnection));
        }
    }
    return ret;
}

bool ControlObjectSlave::connectValueChanged(
        const char* method, Qt::ConnectionType type) {
    return connectValueChanged(parent(), method, type);
}
