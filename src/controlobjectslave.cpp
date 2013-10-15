#include <QApplication>
#include <QtDebug>

#include "controlobjectslave.h"
#include "control/control.h"

ControlObjectSlave::ControlObjectSlave(QObject* pParent)
        : QObject(pParent),
          m_pControl(NULL) {
}

ControlObjectSlave::ControlObjectSlave(const QString& g, const QString& i, QObject* pParent)
        : QObject(pParent) {
    initialize(ConfigKey(g, i));
}

ControlObjectSlave::ControlObjectSlave(const char* g, const char* i, QObject* pParent)
        : QObject(pParent) {
    initialize(ConfigKey(g, i));
}

ControlObjectSlave::ControlObjectSlave(const ConfigKey& key, QObject* pParent)
        : QObject(pParent) {
    initialize(key);
}

void ControlObjectSlave::initialize(const ConfigKey& key) {
    m_pControl = ControlDoublePrivate::getControl(key);
}

ControlObjectSlave::~ControlObjectSlave() {
}

bool ControlObjectSlave::connectValueChanged(const QObject* receiver,
        const char* method, Qt::ConnectionType type) {
    bool ret = false;
    if (m_pControl) {
        ret = connect((QObject*)this, SIGNAL(valueChanged(double)),
                receiver, method, type);
        if (ret) {
            // connect to ControlObjectPrivate only if required
            ret = connect(m_pControl.data(), SIGNAL(valueChanged(double, QObject*)),
                    this, SLOT(slotValueChanged(double, QObject*)),
                    Qt::DirectConnection);
        }
    }
    return ret;
}

bool ControlObjectSlave::connectValueChanged(
        const char* method, Qt::ConnectionType type) {
    return connectValueChanged(parent(), method, type);
}


double ControlObjectSlave::get() {
    return m_pControl ? m_pControl->get() : 0.0;
}

void ControlObjectSlave::slotSet(double v) {
    set(v);
}

void ControlObjectSlave::set(double v) {
    if (m_pControl) {
        m_pControl->set(v, this);
    }
}

void ControlObjectSlave::reset() {
    if (m_pControl) {
        // NOTE(rryan): This is important. The originator of this action does
        // not know the resulting value so it makes sense that we should emit a
        // general valueChanged() signal even though the change originated from
        // us. For this reason, we provide NULL here so that the change is
        // broadcast as valueChanged() and not valueChangedByThis().
        m_pControl->reset();
    }
}

void ControlObjectSlave::emitValueChanged() {
    emit(valueChanged(get()));
}

void ControlObjectSlave::slotValueChanged(double v, QObject* pSetter) {
    if (pSetter != this) {
        // This is base implementation of this function without scaling
        emit(valueChanged(v));
    }
}
