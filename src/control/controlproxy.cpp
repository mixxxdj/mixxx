#include <QtDebug>

#include "control/controlproxy.h"
#include "control/control.h"

ControlProxy::ControlProxy(const QString& g, const QString& i, QObject* pParent, ControlFlags flags)
        : ControlProxy(ConfigKey(g, i), pParent, flags) {
}

ControlProxy::ControlProxy(const char* g, const char* i, QObject* pParent, ControlFlags flags)
        : ControlProxy(ConfigKey(g, i), pParent, flags) {
}

ControlProxy::ControlProxy(const ConfigKey& key, QObject* pParent, ControlFlags flags)
        : QObject(pParent),
          m_pControl(nullptr) {
    DEBUG_ASSERT(!key.isNull() || flags.testFlag(ControlFlag::AllowEmptyKey));
    m_key = key;

    if (!m_key.isNull()) {
        initialize(flags);
    }
}

void ControlProxy::initialize(ControlFlags flags) {
    // Prevent double initialization
    DEBUG_ASSERT(!m_pControl);

    // Prevent empty keys
    VERIFY_OR_DEBUG_ASSERT(!m_key.isNull() || flags.testFlag(ControlFlag::AllowEmptyKey)) {
        return;
    }

    m_pControl = ControlDoublePrivate::getControl(m_key, flags);
    DEBUG_ASSERT(m_pControl || flags.testFlag(ControlFlag::NoAssertIfMissing));
    DEBUG_ASSERT(valid() || flags.testFlag(ControlFlag::NoAssertIfMissing));
}

ControlProxy::~ControlProxy() {
    //qDebug() << "ControlProxy::~ControlProxy()";
}
