#include "skin/qml/qmlcontrolproxy.h"

#include "moc_qmlcontrolproxy.cpp"

namespace mixxx {
namespace skin {
namespace qml {

QmlControlProxy::QmlControlProxy(QObject* parent)
        : QObject(parent),
          m_pControlProxy(nullptr) {
}

bool QmlControlProxy::isKeyValid() const {
    return m_coKey.isValid();
}

bool QmlControlProxy::isInitialized() const {
    return m_pControlProxy != nullptr;
}

void QmlControlProxy::setGroup(const QString& group) {
    const bool keyValidBeforeSet = isKeyValid();
    m_coKey.group = group;
    const bool keyValidAfterSet = isKeyValid();
    if (keyValidBeforeSet != keyValidAfterSet) {
        emit keyValidChanged(keyValidAfterSet);
    }
    tryInitialize();
}

const QString& QmlControlProxy::getGroup() const {
    return m_coKey.group;
}

void QmlControlProxy::setKey(const QString& key) {
    const bool keyValidBeforeSet = isKeyValid();
    m_coKey.item = key;
    const bool keyValidAfterSet = isKeyValid();
    if (keyValidBeforeSet != keyValidAfterSet) {
        emit keyValidChanged(keyValidAfterSet);
    }
    tryInitialize();
}

const QString& QmlControlProxy::getKey() const {
    return m_coKey.item;
}

void QmlControlProxy::setValue(double newValue) {
    if (!isInitialized()) {
        qWarning() << "QmlControlProxy: Attempted to set value" << newValue
                   << "on non-initialized CO" << m_coKey;
        return;
    }
    m_pControlProxy->set(newValue);
    m_pControlProxy->emitValueChanged();
}

double QmlControlProxy::getValue() const {
    if (!isInitialized()) {
        qWarning() << "QmlControlProxy: Attempted to get value from "
                      "non-initialized CO"
                   << m_coKey << "(returning 0)";
        return 0;
    }
    return m_pControlProxy->get();
}

void QmlControlProxy::setParameter(double newValue) {
    if (!isInitialized()) {
        qWarning() << "QmlControlProxy: Attempted to set parameter" << newValue
                   << "on non-initialized CO" << m_coKey;
        return;
    }
    m_pControlProxy->setParameter(newValue);
    m_pControlProxy->emitValueChanged();
}

double QmlControlProxy::getParameter() const {
    if (!isInitialized()) {
        qWarning() << "QmlControlProxy: Attempted to get parameter from "
                      "non-initialized CO"
                   << m_coKey << "(returning 0)";
        return 0;
    }
    return m_pControlProxy->getParameter();
}

bool QmlControlProxy::initialize() {
    const bool wasInitialized = isInitialized();
    VERIFY_OR_DEBUG_ASSERT(!wasInitialized || m_coKey != m_pControlProxy->getKey()) {
        qWarning() << "QmlControlProxy: Tried to initialize although CO"
                   << m_coKey
                   << "is already initialized with same key, ignoring";
        return false;
    }

    VERIFY_OR_DEBUG_ASSERT(isKeyValid()) {
        qWarning() << "QmlControlProxy: Tried to initialize CO" << m_coKey
                   << " with invalid key, resetting...";
        m_pControlProxy.reset();
        if (wasInitialized) {
            emit initializedChanged(false);
        }
        return false;
    }

    std::unique_ptr<ControlProxy> pControlProxy =
            std::make_unique<ControlProxy>(
                    m_coKey, this, ControlFlag::NoAssertIfMissing);
    VERIFY_OR_DEBUG_ASSERT(pControlProxy != nullptr) {
        qWarning() << "QmlControlProxy: Requested CO " << m_coKey
                   << " returned nullptr, resetting...";
        m_pControlProxy.reset();
        if (wasInitialized) {
            emit initializedChanged(false);
        }
        return false;
    }

    if (!pControlProxy->valid()) {
        qWarning() << "QmlControlProxy: Invalid CO" << m_coKey << " requested, resetting...";
        m_pControlProxy.reset();
        if (wasInitialized) {
            emit initializedChanged(false);
        }
        return false;
    }

    m_pControlProxy = std::move(pControlProxy);
    if (!wasInitialized) {
        emit initializedChanged(true);
    }
    m_pControlProxy->connectValueChanged(this, &QmlControlProxy::slotControlProxyValueChanged);
    m_pControlProxy->emitValueChanged();

    return true;
}

bool QmlControlProxy::tryInitialize() {
    // Initialize when the key is valid and the underlying control proxy was
    // either not initialized or has a different key.
    if (isKeyValid() && (!isInitialized() || m_coKey != m_pControlProxy->getKey())) {
        return initialize();
    }
    return false;
}

void QmlControlProxy::slotControlProxyValueChanged(double newValue) {
    emit valueChanged(newValue);
    emit parameterChanged(m_pControlProxy->getParameter());
}

} // namespace qml
} // namespace skin
} // namespace mixxx
