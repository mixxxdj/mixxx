#include "skin/qml/controlproxyqml.h"

#include "moc_controlproxyqml.cpp"

ControlProxyQml::ControlProxyQml(QObject* parent)
        : QObject(parent),
          m_pControlProxy(nullptr) {
}

void ControlProxyQml::setGroup(const QString& group) {
    m_coKey.group = group;
    initialize();
}

const QString& ControlProxyQml::getGroup() const {
    return m_coKey.group;
}

void ControlProxyQml::setKey(const QString& key) {
    m_coKey.item = key;
    initialize();
}

const QString& ControlProxyQml::getKey() const {
    return m_coKey.item;
}

void ControlProxyQml::setValue(double newValue) {
    if (!m_pControlProxy) {
        return;
    }
    m_pControlProxy->set(newValue);
    m_pControlProxy->emitValueChanged();
}

double ControlProxyQml::getValue() const {
    if (!m_pControlProxy) {
        return -1;
    }
    return m_pControlProxy->get();
}

void ControlProxyQml::setParameter(double newValue) {
    if (!m_pControlProxy) {
        return;
    }
    m_pControlProxy->setParameter(newValue);
    m_pControlProxy->emitValueChanged();
}

double ControlProxyQml::getParameter() const {
    if (!m_pControlProxy) {
        return -1;
    }
    return m_pControlProxy->getParameter();
}

void ControlProxyQml::initialize() {
    if (m_coKey.isValid()) {
        m_pControlProxy = std::make_unique<ControlProxy>(
                m_coKey, this, ControlFlag::AllowMissingOrInvalid);
        m_pControlProxy->connectValueChanged(this, &ControlProxyQml::controlProxyValueChanged);
        m_pControlProxy->emitValueChanged();
    }
}

void ControlProxyQml::controlProxyValueChanged(double newValue) {
    emit valueChanged(newValue);
    emit parameterChanged(m_pControlProxy->getParameter());
}
