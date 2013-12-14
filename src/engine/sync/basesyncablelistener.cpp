#include "engine/sync/basesyncablelistener.h"

#include "engine/sync/internalclock.h"

static const char* kInternalClockGroup = "[InternalClock]";

BaseSyncableListener::BaseSyncableListener(ConfigObject<ConfigValue>* pConfig)
        : m_pConfig(pConfig),
          m_pInternalClock(new InternalClock(kInternalClockGroup, this)),
          m_pMasterSyncable(NULL) {
    qRegisterMetaType<SyncMode>("SyncMode");
    m_pInternalClock->setBpm(124.0);
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

int BaseSyncableListener::playingSyncDeckCount() const {
    int playing_sync_decks = 0;

    foreach (const Syncable* pSyncable, m_syncables) {
        SyncMode sync_mode = pSyncable->getSyncMode();
        if (sync_mode == SYNC_NONE) {
            continue;
        }

        if (pSyncable->isPlaying()) {
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

void BaseSyncableListener::setMasterBpm(Syncable* pSource, double bpm) {
    if (pSource != m_pInternalClock) {
        m_pInternalClock->setBpm(bpm);
    }
    foreach (Syncable* pSyncable, m_syncables) {
        if (pSyncable == pSource ||
                pSyncable->getSyncMode() == SYNC_NONE) {
            continue;
        }
        pSyncable->setBpm(bpm);
    }
}

void BaseSyncableListener::setMasterInstantaneousBpm(Syncable* pSource, double bpm) {
    if (pSource != m_pInternalClock) {
        m_pInternalClock->setInstantaneousBpm(bpm);
    }
    foreach (Syncable* pSyncable, m_syncables) {
        if (pSyncable == pSource ||
                pSyncable->getSyncMode() == SYNC_NONE) {
            continue;
        }
        pSyncable->setInstantaneousBpm(bpm);
    }
}

void BaseSyncableListener::setMasterBeatDistance(Syncable* pSource, double beat_distance) {
    if (pSource != m_pInternalClock) {
        m_pInternalClock->setBeatDistance(beat_distance);
    }
    foreach (Syncable* pSyncable, m_syncables) {
        if (pSyncable == pSource ||
                pSyncable->getSyncMode() == SYNC_NONE) {
            continue;
        }
        pSyncable->setBeatDistance(beat_distance);
    }
}
