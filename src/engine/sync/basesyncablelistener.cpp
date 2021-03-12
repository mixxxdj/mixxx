#include "engine/sync/basesyncablelistener.h"

#include <QMetaType>

#include "engine/sync/internalclock.h"

namespace {

const QString kInternalClockGroup = QStringLiteral("[InternalClock]");

} // anonymous namespace

BaseSyncableListener::BaseSyncableListener(UserSettingsPointer pConfig)
        : m_pConfig(pConfig),
          m_pInternalClock(new InternalClock(kInternalClockGroup, this)),
          m_pMasterSyncable(nullptr) {
    qRegisterMetaType<SyncMode>("SyncMode");
    m_pInternalClock->setMasterBpm(124.0);
}

BaseSyncableListener::~BaseSyncableListener() {
    // We use the slider value because that is never set to 0.0.
    m_pConfig->set(ConfigKey("[InternalClock]", "bpm"), ConfigValue(
        m_pInternalClock->getBpm()));
    delete m_pInternalClock;
}

void BaseSyncableListener::addSyncableDeck(Syncable* pSyncable) {
    if (m_syncables.contains(pSyncable)) {
        qDebug() << "BaseSyncableListener: already has" << pSyncable;
        return;
    }
    m_syncables.append(pSyncable);
}

void BaseSyncableListener::onCallbackStart(int sampleRate, int bufferSize) {
    m_pInternalClock->onCallbackStart(sampleRate, bufferSize);
}

void BaseSyncableListener::onCallbackEnd(int sampleRate, int bufferSize) {
    m_pInternalClock->onCallbackEnd(sampleRate, bufferSize);
}

EngineChannel* BaseSyncableListener::getMaster() const {
    return m_pMasterSyncable ? m_pMasterSyncable->getChannel() : nullptr;
}

Syncable* BaseSyncableListener::getSyncableForGroup(const QString& group) {
    foreach (Syncable* pSyncable, m_syncables) {
        if (pSyncable->getGroup() == group) {
            return pSyncable;
        }
    }
    return nullptr;
}

bool BaseSyncableListener::syncDeckExists() const {
    for (const auto& pSyncable : qAsConst(m_syncables)) {
        if (pSyncable->isSynchronized() && pSyncable->getBaseBpm() > 0) {
            return true;
        }
    }
    return false;
}

double BaseSyncableListener::masterBpm() const {
    if (m_pMasterSyncable) {
        return m_pMasterSyncable->getBpm();
    }
    return m_pInternalClock->getBpm();
}

double BaseSyncableListener::masterBeatDistance() const {
    if (m_pMasterSyncable) {
        return m_pMasterSyncable->getBeatDistance();
    }
    return m_pInternalClock->getBeatDistance();
}

double BaseSyncableListener::masterBaseBpm() const {
    if (m_pMasterSyncable) {
        return m_pMasterSyncable->getBaseBpm();
    }
    return m_pInternalClock->getBaseBpm();
}

void BaseSyncableListener::setMasterBpm(Syncable* pSource, double bpm) {
    //qDebug() << "BaseSyncableListener::setMasterBpm" << pSource << bpm;
    if (pSource != m_pInternalClock) {
        m_pInternalClock->setMasterBpm(bpm);
    }
    foreach (Syncable* pSyncable, m_syncables) {
        if (pSyncable == pSource ||
                !pSyncable->isSynchronized()) {
            continue;
        }
        pSyncable->setMasterBpm(bpm);
    }
}

void BaseSyncableListener::setMasterInstantaneousBpm(Syncable* pSource, double bpm) {
    if (pSource != m_pInternalClock) {
        m_pInternalClock->setInstantaneousBpm(bpm);
    }
    foreach (Syncable* pSyncable, m_syncables) {
        if (pSyncable == pSource ||
                !pSyncable->isSynchronized()) {
            continue;
        }
        pSyncable->setInstantaneousBpm(bpm);
    }
}

void BaseSyncableListener::setMasterBeatDistance(Syncable* pSource, double beatDistance) {
    if (pSource != m_pInternalClock) {
        m_pInternalClock->setMasterBeatDistance(beatDistance);
    }
    foreach (Syncable* pSyncable, m_syncables) {
        if (pSyncable == pSource ||
                !pSyncable->isSynchronized()) {
            continue;
        }
        pSyncable->setMasterBeatDistance(beatDistance);
    }
}

void BaseSyncableListener::setMasterParams(Syncable* pSource) {
    const double beatDistance = pSource->getBeatDistance();
    const double baseBpm = pSource->getBaseBpm();
    const double bpm = pSource->getBpm() > 0 ? pSource->getBpm() : pSource->getBaseBpm();
    // qDebug() << "BaseSyncableListener::setMasterParams, source is"
    //          << pSource->getGroup() << beatDistance << baseBpm << bpm;
    if (pSource != m_pInternalClock) {
        m_pInternalClock->setMasterParams(beatDistance, baseBpm, bpm);
    }
    foreach (Syncable* pSyncable, m_syncables) {
        if (pSyncable == pSource || !pSyncable->isSynchronized()) {
            continue;
        }
        pSyncable->setMasterParams(beatDistance, baseBpm, bpm);
    }
}

Syncable* BaseSyncableListener::getUniquePlayingSyncable() {
    int playing_sync_decks = 0;
    Syncable* unique_syncable = nullptr;
    foreach (Syncable* pSyncable, m_syncables) {
        if (!pSyncable->isSynchronized()) {
            continue;
        }

        if (pSyncable->isPlaying()) {
            if (playing_sync_decks > 0) {
                return nullptr;
            }
            unique_syncable = pSyncable;
            ++playing_sync_decks;
        }
    }
    if (playing_sync_decks == 1) {
        return unique_syncable;
    }
    return nullptr;
}
