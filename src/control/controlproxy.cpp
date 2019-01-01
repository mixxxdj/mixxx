#include "control/controlproxy.h"

#include <QtDebug>

#include "control/control.h"
#include "moc_controlproxy.cpp"

namespace {
const ConfigKey kNullKey; // because we return it as a reference
} // namespace

ControlProxy::ControlProxy(const QString& g, const QString& i, QObject* pParent, ControlFlags flags)
        : ControlProxy(ConfigKey(g, i), pParent, flags) {
}

ControlProxy::ControlProxy(const ConfigKey& key, QObject* pParent, ControlFlags flags)
        : QObject(pParent),
          m_pControl(nullptr) {
    initialize(key, flags);
}

void ControlProxy::initialize(const ConfigKey& key, ControlFlags flags) {
    // Prevent double initialization
    DEBUG_ASSERT(!m_pControl);

    // Prevent empty keys
    VERIFY_OR_DEBUG_ASSERT(key.isValid() || flags.testFlag(ControlFlag::AllowInvalidKey)) {
        return;
    }

    m_pControl = ControlDoublePrivate::getControl(key, flags);
    DEBUG_ASSERT(m_pControl || flags.testFlag(ControlFlag::NoAssertIfMissing));
    DEBUG_ASSERT(valid() || flags.testFlag(ControlFlag::NoAssertIfMissing));
}

ControlProxy::~ControlProxy() {
    //qDebug() << "ControlProxy::~ControlProxy()";
}

const ConfigKey& ControlProxy::getKey() const {
    return m_pControl ? m_pControl->getKey() : kNullKey;
}
