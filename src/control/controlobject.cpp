#include "control/controlobject.h"

#include <QtDebug>

#include "control/control.h"
#include "moc_controlobject.cpp"

ControlObject::ControlObject()
        : m_pControl(ControlDoublePrivate::getDefaultControl()) {
}

ControlObject::ControlObject(const ConfigKey& key,
        bool bIgnoreNops,
        bool bTrack,
        bool bPersist,
        double defaultValue)
        : m_key(key) {
    // Don't bother looking up the control if key is invalid. Prevents log spew.
    if (m_key.isValid()) {
        m_pControl = ControlDoublePrivate::getControl(m_key,
                ControlFlag::None,
                this,
                bIgnoreNops,
                bTrack,
                bPersist,
                defaultValue);
    }

    // getControl can fail and return a NULL control even with the create flag.
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
    if (m_isExecutingCallback) {
        return;  // Prevent reentrant execution
    }

    m_isExecutingCallback = true;  // Mark callback as running

    try {
        if (m_callback) {
            m_callback();
        }
    } catch (const std::exception& e) {
        qWarning() << "Exception in ControlObject callback:" << e.what();
    }

    m_isExecutingCallback = false;  // Reset flag after execution
}

// slot
void ControlObject::privateValueChanged(double dValue, QObject* pSender) {
    // Only emit valueChanged() if we did not originate this change.
    if (pSender != this) {
        emit valueChanged(dValue);
    }
}

// static
ControlObject* ControlObject::getControl(const ConfigKey& key, ControlFlags flags) {
    //qDebug() << "ControlObject::getControl for (" << key.group << "," << key.item << ")";
    QSharedPointer<ControlDoublePrivate> pCDP = ControlDoublePrivate::getControl(key, flags);
    if (pCDP) {
        return pCDP->getCreatorCO();
    }
    return nullptr;
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

// static
double ControlObject::get(const ConfigKey& key) {
    QSharedPointer<ControlDoublePrivate> pCop = ControlDoublePrivate::getControl(key);
    return pCop ? pCop->get() : 0.0;
}

double ControlObject::getParameter() const {
    return m_pControl->getParameter();
}

double ControlObject::getParameterForValue(double value) const {
    return m_pControl->getParameterForValue(value);
}

double ControlObject::getParameterForMidi(double midiParameter) const {
    return m_pControl->getParameterForMidi(midiParameter);
}

void ControlObject::setParameter(double v) {
    m_pControl->setParameter(v, this);
}

void ControlObject::setParameterFrom(double v, QObject* pSender) {
    m_pControl->setParameter(v, pSender);
}

// static
void ControlObject::set(const ConfigKey& key, const double& value) {
    QSharedPointer<ControlDoublePrivate> pCop = ControlDoublePrivate::getControl(key);
    if (pCop) {
        pCop->set(value, nullptr);
    }
}

void ControlObject::setReadOnly() {
    connectValueChangeRequest(this, &ControlObject::readOnlyHandler,
                              Qt::DirectConnection);
}

void ControlObject::readOnlyHandler(double v) {
    qWarning() << m_key << "is read-only. Ignoring set of value:" << v;
}

void ControlObject::triggerCallback() {
    if (m_isExecutingCallback) {
        return;  // Prevent reentrant execution
    }

    m_isExecutingCallback = true;  // Mark callback as running

    try {
        if (m_callback) {
            m_callback();
        }
    } catch (const std::exception& e) {
        qWarning() << "Exception in ControlObject callback:" << e.what();
    }

    m_isExecutingCallback = false;  // Reset flag after execution
}
