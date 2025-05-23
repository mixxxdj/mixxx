#include "control/controlobject.h"

#include "control/controlproxy.h"
#include "moc_controlobject.cpp"

ControlObject::ControlObject()
        : m_key("", ""),
          m_isExecutingCallback(false) {}

ControlObject::ControlObject(const ConfigKey& key,
        bool bIgnoreNops,
        bool bTrack,
        bool bPersist,
        double defaultValue)
        : m_key(key),
          m_isExecutingCallback(false) {
    m_pControl = ControlDoublePrivate::getControl(
            key, bIgnoreNops, bTrack, bPersist, defaultValue);

    if (m_pControl) {
        connect(m_pControl.data(), &ControlDoublePrivate::valueChanged,
                this, &ControlObject::privateValueChanged);
    }
}

ControlObject::~ControlObject() {
    bool success = ControlDoublePrivate::releaseControl(m_key);
    DEBUG_ASSERT(success);
}

ControlObject* ControlObject::getControl(const ConfigKey& key, ControlFlags flags) {
    Q_UNUSED(flags);
    return new ControlObject(key);
}

bool ControlObject::exists(const ConfigKey& key) {
    return ControlDoublePrivate::exists(key);
}

double ControlObject::get(const ConfigKey& key) {
    return ControlDoublePrivate::get(key);
}

void ControlObject::set(const ConfigKey& key, const double& value) {
    ControlDoublePrivate::set(key, value);
}

double ControlObject::getParameter() const {
    return m_pControl ? m_pControl->getParameter() : 0.0;
}

double ControlObject::getParameterForValue(double value) const {
    return m_pControl ? m_pControl->getParameterForValue(value) : 0.0;
}

double ControlObject::getParameterForMidi(double midiValue) const {
    return m_pControl ? m_pControl->getParameterForMidi(midiValue) : 0.0;
}

void ControlObject::setParameter(double v) {
    if (m_pControl) m_pControl->setParameter(v);
}

void ControlObject::setParameterFrom(double v, QObject* pSender) {
    if (m_pControl) m_pControl->setParameterFrom(v, pSender);
}

void ControlObject::setReadOnly() {
    if (m_pControl) {
        m_pControl->connectValueChangeRequest(this, &ControlObject::readOnlyHandler);
    }
}

void ControlObject::privateValueChanged(double value, QObject* pSetter) {
    Q_UNUSED(pSetter);
    emit valueChanged(value);
}

void ControlObject::readOnlyHandler(double v) {
    Q_UNUSED(v);
}

void ControlObject::setValueFromMidi(MidiOpCode o, double v) {
    Q_UNUSED(o);
    setParameter(v);
}

double ControlObject::getMidiParameter() const {
    return getParameter();
}

void ControlObject::triggerCallback() {
    if (m_isExecutingCallback) {
        return;
    }
    m_isExecutingCallback = true;

    if (m_pControl) {
        m_pControl->set(m_pControl->get(), this);
    }

    m_isExecutingCallback = false;
}