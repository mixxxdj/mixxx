#include "control/control.h"

#include "control/controlobject.h"
#include "moc_control.cpp"
#include "osc/query/oscquerydescription.h"
#include "util/mutex.h"
#include "util/stat.h"

namespace {
/// Hack to implement persistent controls. This is a pointer to the current
/// user configuration object (if one exists). In general, we do not want the
/// user configuration to be a singleton -- objects that need access to it
/// should be passed it explicitly. However, the Control system is so
/// pervasive that updating every control creation to include the
/// configuration object would be arduous.
UserSettingsPointer s_pUserConfig;

const QString statTrackingKey = QStringLiteral("control %1,%2"); // CO group,key

constexpr Stat::StatType kStatType = Stat::UNSPECIFIED;

constexpr Stat::ComputeFlags kComputeFlags = {Stat::COUNT,
        Stat::SUM,
        Stat::AVERAGE,
        Stat::SAMPLE_VARIANCE,
        Stat::MIN,
        Stat::MAX};

/// Mutex guarding access to s_qCOHash and s_qCOAliasHash.
MMutex s_qCOHashMutex;

/// Hash of ControlDoublePrivate instantiations.
QHash<ConfigKey, QWeakPointer<ControlDoublePrivate>> s_qCOHash
        GUARDED_BY(s_qCOHashMutex);

/// Hash of aliases between ConfigKeys. Solely used for looking up the first
/// alias associated with a key.
QHash<ConfigKey, ConfigKey> s_qCOAliasHash
        GUARDED_BY(s_qCOHashMutex);

OscQueryDescription s_oscQueryDecription
        GUARDED_BY(s_qCOHashMutex);

/// is used instead of a nullptr, helps to omit null checks everywhere
QWeakPointer<ControlDoublePrivate> s_pDefaultCO;
} // namespace

// TODO: re-evaluate whether this is needed.
ControlDoublePrivate::ControlDoublePrivate()
        : ControlDoublePrivate({}, nullptr, true, false, false, kDefaultValue, true){};

ControlDoublePrivate::ControlDoublePrivate(
        const ConfigKey& key,
        ControlObject* pCreatorCO,
        bool bIgnoreNops,
        bool bTrack,
        bool bPersist,
        double defaultValue,
        bool confirmRequired = false)
        : m_key(key),
          m_pBehavior(nullptr),
          m_name(QString()),
          m_description(QString()),
          m_value(defaultValue),
          m_defaultValue(defaultValue),
          m_pCreatorCO(pCreatorCO),
          m_trackingKey(bTrack ? statTrackingKey.arg(key.group, key.item) : QString()),
          m_confirmRequired(confirmRequired),
          m_bPersistInConfiguration(bPersist),
          m_bIgnoreNops(bIgnoreNops),
          m_kbdRepeatable(false) {
    if (bPersist) {
        UserSettingsPointer pConfig = s_pUserConfig;
        if (pConfig) {
            m_value.setValue(pConfig->getValue(m_key, defaultValue));
        } else {
            DEBUG_ASSERT(!"Can't load persistent value s_pUserConfig is null");
        }
    }

    if (!m_trackingKey.isNull()) {
        Stat::track(m_trackingKey, kStatType, kComputeFlags, m_value.getValue());
    }
}

ControlDoublePrivate::~ControlDoublePrivate() {
    s_qCOHashMutex.lock();
    //qDebug() << "ControlDoublePrivate::s_qCOHash.remove(" << m_key.group << "," << m_key.item << ")";
    s_oscQueryDecription.removeControlKey(m_key);
    s_qCOHash.remove(m_key);
    s_qCOHashMutex.unlock();

    if (m_bPersistInConfiguration) {
        UserSettingsPointer pConfig = s_pUserConfig;
        VERIFY_OR_DEBUG_ASSERT(pConfig) {
            return;
        }
        pConfig->set(m_key, QString::number(get()));
    }
}

//static
void ControlDoublePrivate::setUserConfig(const UserSettingsPointer& pConfig) {
    DEBUG_ASSERT(pConfig != s_pUserConfig);
    s_pUserConfig = pConfig;
}

// static
void ControlDoublePrivate::insertAlias(const ConfigKey& alias, const ConfigKey& key) {
    MMutexLocker locker(&s_qCOHashMutex);
    VERIFY_OR_DEBUG_ASSERT(alias != key) {
        qWarning() << "cannot create alias with identical key" << key;
        return;
    }

    auto it = s_qCOHash.constFind(key);
    VERIFY_OR_DEBUG_ASSERT(it != s_qCOHash.constEnd()) {
        qWarning() << "cannot create alias for null control" << key;
        return;
    }

    QSharedPointer<ControlDoublePrivate> pControl = it.value();
    VERIFY_OR_DEBUG_ASSERT(!pControl.isNull()) {
        qWarning() << "cannot create alias for expired control" << key;
        return;
    }

    s_qCOAliasHash.insert(key, alias);
    s_qCOHash.insert(alias, pControl);
}

// static
QSharedPointer<ControlDoublePrivate> ControlDoublePrivate::getControl(
        const ConfigKey& key,
        ControlFlags flags,
        ControlObject* pCreatorCO,
        bool bIgnoreNops,
        bool bTrack,
        bool bPersist,
        double defaultValue) {
    if (!key.isValid()) {
        if (!flags.testFlag(ControlFlag::AllowInvalidKey)) {
            qWarning() << "ControlDoublePrivate::getControl returning nullptr"
                       << "for invalid ConfigKey" << key;
            DEBUG_ASSERT(!"Unexpected invalid key");
        }
        return nullptr;
    }

    // Scope for MMutexLocker.
    {
        const MMutexLocker locker(&s_qCOHashMutex);
        const auto it = s_qCOHash.constFind(key);
        if (it != s_qCOHash.constEnd()) {
            auto pControl = it.value().lock();
            if (pControl) {
                auto actualKey = pControl->getKey();
                if (actualKey != key) {
                    qWarning()
                            << "ControlObject accessed via deprecated key"
                            << key.group << key.item
                            << "- use"
                            << actualKey.group << actualKey.item
                            << "instead";
                }

                // Control object already exists
                if (pCreatorCO) {
                    qWarning()
                            << "ControlObject"
                            << key.group << key.item
                            << "already created";
                    DEBUG_ASSERT(!"pCreatorCO != nullptr, ControlObject already created");
                    return nullptr;
                }
                return pControl;
            } else {
                // The weak pointer has become invalid and can be cleaned up
                s_qCOHash.erase(it);
            }
        }
    }

    if (pCreatorCO) {
        auto pControl = QSharedPointer<ControlDoublePrivate>(
                new ControlDoublePrivate(key,
                        pCreatorCO,
                        bIgnoreNops,
                        bTrack,
                        bPersist,
                        defaultValue));
        const MMutexLocker locker(&s_qCOHashMutex);
        //qDebug() << "ControlDoublePrivate::s_qCOHash.insert(" << key.group << "," << key.item << ")";
        s_qCOHash.insert(key, pControl);
        s_oscQueryDecription.insertControlKey(key);
        return pControl;
    }

    if (!flags.testFlag(ControlFlag::NoWarnIfMissing)) {
        qWarning() << "ControlDoublePrivate::getControl returning NULL for ("
                   << key.group << "," << key.item << ")";
        DEBUG_ASSERT(flags.testFlag(ControlFlag::NoAssertIfMissing));
    }
    return nullptr;
}

//static
QSharedPointer<ControlDoublePrivate> ControlDoublePrivate::getDefaultControl() {
    auto defaultCO = s_pDefaultCO.lock();
    if (!defaultCO) {
        // Try again with the mutex locked to protect against creating two
        // ControlDoublePrivateConst objects. Access to s_defaultCO itself is
        // thread save.
        MMutexLocker locker(&s_qCOHashMutex);
        defaultCO = s_pDefaultCO.lock();
        if (!defaultCO) {
            defaultCO = QSharedPointer<ControlDoublePrivate>(new ControlDoublePrivateConst());
            s_pDefaultCO = defaultCO;
        }
    }
    return defaultCO;
}

// static
void ControlDoublePrivate::writeOscQueryDescription(QString& oscQueryFileName) {
    const MMutexLocker locker(&s_qCOHashMutex);
    s_oscQueryDecription.saveToFile(oscQueryFileName);
}

// static
QList<QSharedPointer<ControlDoublePrivate>> ControlDoublePrivate::getAllInstances() {
    QList<QSharedPointer<ControlDoublePrivate>> result;
    MMutexLocker locker(&s_qCOHashMutex);
    result.reserve(s_qCOHash.size());
    for (auto it = s_qCOHash.constBegin(); it != s_qCOHash.constEnd(); ++it) {
        auto pControl = it.value().lock();
        if (pControl) {
            result.append(std::move(pControl));
        } else {
            // The weak pointer has become invalid and can be cleaned up
            s_qCOHash.erase(it);
        }
    }
    return result;
}

// static
QList<QSharedPointer<ControlDoublePrivate>> ControlDoublePrivate::takeAllInstances() {
    QList<QSharedPointer<ControlDoublePrivate>> result;
    MMutexLocker locker(&s_qCOHashMutex);
    result.reserve(s_qCOHash.size());
    for (auto it = s_qCOHash.begin(); it != s_qCOHash.end(); ++it) {
        auto pControl = it.value().lock();
        if (pControl) {
            result.append(std::move(pControl));
        }
    }
    s_qCOHash.clear();
    return result;
}

//static
QHash<ConfigKey, ConfigKey> ControlDoublePrivate::getControlAliases() {
    MMutexLocker locker(&s_qCOHashMutex);
    // lock thread-unsafe copy constructors of QHash
    return s_qCOAliasHash;
}

void ControlDoublePrivate::deleteCreatorCO() {
    delete m_pCreatorCO.fetchAndStoreOrdered(nullptr);
}

void ControlDoublePrivate::reset() {
    double defaultValue = m_defaultValue.getValue();
    // NOTE: pSender = NULL is important. The originator of this action does
    // not know the resulting value so it makes sense that we should emit a
    // general valueChanged() signal even though we know the originator.
    set(defaultValue, nullptr);
}

void ControlDoublePrivate::set(double value, QObject* pSender) {
    // If the behavior says to ignore the set, ignore it.
    QSharedPointer<ControlNumericBehavior> pBehavior = m_pBehavior;
    if (!pBehavior.isNull() && !pBehavior->setFilter(&value)) {
        return;
    }
    if (m_confirmRequired) {
        emit valueChangeRequest(value);
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
    emit valueChanged(value, pSender);

    if (!m_trackingKey.isNull()) {
        Stat::track(m_trackingKey, kStatType, kComputeFlags, value);
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
    if (!pBehavior) {
        qWarning() << "Cannot get" << m_key << "for Midi";
        DEBUG_ASSERT(!"pBehavior == nullptr, getParameterForMidi is returning 0");
        return 0;
    }
    return pBehavior->midiToParameter(midiParam);
}

void ControlDoublePrivate::setValueFromMidi(MidiOpCode opcode, double midiParam) {
    QSharedPointer<ControlNumericBehavior> pBehavior = m_pBehavior;
    if (!pBehavior) {
        qWarning() << "Cannot set" << m_key << "from Midi";
        DEBUG_ASSERT(!"pBehavior == nullptr, abort setValueFromMidi()");
        return;
    }
    pBehavior->setValueFromMidi(opcode, midiParam, this);
}

double ControlDoublePrivate::getMidiParameter() const {
    QSharedPointer<ControlNumericBehavior> pBehavior = m_pBehavior;
    if (!pBehavior) {
        qWarning() << "Cannot get" << m_key << "as Midi";
        DEBUG_ASSERT(!"pBehavior == nullptr, getMidiParameter() is returning 0");
        return 0;
    }
    return pBehavior->valueToMidiParameter(get());
}
