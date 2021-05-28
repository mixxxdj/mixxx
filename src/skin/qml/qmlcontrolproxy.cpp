#include "skin/qml/qmlcontrolproxy.h"

#include "moc_qmlcontrolproxy.cpp"

namespace mixxx {
namespace skin {
namespace qml {

namespace {
constexpr double kDefaultValue = 0.0;
constexpr double kDefaultParameter = 0.0;
} // namespace

QmlControlProxy::QmlControlProxy(QObject* parent)
        : QObject(parent),
          m_isComponentComplete(false),
          m_pControlProxy(nullptr) {
}

void QmlControlProxy::componentComplete() {
    m_isComponentComplete = true;
    reinitializeFromKey();
}

bool QmlControlProxy::isKeyValid() const {
    return m_coKey.isValid();
}

bool QmlControlProxy::isInitialized() const {
    return m_pControlProxy != nullptr;
}

void QmlControlProxy::setGroup(const QString& group) {
    if (m_coKey.group == group) {
        return;
    }

    const bool keyValidBeforeSet = isKeyValid();
    m_coKey.group = group;
    emit groupChanged(group);

    const bool keyValidAfterSet = isKeyValid();
    if (keyValidBeforeSet != keyValidAfterSet) {
        emit keyValidChanged(keyValidAfterSet);
    }

    reinitializeFromKey();
}

const QString& QmlControlProxy::getGroup() const {
    return m_coKey.group;
}

void QmlControlProxy::setKey(const QString& key) {
    if (m_coKey.item == key) {
        return;
    }

    const bool keyValidBeforeSet = isKeyValid();
    m_coKey.item = key;
    emit keyChanged(key);

    const bool keyValidAfterSet = isKeyValid();
    if (keyValidBeforeSet != keyValidAfterSet) {
        emit keyValidChanged(keyValidAfterSet);
    }

    reinitializeFromKey();
}

const QString& QmlControlProxy::getKey() const {
    return m_coKey.item;
}

void QmlControlProxy::setValue(double newValue) {
    if (!isInitialized()) {
        emit valueChanged(kDefaultValue);
        return;
    }
    m_pControlProxy->set(newValue);
    slotControlProxyValueChanged(newValue);
}

double QmlControlProxy::getValue() const {
    if (!isInitialized()) {
        return kDefaultValue;
    }
    return m_pControlProxy->get();
}

void QmlControlProxy::setParameter(double newValue) {
    if (!isInitialized()) {
        emit parameterChanged(kDefaultValue);
        return;
    }
    m_pControlProxy->setParameter(newValue);
    emit valueChanged(m_pControlProxy->get());
    emit parameterChanged(newValue);
}

double QmlControlProxy::getParameter() const {
    if (!isInitialized()) {
        return kDefaultParameter;
    }
    return m_pControlProxy->getParameter();
}

void QmlControlProxy::reinitializeFromKey() {
    // Just ignore this if the component is still loading, because group or key may not be set yet.
    if (!m_isComponentComplete) {
        return;
    }

    // We don't need to reinitialize or reset the underlying control proxy if
    // the CO key didn't change.
    if (isInitialized() && m_coKey == m_pControlProxy->getKey()) {
        return;
    }

    // If the key is invalid, reset the control proxy if necessary.
    if (!isKeyValid()) {
        qWarning() << "QmlControlProxy: Tried to initialize CO" << m_coKey
                   << " with invalid key, resetting...";
        if (isInitialized()) {
            m_pControlProxy.reset();
            emit initializedChanged(false);
        }
        return;
    }

    // We don't need to warn here if the control is missing, because we'll do a
    // check below and print a warning anyway. If the key is invalid, this will
    // still trigger an assertion because we checked the key validity above. If
    // it's still invalid, that's a programming error.
    std::unique_ptr<ControlProxy> pControlProxy =
            std::make_unique<ControlProxy>(
                    m_coKey, this, ControlFlag::NoWarnIfMissing);

    // This should never happen, but it doesn't hurt to check.
    VERIFY_OR_DEBUG_ASSERT(pControlProxy != nullptr) {
        qWarning() << "QmlControlProxy: Requested CO " << m_coKey
                   << " returned nullptr, resetting...";
        if (isInitialized()) {
            m_pControlProxy.reset();
            emit initializedChanged(false);
        }
        return;
    }

    // If the control does not exist, reset the control proxy if necessary.
    if (!pControlProxy->valid()) {
        qWarning() << "QmlControlProxy: Requested CO" << m_coKey << " does not exist, resetting...";
        if (isInitialized()) {
            m_pControlProxy.reset();
            emit initializedChanged(false);
        }
        return;
    }

    // Set the control proxy and emit signal
    const bool wasInitialized = isInitialized();
    m_pControlProxy = std::move(pControlProxy);
    if (!wasInitialized) {
        emit initializedChanged(true);
    }
    m_pControlProxy->connectValueChanged(this, &QmlControlProxy::slotControlProxyValueChanged);
    slotControlProxyValueChanged(m_pControlProxy->get());
}

void QmlControlProxy::slotControlProxyValueChanged(double newValue) {
    emit valueChanged(newValue);
    emit parameterChanged(m_pControlProxy->getParameter());
}

} // namespace qml
} // namespace skin
} // namespace mixxx
