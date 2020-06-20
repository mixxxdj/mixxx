#include <QtDebug>

#include "control/controlproxy.h"
#include "control/control.h"

ControlProxy::ControlProxy(QObject* pParent)
        : QObject(pParent),
          m_pControl(NULL) {
}

ControlProxy::ControlProxy(const QString& g, const QString& i, QObject* pParent, ControlFlags flags)
        : QObject(pParent) {
    initialize(ConfigKey(g, i), flags);
}

ControlProxy::ControlProxy(const char* g, const char* i, QObject* pParent, ControlFlags flags)
        : QObject(pParent) {
    initialize(ConfigKey(g, i), flags);
}

ControlProxy::ControlProxy(const ConfigKey& key, QObject* pParent, ControlFlags flags)
        : QObject(pParent) {
    initialize(key, flags);
}

void ControlProxy::initialize(const ConfigKey& key, ControlFlags flags) {
    m_key = key;
    // Don't bother looking up the control if key is NULL. Prevents log spew.
    if (!key.isNull()) {
        m_pControl = ControlDoublePrivate::getControl(key, flags);
    }
}

ControlProxy::~ControlProxy() {
    //qDebug() << "ControlProxy::~ControlProxy()";
}

