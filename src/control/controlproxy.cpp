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

void ControlProxy::initialize(const ConfigKey& key, bool warn) {
    m_key = key;
    // Don't bother looking up the control if key is NULL. Prevents log spew.
    if (!key.isNull()) {
        m_pControl = ControlDoublePrivate::getControl(key, warn);
    }
}

ControlProxy::~ControlProxy() {
    //qDebug() << "ControlProxy::~ControlProxy()";
}

