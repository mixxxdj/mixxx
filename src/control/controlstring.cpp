#include "control/controlstring.h"

#include <QMutexLocker>

#include "moc_controlstring.cpp"
#include "util/assert.h"

namespace mixxx {

// Static member definitions
QHash<ConfigKey, QWeakPointer<ControlStringPrivate>> ControlStringPrivate::s_controlHash;
QMutex ControlStringPrivate::s_controlHashMutex;
UserSettingsPointer ControlStringPrivate::s_pUserConfig;
QSharedPointer<ControlStringPrivate> ControlStringPrivate::s_pDefaultControl;

// ControlStringPrivate Implementation

ControlStringPrivate::ControlStringPrivate(const ConfigKey& key,
        ControlString* pCreatorCO,
        bool bIgnoreNops,
        bool bPersist,
        const QString& defaultValue)
        : m_key(key),
          m_bIgnoreNops(bIgnoreNops),
          m_bPersist(bPersist),
          m_pCreatorCO(pCreatorCO) {
    initialize(defaultValue);
}

ControlStringPrivate::~ControlStringPrivate() {
    if (m_bPersist && s_pUserConfig && m_key.isValid()) {
        s_pUserConfig->setValue(m_key, m_value);
    }

    {
        QMutexLocker locker(&s_controlHashMutex);
        s_controlHash.remove(m_key);
    }
}

void ControlStringPrivate::initialize(const QString& defaultValue) {
    m_defaultValue = defaultValue;
    m_value = defaultValue;

    // Load persisted value if available
    if (m_bPersist && s_pUserConfig && m_key.isValid()) {
        QString configValue = s_pUserConfig->getValue(m_key, defaultValue);
        if (!configValue.isNull()) {
            m_value = configValue;
        }
    }

    connect(this, &ControlStringPrivate::valueChangeRequest, this, &ControlStringPrivate::slotValueChangeRequest, Qt::DirectConnection);
}

// static
void ControlStringPrivate::setUserConfig(const UserSettingsPointer& pConfig) {
    s_pUserConfig = pConfig;
}

// static
QSharedPointer<ControlStringPrivate> ControlStringPrivate::getControl(
        const ConfigKey& key,
        ControlFlags flags,
        ControlString* pCreatorCO,
        bool bIgnoreNops,
        bool bPersist,
        const QString& defaultValue) {
    if (!key.isValid()) {
        if (flags & ControlFlag::AllowInvalidKey) {
            return getDefaultControl();
        }
        DEBUG_ASSERT(!"Invalid ConfigKey");
        return nullptr;
    }

    QSharedPointer<ControlStringPrivate> pControl;

    {
        QMutexLocker locker(&s_controlHashMutex);
        auto it = s_controlHash.find(key);
        if (it != s_controlHash.end()) {
            pControl = it.value().toStrongRef();
            if (!pControl) {
                // Weak pointer expired, remove it
                s_controlHash.erase(it);
            }
        }
    }

    if (!pControl && pCreatorCO) {
        // Create new control
        pControl = QSharedPointer<ControlStringPrivate>(
                new ControlStringPrivate(key, pCreatorCO, bIgnoreNops, bPersist, defaultValue));
        QMutexLocker locker(&s_controlHashMutex);
        s_controlHash.insert(key, pControl);
    }

    if (!pControl) {
        if (flags & ControlFlag::NoWarnIfMissing) {
            return nullptr;
        }
        qWarning() << "ControlStringPrivate::getControl"
                   << "Failed to get control for key:" << key.group << key.item;
        DEBUG_ASSERT(flags & ControlFlag::NoAssertIfMissing);
        return nullptr;
    }

    return pControl;
}

// static
QSharedPointer<ControlStringPrivate> ControlStringPrivate::getDefaultControl() {
    if (!s_pDefaultControl) {
        ConfigKey key;
        s_pDefaultControl = QSharedPointer<ControlStringPrivate>(
                new ControlStringPrivate(key, nullptr, true, false, QString()));
    }
    return s_pDefaultControl;
}

// static
QList<QSharedPointer<ControlStringPrivate>> ControlStringPrivate::getAllInstances() {
    QMutexLocker locker(&s_controlHashMutex);
    QList<QSharedPointer<ControlStringPrivate>> result;

    for (auto it = s_controlHash.begin(); it != s_controlHash.end(); ++it) {
        auto pControl = it.value().toStrongRef();
        if (pControl) {
            result.append(pControl);
        }
    }

    return result;
}

// static
QList<QSharedPointer<ControlStringPrivate>> ControlStringPrivate::takeAllInstances() {
    QMutexLocker locker(&s_controlHashMutex);
    QList<QSharedPointer<ControlStringPrivate>> result;

    for (auto it = s_controlHash.begin(); it != s_controlHash.end(); ++it) {
        auto pControl = it.value().toStrongRef();
        if (pControl) {
            result.append(pControl);
        }
    }

    s_controlHash.clear();
    return result;
}

void ControlStringPrivate::set(const QString& value, QObject* pSender) {
    Q_UNUSED(pSender);
    if (m_bIgnoreNops && value == get()) {
        return;
    }

    emit valueChangeRequest(value);
}

void ControlStringPrivate::setAndConfirm(const QString& value, QObject* pSender) {
    {
        QMutexLocker locker(&m_mutex);
        if (m_bIgnoreNops && value == m_value) {
            return;
        }
        m_value = value;
    }

    emit valueChanged(value, pSender);
}

QString ControlStringPrivate::get() const {
    QMutexLocker locker(&m_mutex);
    return m_value;
}

void ControlStringPrivate::reset() {
    setAndConfirm(m_defaultValue, nullptr);
}

void ControlStringPrivate::setDefaultValue(const QString& value) {
    QMutexLocker locker(&m_mutex);
    m_defaultValue = value;
}

QString ControlStringPrivate::defaultValue() const {
    QMutexLocker locker(&m_mutex);
    return m_defaultValue;
}

ControlString* ControlStringPrivate::getCreatorCO() const {
    return m_pCreatorCO.loadAcquire();
}

bool ControlStringPrivate::resetCreatorCO(ControlString* pCreatorCO) {
    return m_pCreatorCO.testAndSetOrdered(pCreatorCO, nullptr);
}

void ControlStringPrivate::deleteCreatorCO() {
    ControlString* pCreatorCO = m_pCreatorCO.fetchAndStoreOrdered(nullptr);
    delete pCreatorCO;
}

void ControlStringPrivate::slotValueChangeRequest(const QString& value) {
    setAndConfirm(value, nullptr);
}

// ControlString Implementation

ControlString::ControlString()
        : m_pControl(ControlStringPrivate::getDefaultControl()) {
}

ControlString::ControlString(const ConfigKey& key,
        bool bIgnoreNops,
        bool bPersist,
        const QString& defaultValue)
        : m_key(key) {
    // Don't bother looking up the control if key is invalid. Prevents log spew.
    if (m_key.isValid()) {
        m_pControl = ControlStringPrivate::getControl(m_key,
                ControlFlag::None,
                this,
                bIgnoreNops,
                bPersist,
                defaultValue);
    }

    // getControl can fail and return a NULL control even with the create flag.
    if (m_pControl) {
        connect(m_pControl.data(),
                &ControlStringPrivate::valueChanged,
                this,
                &ControlString::privateValueChanged,
                Qt::DirectConnection);
    } else {
        m_pControl = ControlStringPrivate::getDefaultControl();
    }
}

ControlString::~ControlString() {
    DEBUG_ASSERT(m_pControl);
    const bool success = m_pControl->resetCreatorCO(this);
    Q_UNUSED(success);
    DEBUG_ASSERT(success);
}

// slot
void ControlString::privateValueChanged(const QString& value, QObject* pSender) {
    // Only emit valueChanged() if we did not originate this change.
    if (pSender != this) {
        emit valueChanged(value);
    }
}

// static
ControlString* ControlString::getControl(const ConfigKey& key, ControlFlags flags) {
    QSharedPointer<ControlStringPrivate> pCSP = ControlStringPrivate::getControl(key, flags);
    if (pCSP) {
        return pCSP->getCreatorCO();
    }
    return nullptr;
}

bool ControlString::exists(const ConfigKey& key) {
    return !ControlStringPrivate::getControl(key, ControlFlag::NoWarnIfMissing).isNull();
}

// static
QString ControlString::get(const ConfigKey& key) {
    QSharedPointer<ControlStringPrivate> pControl = ControlStringPrivate::getControl(key);
    return pControl ? pControl->get() : QString();
}

// static
void ControlString::set(const ConfigKey& key, const QString& value) {
    QSharedPointer<ControlStringPrivate> pControl = ControlStringPrivate::getControl(key);
    if (pControl) {
        pControl->set(value, nullptr);
    }
}

void ControlString::setReadOnly() {
    connectValueChangeRequest(this, &ControlString::readOnlyHandler);
}

void ControlString::readOnlyHandler(const QString& v) {
    // Ignore all value change requests
    Q_UNUSED(v);
}

} // namespace mixxx