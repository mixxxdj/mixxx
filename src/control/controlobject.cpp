#include "control/controlobject.h"

#include <QHash>
#include <QMutexLocker>
#include <QSet>
#include <QtDebug>

#include "control/control.h"
#include "moc_controlobject.cpp"
#include "util/stat.h"
#include "util/timer.h"

ControlObject::ControlObject() {
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
    }
}

ControlObject::~ControlObject() {
    if (m_pControl) {
        const bool success = m_pControl->resetCreatorCO(this);
        Q_UNUSED(success);
        DEBUG_ASSERT(success);
    }
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

void ControlObject::setValueFromMidi(MidiOpCode o, double v) {
    if (m_pControl) {
        m_pControl->setValueFromMidi(o, v);
    }
}

double ControlObject::getMidiParameter() const {
    return m_pControl ? m_pControl->getMidiParameter() : 0.0;
}

// static
double ControlObject::get(const ConfigKey& key) {
    QSharedPointer<ControlDoublePrivate> pCop = ControlDoublePrivate::getControl(key);
    return pCop ? pCop->get() : 0.0;
}

double ControlObject::getParameter() const {
    return m_pControl ? m_pControl->getParameter() : 0.0;
}

double ControlObject::getParameterForValue(double value) const {
    return m_pControl ? m_pControl->getParameterForValue(value) : 0.0;
}

double ControlObject::getParameterForMidi(double midiParameter) const {
    return m_pControl ? m_pControl->getParameterForMidi(midiParameter) : 0.0;
}

void ControlObject::setParameter(double v) {
    if (m_pControl) {
        m_pControl->setParameter(v, this);
    }
}

void ControlObject::setParameterFrom(double v, QObject* pSender) {
    if (m_pControl) {
        m_pControl->setParameter(v, pSender);
    }
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
