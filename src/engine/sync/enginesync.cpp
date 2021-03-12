#include "engine/sync/enginesync.h"

#include <QStringList>

#include "engine/channels/enginechannel.h"
#include "engine/enginebuffer.h"
#include "engine/sync/internalclock.h"
#include "util/assert.h"
#include "util/logger.h"

namespace {
const mixxx::Logger kLogger("EngineSync");
} // namespace

EngineSync::EngineSync(UserSettingsPointer pConfig)
        : BaseSyncableListener(pConfig) {
}

EngineSync::~EngineSync() {
}

void EngineSync::requestSyncMode(Syncable* pSyncable, SyncMode mode) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::requestSyncMode" << pSyncable->getGroup() << mode;
    }
    // Based on the call hierarchy I don't think this is possible. (Famous last words.)
    VERIFY_OR_DEBUG_ASSERT(pSyncable) {
        return;
    }

    // The syncable that will be used to initialize the master params, if needed.
    Syncable* pParamsSyncable = m_pMasterSyncable;

    switch (mode) {
    case SYNC_MASTER_EXPLICIT: {
        if (pSyncable->getBaseBpm() > 0 || pSyncable == m_pInternalClock) {
            activateMaster(pSyncable, SYNC_MASTER_EXPLICIT);
            pParamsSyncable = pSyncable;
        } else {
            // If we have no bpm, we can't be a valid master. Override.
            activateFollower(pSyncable);
        }
        break;
    }
    case SYNC_FOLLOWER:
    case SYNC_MASTER_SOFT: {
        // A request for follower mode may be converted into an enabling of soft
        // master mode.
        // Note: SYNC_MASTER_SOFT and SYNC_FOLLOWER cannot be set explicitly,
        // they are calculated by pickMaster.
        // Internal clock cannot be disabled, it is always listening
        if (m_pMasterSyncable == pSyncable) {
            // This Syncable was master before. Hand off.
            m_pMasterSyncable = nullptr;
            pSyncable->setSyncMode(mode);
        }

        Syncable* newMaster = pickMaster(pSyncable);
        if (newMaster && newMaster != m_pMasterSyncable) {
            // if the master has changed, activate it.
            activateMaster(newMaster, SYNC_MASTER_SOFT);
            pParamsSyncable = newMaster;
        }
        // If we are the new master, we need to match to some other deck.
        if (newMaster == pSyncable) {
            // There is no other synced deck. If any other deck is playing we will
            // match the first available bpm -- sync won't be enabled on these decks,
            // otherwise there would have been a master.
            pParamsSyncable = findBpmMatchTarget(pSyncable);
            if (!pParamsSyncable) {
                // We weren't able to find anything to match to, so set ourselves as the
                // target.  That way we'll use our own params when we setMasterParams below.
                pParamsSyncable = pSyncable;
            }
        } else {
            // This happens if this Deck has no valid BPM
            activateFollower(pSyncable);
        }
        break;
    }
    case SYNC_NONE: {
        if (pSyncable != m_pInternalClock) {
            deactivateSync(pSyncable);
        }
        break;
    }
    default:;
    }

    if (auto unique_syncable = getUniquePlayingSyncable(); unique_syncable) {
        unique_syncable->notifyOnlyPlayingSyncable();
    }

    // Now that all of the decks have their assignments, reinit master params if needed.
    if (pParamsSyncable) {
        setMasterParams(pParamsSyncable);
        pSyncable->setInstantaneousBpm(pParamsSyncable->getBpm());
        if (pParamsSyncable != pSyncable) {
            pSyncable->requestSync();
        }
    }
}

void EngineSync::requestEnableSync(Syncable* pSyncable, bool bEnabled) {
    if (bEnabled) {
        requestSyncMode(pSyncable, SYNC_FOLLOWER);
    } else {
        requestSyncMode(pSyncable, SYNC_NONE);
    }
}

void EngineSync::activateFollower(Syncable* pSyncable) {
    if (pSyncable == nullptr) {
        qWarning() << "WARNING: Logic Error: Called activateFollower on a nullptr Syncable.";
        return;
    }
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::activateFollower: "
                        << pSyncable->getGroup();
    }

    pSyncable->setSyncMode(SYNC_FOLLOWER);
}

void EngineSync::activateMaster(Syncable* pSyncable, SyncMode masterType) {
    VERIFY_OR_DEBUG_ASSERT(pSyncable) {
        qWarning() << "WARNING: Logic Error: Called activateMaster on a nullptr Syncable.";
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(masterType == SYNC_MASTER_SOFT || masterType == SYNC_MASTER_EXPLICIT) {
        qWarning() << "WARNING: Logic Error: Called activateMaster with non-master mode";
    }
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::activateMaster: "
                        << pSyncable->getGroup() << "type: "
                        << masterType;
    }

    if (m_pMasterSyncable == pSyncable) {
        // Already master, update the master type.
        if (m_pMasterSyncable->getSyncMode() != masterType) {
            m_pMasterSyncable->setSyncMode(masterType);
        }
        // nothing else to do
        return;
    }

    // If a different channel is already master, disable it.
    Syncable* pOldChannelMaster = m_pMasterSyncable;
    m_pMasterSyncable = nullptr;
    if (pOldChannelMaster) {
        pOldChannelMaster->setSyncMode(SYNC_FOLLOWER);
    }

    m_pMasterSyncable = pSyncable;
    pSyncable->setSyncMode(masterType);

    if (m_pMasterSyncable != m_pInternalClock) {
        // the internal clock gets activated and its values are overwritten with this
        // new deck.
        activateFollower(m_pInternalClock);
    }

    // It is up to callers of this function to initialize bpm and beat_distance
    // if necessary.
}

void EngineSync::deactivateSync(Syncable* pSyncable) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::deactivateSync" << pSyncable->getGroup();
    }
    bool wasMaster = isMaster(pSyncable->getSyncMode());
    if (wasMaster) {
        m_pMasterSyncable = nullptr;
    }

    // Notifications happen after-the-fact.
    pSyncable->setSyncMode(SYNC_NONE);

    bool bSyncDeckExists = syncDeckExists();

    if (pSyncable != m_pInternalClock && !bSyncDeckExists) {
        // Deactivate the internal clock if there are no more sync decks left.
        m_pMasterSyncable = nullptr;
        m_pInternalClock->setSyncMode(SYNC_NONE);
    }

    Syncable* newMaster = pickMaster(nullptr);
    if (newMaster != nullptr && m_pMasterSyncable != newMaster) {
        activateMaster(newMaster, SYNC_MASTER_SOFT);
    }
}

Syncable* EngineSync::pickMaster(Syncable* enabling_syncable) {
    // If there is an explicit master, and it is playing, keep it.
    if (m_pMasterSyncable &&
            isMaster(m_pMasterSyncable->getSyncMode()) &&
            (m_pMasterSyncable->isPlaying() || m_pMasterSyncable == m_pInternalClock)) {
        return m_pMasterSyncable;
    }

    Syncable* first_stopped_deck = nullptr;
    Syncable* first_playing_deck = nullptr;
    int stopped_deck_count = 0;
    int playing_deck_count = 0;

    for (const auto& pSyncable : qAsConst(m_syncables)) {
        if (pSyncable->getBaseBpm() <= 0.0) {
            continue;
        }

        if (pSyncable != enabling_syncable) {
            if (!pSyncable->getChannel()->isPrimaryDeck()) {
                continue;
            }
            if (!pSyncable->isSynchronized()) {
                continue;
            }
        }

        if (pSyncable->isPlaying()) {
            if (playing_deck_count == 0) {
                first_playing_deck = pSyncable;
            }
            playing_deck_count++;
        } else {
            if (stopped_deck_count == 0) {
                first_stopped_deck = pSyncable;
            }
            stopped_deck_count++;
        }
    }

    if (playing_deck_count == 1) {
        return first_playing_deck;
    } else if (playing_deck_count > 1) {
        return nullptr;
    }

    // No valid playing sync decks
    if (stopped_deck_count == 1) {
        return first_stopped_deck;
    } else if (stopped_deck_count > 1) {
        return nullptr;
    }

    // No valid stopped sync decks
    return nullptr;
}

Syncable* EngineSync::findBpmMatchTarget(Syncable* requester) {
    Syncable* pStoppedTarget = nullptr;

    for (const auto& pOtherSyncable : qAsConst(m_syncables)) {
        if (pOtherSyncable == requester) {
            continue;
        }
        // Skip non-master decks, like preview decks.
        if (!pOtherSyncable->getChannel()->isMasterEnabled()) {
            continue;
        }
        if (!pOtherSyncable->getChannel()->isPrimaryDeck()) {
            continue;
        }
        if (pOtherSyncable->getBaseBpm() == 0.0) {
            continue;
        }

        // If the other deck is playing we stop looking immediately. Otherwise continue looking
        // for a playing deck with bpm > 0.0.
        if (pOtherSyncable->isPlaying()) {
            return pOtherSyncable;
        }

        // The target is not playing. If this is the first one we have seen,
        // record it. If we never find a playing target, we'll return
        // this one as a fallback.
        // Exception: if the requester is playing, we don't want to match it
        // against a stopped deck.
        if (!pStoppedTarget && !requester->isPlaying()) {
            pStoppedTarget = pOtherSyncable;
        }
    }

    return pStoppedTarget;
}

void EngineSync::notifyPlaying(Syncable* pSyncable, bool playing) {
    Q_UNUSED(playing);
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::notifyPlaying" << pSyncable->getGroup() << playing;
    }
    // For now we don't care if the deck is now playing or stopping.
    if (!pSyncable->isSynchronized()) {
        return;
    }

    // similar to enablesync -- we pick a new master and maybe reinit.
    Syncable* newMaster = pickMaster(playing ? pSyncable : nullptr);

    if (newMaster != nullptr && newMaster != m_pMasterSyncable) {
        activateMaster(newMaster, SYNC_MASTER_SOFT);
    }

    if (auto unique_syncable = getUniquePlayingSyncable(); unique_syncable) {
        // If there is only one remaining syncable, it should reinit the
        // master parameters.
        unique_syncable->notifyOnlyPlayingSyncable();
        setMasterParams(unique_syncable);
    }

    pSyncable->requestSync();
}

void EngineSync::notifyScratching(Syncable* pSyncable, bool scratching) {
    // No special behavior for now.
    Q_UNUSED(pSyncable);
    Q_UNUSED(scratching);
}

void EngineSync::notifyBaseBpmChanged(Syncable* pSyncable, double bpm) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::notifyBaseBpmChanged" << pSyncable->getGroup() << bpm;
    }

    if (isMaster(pSyncable->getSyncMode())) {
        setMasterBpm(pSyncable, bpm);
    }
}

void EngineSync::notifyRateChanged(Syncable* pSyncable, double bpm) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::notifyRateChanged" << pSyncable->getGroup() << bpm;
    }

    setMasterBpm(pSyncable, bpm);
}

void EngineSync::requestBpmUpdate(Syncable* pSyncable, double bpm) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::requestBpmUpdate" << pSyncable->getGroup() << bpm;
    }

    double mbaseBpm = 0.0;
    double mbpm = 0.0;
    double beatDistance = 0.0;
    if (m_pMasterSyncable) {
        mbaseBpm = m_pMasterSyncable->getBaseBpm();
        mbpm = m_pMasterSyncable->getBpm();
        beatDistance = m_pMasterSyncable->getBeatDistance();
    }

    if (mbaseBpm != 0.0) {
        // resync to current master
        pSyncable->setMasterParams(beatDistance, mbaseBpm, mbpm);
    } else {
        // There is no other master, adopt this bpm as master
        pSyncable->setMasterParams(0.0, 0.0, bpm);
    }
}

void EngineSync::notifyInstantaneousBpmChanged(Syncable* pSyncable, double bpm) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::notifyInstantaneousBpmChanged" << pSyncable->getGroup() << bpm;
    }
    if (pSyncable != m_pInternalClock) {
        return;
    }

    // Do not update the master rate slider because instantaneous changes are
    // not user visible.
    setMasterInstantaneousBpm(pSyncable, bpm);
}

void EngineSync::notifyBeatDistanceChanged(Syncable* pSyncable, double beatDistance) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::notifyBeatDistanceChanged"
                        << pSyncable->getGroup() << beatDistance;
    }
    if (pSyncable != m_pInternalClock) {
        return;
    }

    setMasterBeatDistance(pSyncable, beatDistance);
}

Syncable* EngineSync::pickNonSyncSyncTarget(EngineChannel* pDontPick) const {
    // First choice: the sync master, if it's a deck
    if (m_pMasterSyncable &&
            m_pMasterSyncable->getChannel() &&
            m_pMasterSyncable->getChannel() != pDontPick) {
        return m_pMasterSyncable;
    }

    Syncable* pFirstPlayingDeck = nullptr;
    Syncable* pFirstNonplayingDeck = nullptr;
    foreach (Syncable* pSyncable, m_syncables) {
        EngineChannel* pChannel = pSyncable->getChannel();
        // Exclude non-decks
        if (pChannel == nullptr || pChannel == pDontPick) {
            continue;
        }

        // Only consider channels that have a track loaded, are in the master
        // mix, and are primary decks.
        if (pChannel->isActive() && pChannel->isMasterEnabled() && pChannel->isPrimaryDeck()) {
            EngineBuffer* pBuffer = pChannel->getEngineBuffer();
            if (pBuffer && pBuffer->getBpm() > 0) {
                if (pBuffer->getSpeed() != 0.0) {
                    if (pSyncable->getSyncMode() != SYNC_NONE) {
                        // Second choice: first playing sync deck
                        return pSyncable;
                    }
                    if (pFirstPlayingDeck == nullptr) {
                        pFirstPlayingDeck = pSyncable;
                    }
                } else if (pFirstNonplayingDeck == nullptr) {
                    pFirstNonplayingDeck = pSyncable;
                }
            }
        }
    }
    if (pFirstPlayingDeck) {
        // Third choice: first playing non-sync deck
        return pFirstPlayingDeck;
    }

    // No playing decks have a BPM. Go with the first deck that was stopped but
    // had a BPM.
    return pFirstNonplayingDeck;
}

bool EngineSync::otherSyncedPlaying(const QString& group) {
    bool othersInSync = false;
    for (Syncable* theSyncable : qAsConst(m_syncables)) {
        bool isSynchonized = theSyncable->isSynchronized();
        if (theSyncable->getGroup() == group) {
            if (!isSynchonized) {
                return false;
            }
            continue;
        }
        if (theSyncable->isPlaying() && isSynchonized) {
            othersInSync = true;
        }
    }
    return othersInSync;
}
