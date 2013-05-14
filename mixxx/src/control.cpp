#include <QtDebug>
#include <QMutexLocker>

#include "control.h"

#include "util/stat.h"
#include "util/timer.h"

// Static member variable definition
QHash<ConfigKey, ControlNumericPrivate*> ControlNumericPrivate::m_sqCOHash;
QMutex ControlNumericPrivate::m_sqCOHashMutex;

ControlNumericPrivate::ControlNumericPrivate()
        : m_bIgnoreNops(true),
          m_bTrack(false) {
    m_defaultValue.setValue(0);
    m_value.setValue(0);
}

ControlNumericPrivate::ControlNumericPrivate(ConfigKey key,
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

ControlNumericPrivate::~ControlNumericPrivate() {
    m_sqCOHashMutex.lock();
    m_sqCOHash.remove(m_key);
    m_sqCOHashMutex.unlock();
}

// static
ControlNumericPrivate* ControlNumericPrivate::getControl(
    const ConfigKey& key, bool bCreate, bool bIgnoreNops, bool bTrack) {
    QMutexLocker locker(&m_sqCOHashMutex);
    QHash<ConfigKey, ControlNumericPrivate*>::const_iterator it = m_sqCOHash.find(key);
    if (it != m_sqCOHash.end()) {
        return it.value();
    }
    locker.unlock();

    ControlNumericPrivate* pControl = NULL;
    if (bCreate) {
        pControl = new ControlNumericPrivate(key, bIgnoreNops, bTrack);
        locker.relock();
        m_sqCOHash.insert(key, pControl);
        locker.unlock();
    }

    if (pControl == NULL) {
        qWarning() << "ControlNumericPrivate::getControl returning NULL for ("
                   << key.group << "," << key.item << ")";
    }

    return pControl;
}

double ControlNumericPrivate::get() const {
    return m_value.getValue();
}

void ControlNumericPrivate::reset(QObject* pSender) {
    double defaultValue = m_defaultValue.getValue();
    set(defaultValue, pSender);
}

void ControlNumericPrivate::set(const double& value, QObject* pSender) {
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

void ControlNumericPrivate::add(double dValue, QObject* pSender) {
    if (m_bIgnoreNops && !dValue) {
        return;
    }
    set(get() + dValue, pSender);
}

void ControlNumericPrivate::sub(double dValue, QObject* pSender) {
    if (m_bIgnoreNops && !dValue) {
        return;
    }
    set(get() + dValue, pSender);
}


