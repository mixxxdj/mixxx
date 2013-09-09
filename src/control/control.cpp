#include <QtDebug>
#include <QMutexLocker>

#include "control/control.h"

#include "util/stat.h"
#include "util/timer.h"

// Static member variable definition
QHash<ConfigKey, ControlDoublePrivate*> ControlDoublePrivate::m_sqCOHash;
QMutex ControlDoublePrivate::m_sqCOHashMutex;

ControlDoublePrivate::ControlDoublePrivate()
        : m_bIgnoreNops(true),
          m_bTrack(false),
          m_trackType(Stat::UNSPECIFIED),
          m_trackFlags(Stat::COUNT | Stat::SUM | Stat::AVERAGE |
                       Stat::SAMPLE_VARIANCE | Stat::MIN | Stat::MAX),
          m_confirmRequired(false) {
    initialize();
}

ControlDoublePrivate::ControlDoublePrivate(ConfigKey key,
                                           bool bIgnoreNops, bool bTrack)
        : m_key(key),
          m_bIgnoreNops(bIgnoreNops),
          m_bTrack(bTrack),
          m_trackKey("control " + m_key.group + "," + m_key.item),
          m_trackType(Stat::UNSPECIFIED),
          m_trackFlags(Stat::COUNT | Stat::SUM | Stat::AVERAGE |
                       Stat::SAMPLE_VARIANCE | Stat::MIN | Stat::MAX),
          m_confirmRequired(false) {
    initialize();
}

void ControlDoublePrivate::initialize() {
    m_defaultValue.setValue(0);
    m_value.setValue(0);

    m_sqCOHashMutex.lock();
    m_sqCOHash.insert(m_key, this);
    m_sqCOHashMutex.unlock();

    if (m_bTrack) {
        // TODO(rryan): Make configurable.
        Stat::track(m_trackKey, static_cast<Stat::StatType>(m_trackType),
                    static_cast<Stat::ComputeFlags>(m_trackFlags),
                    m_value.getValue());
    }
}

ControlDoublePrivate::~ControlDoublePrivate() {
    m_sqCOHashMutex.lock();
    m_sqCOHash.remove(m_key);
    m_sqCOHashMutex.unlock();
}

// static
ControlDoublePrivate* ControlDoublePrivate::getControl(
        const ConfigKey& key, bool bCreate, bool bIgnoreNops, bool bTrack) {
    QMutexLocker locker(&m_sqCOHashMutex);
    QHash<ConfigKey, ControlDoublePrivate*>::const_iterator it = m_sqCOHash.find(key);
    if (it != m_sqCOHash.end()) {
        return it.value();
    }
    locker.unlock();

    ControlDoublePrivate* pControl = NULL;
    if (bCreate) {
        pControl = new ControlDoublePrivate(key, bIgnoreNops, bTrack);
        locker.relock();
        m_sqCOHash.insert(key, pControl);
        locker.unlock();
    }

    if (pControl == NULL) {
        qWarning() << "ControlDoublePrivate::getControl returning NULL for ("
                   << key.group << "," << key.item << ")";
    }

    return pControl;
}

double ControlDoublePrivate::get() const {
    return m_value.getValue();
}

void ControlDoublePrivate::reset() {
    double defaultValue = m_defaultValue.getValue();
    // NOTE: pSender = NULL is important. The originator of this action does
    // not know the resulting value so it makes sense that we should emit a
    // general valueChanged() signal even though we know the originator.
    set(defaultValue, NULL);
}

void ControlDoublePrivate::set(double value, QObject* pSender) {
    // If the behavior says to ignore the set, ignore it.
    QSharedPointer<ControlNumericBehavior> pBehavior = m_pBehavior;
    if (!pBehavior.isNull() && !pBehavior->setFilter(&value)) {
        return;
    }
    if(m_confirmRequired) {
        emit(valueChangeRequest(value));
    } else {
        setInner(value, pSender);
    }
}

void ControlDoublePrivate::setAndConfirm(double value, QObject* pSender) {
    setInner(value, pSender);
}


void ControlDoublePrivate::setInner(double value, QObject* pSender) {
    if (m_bIgnoreNops && get() == value) {
        return;
    }
    m_value.setValue(value);
    emit(valueChanged(value, pSender));

    if (m_bTrack) {
        Stat::track(m_trackKey, static_cast<Stat::StatType>(m_trackType),
                    static_cast<Stat::ComputeFlags>(m_trackFlags), value);
    }
}

void ControlDoublePrivate::setBehavior(ControlNumericBehavior* pBehavior) {
    // This marks the old mpBehaviour for deletion. It is deleted once it is not
    // used in any other function
    m_pBehavior = QSharedPointer<ControlNumericBehavior>(pBehavior);
}

void ControlDoublePrivate::setWidgetParameter(double dParam, QObject* pSender) {
    QSharedPointer<ControlNumericBehavior> pBehavior = m_pBehavior;
    if (pBehavior.isNull()) {
        set(dParam, pSender);
    } else {
        set(pBehavior->widgetParameterToValue(dParam), pSender);
    }
}

double ControlDoublePrivate::getWidgetParameter() const {
    QSharedPointer<ControlNumericBehavior> pBehavior = m_pBehavior;
    double value = get();
    if (!pBehavior.isNull()) {
        value = pBehavior->valueToWidgetParameter(value);
    }
    return value;
}

void ControlDoublePrivate::setMidiParameter(MidiOpCode opcode, double dParam) {
    QSharedPointer<ControlNumericBehavior> pBehavior = m_pBehavior;
    if (!pBehavior.isNull()) {
        pBehavior->setValueFromMidiParameter(opcode, dParam, this);
    } else {
        set(dParam, NULL);
    }
}

double ControlDoublePrivate::getMidiParameter() const {
    QSharedPointer<ControlNumericBehavior> pBehavior = m_pBehavior;
    double value = get();
    if (!pBehavior.isNull()) {
        value = pBehavior->valueToMidiParameter(value);
    }
    return value;
}

bool ControlDoublePrivate::connectValueChangeRequest(const QObject* receiver,
        const char* method, Qt::ConnectionType type) {
    // confirmation is only required if connect was successful
    m_confirmRequired = connect(this, SIGNAL(valueChangeRequest(double)),
                receiver, method, type);
    return m_confirmRequired;
}
