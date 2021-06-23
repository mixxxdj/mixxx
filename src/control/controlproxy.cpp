#include "control/controlproxy.h"

#include <QtDebug>

#include "control/control.h"
#include "moc_controlproxy.cpp"

ControlProxy::ControlProxy(const QString& g, const QString& i, QObject* pParent, ControlFlags flags)
        : ControlProxy(ConfigKey(g, i), pParent, flags) {
}

ControlProxy::ControlProxy(const char* g, const char* i, QObject* pParent, ControlFlags flags)
        : ControlProxy(ConfigKey(g, i), pParent, flags) {
}

ControlProxy::ControlProxy(const ConfigKey& key, QObject* pParent, ControlFlags flags)
        : QObject(pParent),
          m_pControl(nullptr) {
    DEBUG_ASSERT(key.isValid() || flags.testFlag(ControlFlag::AllowInvalidKey));
    m_key = key;

    if (m_key.isValid()) {
        initialize(flags);
    }
}

void ControlProxy::initialize(ControlFlags flags) {
    // Prevent double initialization
    DEBUG_ASSERT(!m_pControl);

    // Prevent empty keys
    VERIFY_OR_DEBUG_ASSERT(m_key.isValid() || flags.testFlag(ControlFlag::AllowInvalidKey)) {
        return;
    }

    m_pControl = ControlDoublePrivate::getControl(m_key, flags);
    DEBUG_ASSERT(m_pControl || flags.testFlag(ControlFlag::NoAssertIfMissing));
    DEBUG_ASSERT(valid() || flags.testFlag(ControlFlag::NoAssertIfMissing));
}

ControlProxy::~ControlProxy() {
    //qDebug() << "ControlProxy::~ControlProxy()";
}
