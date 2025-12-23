#include "control/controlproxy.h"

#include "control/control.h"
#include "moc_controlproxy.cpp"

ControlProxy::ControlProxy(const QString& g, const QString& i, QObject* pParent, ControlFlags flags)
        : ControlProxy(ConfigKey(g, i), pParent, flags) {
}

ControlProxy::ControlProxy(const ConfigKey& key, QObject* pParent, ControlFlags flags)
        : QObject(pParent) {
    m_pControl = ControlDoublePrivate::getControl(key, flags);
    if (!m_pControl) {
        DEBUG_ASSERT(flags & ControlFlag::AllowMissingOrInvalid);
        m_pControl = ControlDoublePrivate::getDefaultControl();
    }
    DEBUG_ASSERT(m_pControl);
}

ControlProxy::~ControlProxy() {
    //qDebug() << "ControlProxy::~ControlProxy()";
}

const ConfigKey& ControlProxy::getKey() const {
    return m_pControl->getKey();
}
