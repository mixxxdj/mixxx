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

Syncable* EngineSync::pickMaster(Syncable* enabling_syncable) {
    // If there is an explicit master, and it is playing, keep it.
    if (m_pMasterSyncable && m_pMasterSyncable->getSyncMode() == SYNC_MASTER_EXPLICIT) {
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
        return m_pInternalClock;
    }

    // No valid playing sync decks
    if (stopped_deck_count == 1) {
        return first_stopped_deck;
    } else if (stopped_deck_count > 1) {
        return m_pInternalClock;
    }

    // No valid stopped sync decks
    return nullptr;
}

void EngineSync::requestSyncMode(Syncable* pSyncable, SyncMode mode) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::requestSyncMode" << pSyncable->getGroup() << mode;
    }
    // Based on the call hierarchy I don't think this is possible. (Famous last words.)
    VERIFY_OR_DEBUG_ASSERT(pSyncable) {
        return;
    }

    if (mode == SYNC_MASTER_EXPLICIT) {
        // Note: we enable master unconditionally. If it has no valid
        // tempo, the tempo of the old master remains until we know better
        activateMaster(pSyncable, true);
        if (pSyncable->getBaseBpm() > 0) {
            setMasterParams(pSyncable, pSyncable->getBeatDistance(),
                            pSyncable->getBaseBpm(), pSyncable->getBpm());
        }
    } else if (mode == SYNC_FOLLOWER ||
            mode == SYNC_MASTER_SOFT ||
            pSyncable == m_pInternalClock) {
        // Note: SYNC_MASTER_SOFT and SYNC_FOLLOWER cannot be set explicitly,
        // they are calculated by pickMaster.
        // Internal clock cannot be disabled, it is always listening
        if (m_pMasterSyncable == pSyncable) {
            // This Syncable was master before. Hand off.
            m_pMasterSyncable = nullptr;
            pSyncable->setSyncMode(SYNC_FOLLOWER);
        }
        Syncable* newMaster = pickMaster(pSyncable);
        if (newMaster) {
            activateMaster(newMaster, false);
        }
        if (pSyncable != newMaster) {
            activateFollower(pSyncable);
        }
    } else {
        deactivateSync(pSyncable);
    }
    checkUniquePlayingSyncable();
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
        if (!pStoppedTarget && !requester->isPlaying()) {
            pStoppedTarget = pOtherSyncable;
        }
    }

    return pStoppedTarget;
}

void EngineSync::requestEnableSync(Syncable* pSyncable, bool bEnabled) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::requestEnableSync " << pSyncable->getGroup() << bEnabled;
    }
    // Sync disable request, hand off to a different function
    if (!bEnabled) {
        // Already disabled?  Do nothing.
        if (!pSyncable->isSynchronized()) {
            return;
        }
        deactivateSync(pSyncable);
        return;
    }

    // Already enabled?  Do nothing.
    if (pSyncable->isSynchronized()) {
        return;
    }

    // Now go through and possible pick a new master deck if we can find one.
    Syncable* newMaster = pickMaster(pSyncable);

    // The syncable that will be used to initialize the master params, if needed
    Syncable* pParamsSyncable = nullptr;

    if (newMaster == m_pInternalClock) {
        // This happens if we had a single master before
        if (m_pMasterSyncable != m_pInternalClock) {
            if (pSyncable->isPlaying()) {
                // We are already playing, only change speed if
                // the old master is playing as well
                if (m_pMasterSyncable && m_pMasterSyncable->isPlaying()) {
                    // Adopt value from the old master if it is still playing
                    pParamsSyncable = m_pMasterSyncable;
                } else {
                    DEBUG_ASSERT(pSyncable->getBaseBpm() > 0);
                    pParamsSyncable = pSyncable;
                }
            } else {
                if (m_pMasterSyncable) {
                    pParamsSyncable = m_pMasterSyncable;
                } else {
                    // Can this ever happen?
                    DEBUG_ASSERT(false);
                    pParamsSyncable = pSyncable;
                }
            }
        }
    } else if (newMaster == pSyncable) {
        // There is no other synced deck. If any other deck is playing we will
        // match the first available bpm -- sync won't be enabled on these decks,
        // otherwise there would have been a master.
        pParamsSyncable = findBpmMatchTarget(pSyncable);
        if (pParamsSyncable == nullptr) {
            // We weren't able to find anything to match to, so set ourselves as the
            // target.  That way we'll use our own params when we setMasterParams below.
            pParamsSyncable = pSyncable;
        }
    } else if (newMaster) {
        // This happens if this Deck has no valid BPM
        // avoid that other decks are adjusted
        pParamsSyncable = newMaster;
    } else {
        // This happens if there is no other SyncDeck around and this has no valid BPM
        // nothing to do.
    }

    if (newMaster != nullptr && newMaster != m_pMasterSyncable) {
        activateMaster(newMaster, false);
    }

    if (newMaster != pSyncable) {
        pSyncable->setSyncMode(SYNC_FOLLOWER);
    }

    if (pParamsSyncable != nullptr) {
        setMasterParams(pParamsSyncable,
                pParamsSyncable->getBeatDistance(),
                pParamsSyncable->getBaseBpm(),
                pParamsSyncable->getBpm());
        pSyncable->setInstantaneousBpm(pParamsSyncable->getBpm());
        if (pParamsSyncable != pSyncable) {
            pSyncable->requestSync();
        }
    }
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
        activateMaster(newMaster, false);
        setMasterParams(newMaster, newMaster->getBeatDistance(), newMaster->getBaseBpm(), newMaster->getBpm());
    }

    pSyncable->requestSync();
}

void EngineSync::notifyScratching(Syncable* pSyncable, bool scratching) {
    // No special behavior for now.
    Q_UNUSED(pSyncable);
    Q_UNUSED(scratching);
}

void EngineSync::notifyBpmChanged(Syncable* pSyncable, double bpm) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::notifyBpmChanged" << pSyncable->getGroup() << bpm;
    }

    // Master Base BPM shouldn't be updated for every random deck that twiddles
    // the rate.
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

    if (mbaseBpm != 0.0 && mbpm != 0.0) {
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
    if (!isMaster(pSyncable->getSyncMode())) {
        return;
    }

    // Do not update the master rate slider because instantaneous changes are
    // not user visible.
    setMasterInstantaneousBpm(pSyncable, bpm);
}

void EngineSync::notifyBeatDistanceChanged(Syncable* pSyncable, double beat_distance) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::notifyBeatDistanceChanged" << pSyncable->getGroup() << beat_distance;
    }
    if (!isMaster(pSyncable->getSyncMode())) {
        return;
    }

    setMasterBeatDistance(pSyncable, beat_distance);
}

void EngineSync::activateFollower(Syncable* pSyncable) {
    if (pSyncable == nullptr) {
        qWarning() << "WARNING: Logic Error: Called activateFollower on a nullptr Syncable.";
        return;
    }

    pSyncable->setSyncMode(SYNC_FOLLOWER);
    pSyncable->setMasterParams(masterBeatDistance(), masterBaseBpm(), masterBpm());
    pSyncable->setInstantaneousBpm(masterBpm());
}

void EngineSync::activateMaster(Syncable* pSyncable, bool explicitMaster) {
    VERIFY_OR_DEBUG_ASSERT(pSyncable) {
        qWarning() << "WARNING: Logic Error: Called activateMaster on a nullptr Syncable.";
        return;
    }
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::activateMaster: "
                        << pSyncable->getGroup() << "explicit? "
                        << explicitMaster;
    }

    if (m_pMasterSyncable == pSyncable) {
        // Already master, update the explicit State.
        if (explicitMaster) {
            if (m_pMasterSyncable->getSyncMode() != SYNC_MASTER_EXPLICIT) {
                m_pMasterSyncable->setSyncMode(SYNC_MASTER_EXPLICIT);
            } else if (m_pMasterSyncable->getSyncMode() != SYNC_MASTER_SOFT) {
                m_pMasterSyncable->setSyncMode(SYNC_MASTER_SOFT);
            } else {
                DEBUG_ASSERT(!"Logic Error: m_pMasterSyncable is a syncable that does not think it is master.");
            }
        }
        // nothing else to do
        return;
    }

    // If a channel is master, disable it.
    Syncable* pOldChannelMaster = m_pMasterSyncable;

    m_pMasterSyncable = nullptr;
    if (pOldChannelMaster) {
        pOldChannelMaster->setSyncMode(SYNC_FOLLOWER);
    }

    //qDebug() << "Setting up master " << pSyncable->getGroup();
    m_pMasterSyncable = pSyncable;
    if (explicitMaster) {
        pSyncable->setSyncMode(SYNC_MASTER_EXPLICIT);
    } else {
        pSyncable->setSyncMode(SYNC_MASTER_SOFT);
    }
    pSyncable->setMasterParams(masterBeatDistance(), masterBaseBpm(), masterBpm());
    pSyncable->setInstantaneousBpm(masterBpm());

    if (m_pMasterSyncable != m_pInternalClock) {
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
        activateMaster(newMaster, false);
    }
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
