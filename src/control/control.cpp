#include <QtDebug>
#include <QSharedPointer>

#include "control/control.h"

#include "util/stat.h"

// Static member variable definition
UserSettingsPointer ControlDoublePrivate::s_pUserConfig;

QHash<ConfigKey, QWeakPointer<ControlDoublePrivate> > ControlDoublePrivate::s_qCOHash
GUARDED_BY(ControlDoublePrivate::s_qCOHashMutex);

QHash<ConfigKey, ConfigKey> ControlDoublePrivate::s_qCOAliasHash
GUARDED_BY(ControlDoublePrivate::s_qCOHashMutex);

MMutex ControlDoublePrivate::s_qCOHashMutex;

/*
ControlDoublePrivate::ControlDoublePrivate()
        : m_bIgnoreNops(true),
          m_bTrack(false),
          m_trackType(Stat::UNSPECIFIED),
          m_trackFlags(Stat::COUNT | Stat::SUM | Stat::AVERAGE |
                       Stat::SAMPLE_VARIANCE | Stat::MIN | Stat::MAX),
          m_confirmRequired(false) {
    initialize();
}
*/

ControlDoublePrivate::ControlDoublePrivate(ConfigKey key,
                                           ControlObject* pCreatorCO,
                                           bool bIgnoreNops, bool bTrack,
                                           bool bPersist, double defaultValue)
        : m_key(key),
          m_bPersistInConfiguration(bPersist),
          m_bIgnoreNops(bIgnoreNops),
          m_bTrack(bTrack),
          m_trackType(Stat::UNSPECIFIED),
          m_trackFlags(Stat::COUNT | Stat::SUM | Stat::AVERAGE |
                       Stat::SAMPLE_VARIANCE | Stat::MIN | Stat::MAX),
          m_confirmRequired(false),
          m_pCreatorCO(pCreatorCO) {
    initialize(defaultValue);
}

void ControlDoublePrivate::initialize(double defaultValue) {
    double value = defaultValue;
    if (m_bPersistInConfiguration) {
        UserSettingsPointer pConfig = ControlDoublePrivate::s_pUserConfig;
        if (pConfig != nullptr) {
            value = pConfig->getValue(m_key, defaultValue);
        }
    }
    m_defaultValue.setValue(defaultValue);
    m_value.setValue(value);

    //qDebug() << "Creating:" << m_trackKey << "at" << &m_value << sizeof(m_value);

    if (m_bTrack) {
        // TODO(rryan): Make configurable.
        m_trackKey = "control " + m_key.group + "," + m_key.item;
        Stat::track(m_trackKey, static_cast<Stat::StatType>(m_trackType),
                    static_cast<Stat::ComputeFlags>(m_trackFlags),
                    m_value.getValue());
    }
}

ControlDoublePrivate::~ControlDoublePrivate() {
    s_qCOHashMutex.lock();
    //qDebug() << "ControlDoublePrivate::s_qCOHash.remove(" << m_key.group << "," << m_key.item << ")";
    s_qCOHash.remove(m_key);
    s_qCOHashMutex.unlock();

    if (m_bPersistInConfiguration) {
        UserSettingsPointer pConfig = ControlDoublePrivate::s_pUserConfig;
        if (pConfig != NULL) {
            pConfig->set(m_key, QString::number(get()));
        }
    }
}

// static
void ControlDoublePrivate::insertAlias(const ConfigKey& alias, const ConfigKey& key) {
    MMutexLocker locker(&s_qCOHashMutex);

    auto it = s_qCOHash.constFind(key);
    if (it == s_qCOHash.constEnd()) {
        qWarning() << "WARNING: ControlDoublePrivate::insertAlias called for null control" << key;
        return;
    }

    QSharedPointer<ControlDoublePrivate> pControl = it.value();
    if (pControl.isNull()) {
        qWarning() << "WARNING: ControlDoublePrivate::insertAlias called for expired control" << key;
        return;
    }

    s_qCOAliasHash.insert(key, alias);
    s_qCOHash.insert(alias, pControl);
}

// static
QSharedPointer<ControlDoublePrivate> ControlDoublePrivate::getControl(
        const ConfigKey& key, bool warn, ControlObject* pCreatorCO,
        bool bIgnoreNops, bool bTrack, bool bPersist, double defaultValue) {
    if (key.isEmpty()) {
        if (warn) {
            qWarning() << "ControlDoublePrivate::getControl returning NULL"
                       << "for empty ConfigKey.";
        }
        return QSharedPointer<ControlDoublePrivate>();
    }


    QSharedPointer<ControlDoublePrivate> pControl;
    // Scope for MMutexLocker.
    {
        MMutexLocker locker(&s_qCOHashMutex);
        auto it = s_qCOHash.constFind(key);
        if (it != s_qCOHash.constEnd()) {
            if (pCreatorCO) {
                if (warn) {
                    qDebug() << "ControlObject" << key.group << key.item << "already created";
                }
            } else {
                pControl = it.value();
            }
        }
    }

    if (pControl == NULL) {
        if (pCreatorCO) {
            pControl = QSharedPointer<ControlDoublePrivate>(
                    new ControlDoublePrivate(key, pCreatorCO, bIgnoreNops,
                                             bTrack, bPersist, defaultValue));
            MMutexLocker locker(&s_qCOHashMutex);
            //qDebug() << "ControlDoublePrivate::s_qCOHash.insert(" << key.group << "," << key.item << ")";
            s_qCOHash.insert(key, pControl);
        } else if (warn) {
            qWarning() << "ControlDoublePrivate::getControl returning NULL for ("
                       << key.group << "," << key.item << ")";
        }
    }
    return pControl;
}

// static
void ControlDoublePrivate::getControls(
        QList<QSharedPointer<ControlDoublePrivate> >* pControlList) {
    s_qCOHashMutex.lock();
    pControlList->clear();
    for (auto it = s_qCOHash.constBegin(); it != s_qCOHash.constEnd(); ++it) {
        QSharedPointer<ControlDoublePrivate> pControl = it.value();
        if (!pControl.isNull()) {
            pControlList->push_back(pControl);
        }
    }
    s_qCOHashMutex.unlock();
}

// static
QHash<ConfigKey, ConfigKey> ControlDoublePrivate::getControlAliases() {
    MMutexLocker locker(&s_qCOHashMutex);
    return s_qCOAliasHash;
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
    if (m_confirmRequired) {
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
    // This marks the old mpBehavior for deletion. It is deleted once it is not
    // used in any other function
    m_pBehavior = QSharedPointer<ControlNumericBehavior>(pBehavior);
}

void ControlDoublePrivate::setParameter(double dParam, QObject* pSender) {
    QSharedPointer<ControlNumericBehavior> pBehavior = m_pBehavior;
    if (pBehavior.isNull()) {
        set(dParam, pSender);
    } else {
        set(pBehavior->parameterToValue(dParam), pSender);
    }
}

double ControlDoublePrivate::getParameter() const {
    return getParameterForValue(get());
}

double ControlDoublePrivate::getParameterForValue(double value) const {
    QSharedPointer<ControlNumericBehavior> pBehavior = m_pBehavior;
    if (!pBehavior.isNull()) {
        value = pBehavior->valueToParameter(value);
    }
    return value;
}

double ControlDoublePrivate::getParameterForMidi(double midiParam) const {
    QSharedPointer<ControlNumericBehavior> pBehavior = m_pBehavior;
    VERIFY_OR_DEBUG_ASSERT(pBehavior) {
        qWarning() << "Cannot set" << m_key << "by Midi";
        return 0;
    }
    return pBehavior->midiToParameter(midiParam);
}

void ControlDoublePrivate::setValueFromMidi(MidiOpCode opcode, double midiParam) {
    QSharedPointer<ControlNumericBehavior> pBehavior = m_pBehavior;
    VERIFY_OR_DEBUG_ASSERT(pBehavior) {
        qWarning() << "Cannot set" << m_key << "by Midi";
        return;
    }
    pBehavior->setValueFromMidi(opcode, midiParam, this);
}

double ControlDoublePrivate::getMidiParameter() const {
    QSharedPointer<ControlNumericBehavior> pBehavior = m_pBehavior;
    VERIFY_OR_DEBUG_ASSERT(pBehavior) {
        qWarning() << "Cannot get" << m_key << "by Midi";
        return 0;
    }
    return pBehavior->valueToMidiParameter(get());
}
