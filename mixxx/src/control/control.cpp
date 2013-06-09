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
          m_bTrack(false) {
    m_defaultValue.setValue(0);
    m_value.setValue(0);
}

ControlDoublePrivate::ControlDoublePrivate(ConfigKey key,
                                           bool bIgnoreNops, bool bTrack)
        : m_key(key),
          m_bIgnoreNops(bIgnoreNops),
          m_bTrack(bTrack),
          m_trackKey("control " + m_key.group + "," + m_key.item),
          m_trackType(Stat::UNSPECIFIED),
          m_trackFlags(Stat::COUNT | Stat::SUM | Stat::AVERAGE |
                       Stat::SAMPLE_VARIANCE | Stat::MIN | Stat::MAX) {
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

void ControlDoublePrivate::reset(QObject* pSender) {
    double defaultValue = m_defaultValue.getValue();
    set(defaultValue, pSender);
}

void ControlDoublePrivate::set(const double& value, QObject* pSender) {
    if (m_bIgnoreNops && get() == value) {
        return;
    }

    double dValue = value;
    // If the behavior says to ignore the set, ignore it.
    ControlNumericBehavior* pBehavior = m_pBehavior;
    if (pBehavior && !pBehavior->setFilter(&dValue)) {
        return;
    }
    m_value.setValue(dValue);
    emit(valueChanged(dValue, pSender));

    if (m_bTrack) {
        Stat::track(m_trackKey, static_cast<Stat::StatType>(m_trackType),
                    static_cast<Stat::ComputeFlags>(m_trackFlags), dValue);
    }
}

ControlNumericBehavior* ControlDoublePrivate::setBehavior(ControlNumericBehavior* pBehavior) {
    return m_pBehavior.fetchAndStoreRelaxed(pBehavior);
}

void ControlDoublePrivate::setWidgetParameter(double dParam, QObject* pSetter) {
    ControlNumericBehavior* pBehavior = m_pBehavior;
    set(pBehavior ? pBehavior->widgetParameterToValue(dParam) : dParam, pSetter);
}

double ControlDoublePrivate::getWidgetParameter() const {
    ControlNumericBehavior* pBehavior = m_pBehavior;
    return pBehavior ? pBehavior->valueToWidgetParameter(get()) : get();
}

void ControlDoublePrivate::setMidiParameter(MidiOpCode opcode, double dParam) {
    ControlNumericBehavior* pBehavior = m_pBehavior;
    if (pBehavior) {
        pBehavior->setValueFromMidiParameter(opcode, dParam, this);
    } else {
        set(dParam, NULL);
    }
}

double ControlDoublePrivate::getMidiParameter() const {
    ControlNumericBehavior* pBehavior = m_pBehavior;
    return pBehavior ? pBehavior->valueToMidiParameter(get()) : get();
}

