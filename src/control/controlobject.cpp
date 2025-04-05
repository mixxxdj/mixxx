#include "control/controlobject.h"

#include <QtDebug>

#include "control/control.h"
#include "moc_controlobject.cpp"

ControlObject::ControlObject()
    : m_pControl(ControlDoublePrivate::getDefaultControl()) {}

ControlObject::ControlObject(const ConfigKey& key,
                             bool bIgnoreNops,
                             bool bTrack,
                             bool bPersist,
                             double defaultValue)
    : m_key(key) {
    if (m_key.isValid()) {
        m_pControl = ControlDoublePrivate::getControl(m_key,
                                                      ControlFlag::None,
                                                      this,
                                                      bIgnoreNops,
                                                      bTrack,
                                                      bPersist,
                                                      defaultValue);
    }

    if (m_pControl) {
        connect(m_pControl.data(),
                &ControlDoublePrivate::valueChanged,
                this,
                &ControlObject::privateValueChanged,
                Qt::DirectConnection);
    } else {
        m_pControl = ControlDoublePrivate::getDefaultControl();
    }
}

ControlObject::~ControlObject() {
    DEBUG_ASSERT(m_pControl);
    const bool success = m_pControl->resetCreatorCO(this);
    Q_UNUSED(success);
    DEBUG_ASSERT(success);
}

void ControlObject::triggerCallback() {
    if (m_isExecutingCallback) return;

    m_isExecutingCallback = true;

    try {
        if (m_callback) m_callback();
    } catch (const std::exception& e) {
        qWarning() << "Exception in ControlObject callback:" << e.what();
    }

    m_isExecutingCallback = false;
}

void ControlObject::privateValueChanged(double dValue, QObject* pSender) {
    if (pSender != this) {
        emit valueChanged(dValue);
    }
}

ControlObject* ControlObject::getControl(const ConfigKey& key, ControlFlags flags) {
    QSharedPointer<ControlDoublePrivate> pCDP = ControlDoublePrivate::getControl(key, flags);
    return pCDP ? pCDP->getCreatorCO() : nullptr;
}

bool ControlObject::exists(const ConfigKey& key) {
    return !ControlDoublePrivate::getControl(key, ControlFlag::NoWarnIfMissing).isNull();
}

void ControlObject::setValueFromMidi(MidiOpCode o, double v) {
    m_pControl->setValueFromMidi(o, v);
}

double ControlObject::getMidiParameter() const {
    return m_pControl->getMidiParameter();
}

double ControlObject::get(const ConfigKey& key) {
    QSharedPointer<ControlDoublePrivate> pCop = ControlDoublePrivate::getControl(key);
    return pCop ? pCop->get() : 0.0;
}

QString ControlObject::name() const {
    return m_pControl ? m_pControl->name() : QString();
}

void ControlObject::setName(const QString& name) {
    if (m_pControl) {
        m_pControl->setName(name);
    }
}

QString ControlObject::description() const {
    return m_pControl ? m_pControl->description() : QString();
}

void ControlObject::setDescription(const QString& description) {
    if (m_pControl) {
        m_pControl->setDescription(description);
    }
}

void ControlObject::setKbdRepeatable(bool enable) {
    if (m_pControl) {
        m_pControl->setKbdRepeatable(enable);
    }
}

bool ControlObject::getKbdRepeatable() const {
    return m_pControl ? m_pControl->getKbdRepeatable() : false;
}

void ControlObject::addAlias(const ConfigKey& aliasKey) const {
    ControlDoublePrivate::insertAlias(aliasKey, m_key);
}

ConfigKey ControlObject::getKey() const {
    return m_key;
}

double ControlObject::get() const {
    return m_pControl ? m_pControl->get() : 0.0;
}

bool ControlObject::toBool() const {
    return get() > 0.0;
}

bool ControlObject::toBool(const ConfigKey& key) {
    return get(key) > 0.0;
}

void ControlObject::set(double value) {
    if (m_pControl) {
        m_pControl->set(value, this);
    }
}

void ControlObject::setAndConfirm(double value) {
    if (m_pControl) {
        m_pControl->setAndConfirm(value, this);
    }
}

void ControlObject::forceSet(double value) {
    setAndConfirm(value);
}

void ControlObject::set(const ConfigKey& key, const double& value) {
    QSharedPointer<ControlDoublePrivate> pCop = ControlDoublePrivate::getControl(key);
    if (pCop) {
        pCop->set(value, nullptr);
    }
}

void ControlObject::reset() {
    if (m_pControl) {
        m_pControl->reset();
    }
}

void ControlObject::setDefaultValue(double dValue) {
    if (m_pControl) {
        m_pControl->setDefaultValue(dValue);
    }
}

double ControlObject::defaultValue() const {
    return m_pControl ? m_pControl->defaultValue() : 0.0;
}

double ControlObject::getParameter() const {
    return m_pControl->getParameter();
}

double ControlObject::getParameterForValue(double value) const {
    return m_pControl->getParameterForValue(value);
}

double ControlObject::getParameterForMidi(double midiValue) const {
    return m_pControl->getParameterForMidi(midiValue);
}

void ControlObject::setParameter(double v) {
    m_pControl->setParameter(v, this);
}

void ControlObject::setParameterFrom(double v, QObject* pSender) {
    m_pControl->setParameter(v, pSender);
}

void ControlObject::setReadOnly() {
    connectValueChangeRequest(this, &ControlObject::readOnlyHandler, Qt::DirectConnection);
}

void ControlObject::readOnlyHandler(double v) {
    qWarning() << m_key << "is read-only. Ignoring set of value:" << v;
}
