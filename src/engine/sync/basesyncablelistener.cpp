#include "engine/sync/basesyncablelistener.h"

#include <QMetaType>

#include "engine/sync/internalclock.h"

static const char* kInternalClockGroup = "[InternalClock]";

BaseSyncableListener::BaseSyncableListener(UserSettingsPointer pConfig)
        : m_pConfig(pConfig),
          m_pInternalClock(new InternalClock(kInternalClockGroup, this)),
          m_pMasterSyncable(NULL) {
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
    return m_pMasterSyncable ? m_pMasterSyncable->getChannel() : NULL;
}

Syncable* BaseSyncableListener::getSyncableForGroup(const QString& group) {
    foreach (Syncable* pSyncable, m_syncables) {
        if (pSyncable->getGroup() == group) {
            return pSyncable;
        }
    }
    return NULL;
}

bool BaseSyncableListener::syncDeckExists() const {
    foreach (const Syncable* pSyncable, m_syncables) {
        if (pSyncable->isSynchronized() && pSyncable->getBaseBpm() > 0) {
            return true;
        }
    }
    return false;
}

int BaseSyncableListener::playingSyncDeckCount() const {
    int playing_sync_decks = 0;

    foreach (const Syncable* pSyncable, m_syncables) {
        if (pSyncable->isSynchronized() && pSyncable->isPlaying()) {
            ++playing_sync_decks;
        }
    }

    return playing_sync_decks;
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
    qDebug() << "BaseSyncableListener::setMasterBpm" << pSource << bpm;
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

void BaseSyncableListener::setMasterBaseBpm(Syncable* pSource, double bpm) {
    if (pSource != m_pInternalClock) {
        m_pInternalClock->setMasterBaseBpm(bpm);
    }
    foreach (Syncable* pSyncable, m_syncables) {
        if (pSyncable == pSource ||
                !pSyncable->isSynchronized()) {
            continue;
        }
        pSyncable->setMasterBaseBpm(bpm);
    }
}

void BaseSyncableListener::setMasterBeatDistance(Syncable* pSource, double beat_distance) {
    if (pSource != m_pInternalClock) {
        m_pInternalClock->setMasterBeatDistance(beat_distance);
    }
    foreach (Syncable* pSyncable, m_syncables) {
        if (pSyncable == pSource ||
                !pSyncable->isSynchronized()) {
            continue;
        }
        pSyncable->setMasterBeatDistance(beat_distance);
    }
}

void BaseSyncableListener::setMasterParams(Syncable* pSource, double beat_distance,
                                           double base_bpm, double bpm) {
    if (pSource != m_pInternalClock) {
        m_pInternalClock->setMasterParams(beat_distance, base_bpm, bpm);
    }
    foreach (Syncable* pSyncable, m_syncables) {
        if (pSyncable == pSource ||
                !pSyncable->isSynchronized()) {
            continue;
        }
        pSyncable->setMasterParams(beat_distance, base_bpm, bpm);
    }
}

void BaseSyncableListener::checkUniquePlayingSyncable() {
    int playing_sync_decks = 0;
    Syncable* unique_syncable = NULL;
    foreach (Syncable* pSyncable, m_syncables) {
        if (!pSyncable->isSynchronized()) {
            continue;
        }

        if (pSyncable->isPlaying()) {
            if (playing_sync_decks > 0) {
                return;
            }
            unique_syncable = pSyncable;
            ++playing_sync_decks;
        }
    }
    if (playing_sync_decks == 1) {
        unique_syncable->notifyOnlyPlayingSyncable();
    }
}
