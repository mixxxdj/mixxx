/***************************************************************************
                          enginesync.cpp  -  master sync control for
                          maintaining beatmatching amongst n decks
                             -------------------
    begin                : Mon Mar 12 2012
    copyright            : (C) 2012 by Owen Williams
    email                : owilliams@mixxx.org
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "engine/sync/enginesync.h"

#include <QStringList>

#include "engine/channels/enginechannel.h"
#include "engine/enginebuffer.h"
#include "engine/sync/internalclock.h"
#include "util/assert.h"
#include "util/defs.h"
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
    if (m_pMasterSyncable && m_pMasterSyncable->getSyncMode() == SYNC_MASTER_EXPLICIT && m_pMasterSyncable->isPlaying()) {
        return m_pMasterSyncable;
    }

    Syncable* first_stopped_deck = nullptr;
    Syncable* first_playing_deck = nullptr;
    int stopped_deck_count = 0;
    int playing_deck_count = 0;

    if (enabling_syncable != nullptr && enabling_syncable->getBaseBpm() != 0.0) {
        if (enabling_syncable->isPlaying()) {
            if (playing_deck_count == 0) {
                first_playing_deck = enabling_syncable;
            }
            playing_deck_count++;
        } else {
            if (stopped_deck_count == 0) {
                first_stopped_deck = enabling_syncable;
            }
            stopped_deck_count++;
        }
    }

    for (const auto& pSyncable : m_syncables) {
        if (pSyncable == enabling_syncable) {
            continue;
        }
        if (!pSyncable->getChannel()->isPrimaryDeck()) {
            continue;
        }
        if (!pSyncable->isSynchronized()) {
            continue;
        }
        if (pSyncable->getBaseBpm() == 0.0) {
            continue;
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

    const bool channelIsMaster = m_pMasterSyncable == pSyncable;

    if (isMaster(mode)) {
        activateMaster(pSyncable, mode == SYNC_MASTER_EXPLICIT);
        if (pSyncable->getBaseBpm() > 0) {
            setMasterParams(pSyncable, pSyncable->getBeatDistance(),
                            pSyncable->getBaseBpm(), pSyncable->getBpm());
        }
    } else if (mode == SYNC_FOLLOWER) {
        if (pSyncable == m_pInternalClock && channelIsMaster) {
            if (syncDeckExists()) {
                // Internal clock cannot be set to follower if there are other decks
                // with sync on. Notify them that their mode has not changed.
                pSyncable->setSyncMode(SYNC_MASTER_SOFT);
            } else {
                // No sync deck exists. Allow the clock to go inactive. This
                // case does not happen in practice since the logic in
                // deactivateSync also deactivates the internal clock when the
                // last sync deck deactivates but leave this in for good
                // measure.
                pSyncable->setSyncMode(SYNC_FOLLOWER);
            }
        } else if (channelIsMaster) {
            // Was this deck master before? If so do a handoff.
            m_pMasterSyncable = nullptr;
            activateFollower(pSyncable);
            // Hand off to the internal clock and keep the current BPM and beat
            // distance.
            activateMaster(m_pInternalClock, false);
        } else if (m_pMasterSyncable == nullptr) {
            // If no master active, activate the internal clock.
            activateMaster(m_pInternalClock, false);
            Syncable* targetSyncable = findBpmMatchTarget(pSyncable);
            if (targetSyncable == nullptr) {
                targetSyncable = pSyncable;
            }
            if (targetSyncable->getBaseBpm() > 0) {
                setMasterParams(targetSyncable,
                        targetSyncable->getBeatDistance(),
                        targetSyncable->getBaseBpm(),
                        targetSyncable->getBpm());
            }
            activateFollower(pSyncable);
        } else {
            activateFollower(pSyncable);
        }
    } else {
        if (pSyncable == m_pInternalClock && channelIsMaster &&
                syncDeckExists()) {
            // Internal clock cannot be disabled if there are other decks with
            // sync on. Notify them that their mode has not changed.
            pSyncable->setSyncMode(SYNC_MASTER_SOFT);
        } else {
            deactivateSync(pSyncable);
        }
    }
    checkUniquePlayingSyncable();
}

Syncable* EngineSync::findBpmMatchTarget(Syncable* requester) {
    Syncable* stoppedTarget = nullptr;
    bool foundTargetBpm = false;

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

        // The target is not playing. If the requesting deck is playing, or we have already a
        // non playing deck found, skip.
        if (foundTargetBpm || requester->isPlaying()) {
            continue;
        }

        // Take the first non-playing non-sync deck.
        foundTargetBpm = true;
        stoppedTarget = pOtherSyncable;
    }

    return stoppedTarget;
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

    // The syncable that will be used to initialize the master params, if needed
    Syncable* targetSyncable = nullptr;

    if (m_pMasterSyncable == nullptr) {
        // There is no master. If any other deck is playing we will match
        // the first available bpm -- sync won't be enabled on these decks,
        // otherwise there would have been a master.
        targetSyncable = findBpmMatchTarget(pSyncable);
        if (targetSyncable == nullptr) {
            targetSyncable = pSyncable;
        }
    } else if (m_pMasterSyncable == m_pInternalClock) {
        // Internal clock is master. If there are no active sync decks, reset the internal clock bpm
        // and beat distance.
        if (!syncDeckExists() && pSyncable->getBaseBpm() > 0) {
            targetSyncable = pSyncable;
        }
    }

    // Now go through and possible pick a new master deck if we can find one.
    Syncable* newMaster = pickMaster(pSyncable);

    if (newMaster != nullptr && newMaster != m_pMasterSyncable) {
        activateMaster(newMaster, false);
        if (targetSyncable == nullptr) {
            setMasterParams(newMaster, newMaster->getBeatDistance(), newMaster->getBaseBpm(), newMaster->getBpm());
        }
    }

    if (newMaster != pSyncable) {
        activateFollower(pSyncable);
        pSyncable->requestSync();
    }

    if (targetSyncable != nullptr) {
        setMasterParams(targetSyncable, targetSyncable->getBeatDistance(), targetSyncable->getBaseBpm(), targetSyncable->getBpm());
        if (targetSyncable != pSyncable) {
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

void EngineSync::notifyTrackLoaded(Syncable* pSyncable, double suggested_bpm) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::notifyTrackLoaded";
    }
    // If there are no other sync decks, initialize master based on this.
    // If there is, make sure to set our rate based on that.

    // TODO(owilliams): Check this logic with an explicit master
    if (pSyncable->getSyncMode() != SYNC_FOLLOWER) {
        return;
    }

    bool sync_deck_exists = false;
    foreach (const Syncable* pOtherSyncable, m_syncables) {
        if (pOtherSyncable == pSyncable) {
            continue;
        }
        if (pOtherSyncable->isSynchronized() && pOtherSyncable->getBpm() != 0) {
            sync_deck_exists = true;
            break;
        }
    }

    if (!sync_deck_exists) {
        setMasterBpm(pSyncable, suggested_bpm);
    } else {
        qDebug() << "from trackloaded";
        pSyncable->setMasterBpm(masterBpm());
    }
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

    double mbaseBpm = masterBaseBpm();
    double mbpm = masterBpm();
    if (mbaseBpm != 0.0 && mbpm != 0.0) {
        // resync to current master
        pSyncable->setMasterBaseBpm(mbaseBpm);
        pSyncable->setMasterBpm(mbpm);
    } else {
        // There is no other master, adopt this bpm as master
        setMasterBpm(pSyncable, bpm);
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
    if (pSyncable == nullptr) {
        qWarning() << "WARNING: Logic Error: Called activateMaster on a nullptr Syncable.";
        return;
    }

    // Already master, no need to do anything.
    if (m_pMasterSyncable == pSyncable) {
        // Sanity check.
        if (!isMaster(m_pMasterSyncable->getSyncMode())) {
            qWarning() << "WARNING: Logic Error: m_pMasterSyncable is a syncable that does not think it is master.";
        }
        return;
    }

    // If a channel is master, disable it.
    Syncable* pOldChannelMaster = m_pMasterSyncable;

    m_pMasterSyncable = nullptr;
    if (pOldChannelMaster) {
        activateFollower(pOldChannelMaster);
    }

    //qDebug() << "Setting up master " << pSyncable->getGroup();
    m_pMasterSyncable = pSyncable;
    if (explicitMaster) {
        pSyncable->setSyncMode(SYNC_MASTER_EXPLICIT);
    } else {
        pSyncable->setSyncMode(SYNC_MASTER_SOFT);
    }
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

EngineChannel* EngineSync::pickNonSyncSyncTarget(EngineChannel* pDontPick) const {
    EngineChannel* pFirstNonplayingDeck = nullptr;
    foreach (Syncable* pSyncable, m_syncables) {
        EngineChannel* pChannel = pSyncable->getChannel();
        if (pChannel == nullptr || pChannel == pDontPick) {
            continue;
        }

        // Only consider channels that have a track loaded and are in the master
        // mix.
        if (pChannel->isActive() && pChannel->isMasterEnabled()) {
            EngineBuffer* pBuffer = pChannel->getEngineBuffer();
            if (pBuffer && pBuffer->getBpm() > 0) {
                // If the deck is playing then go with it immediately.
                if (pBuffer->getSpeed() != 0.0) {
                    return pChannel;
                }
                // Otherwise hold out for a deck that might be playing but
                // remember the first deck that matched our criteria.
                if (pFirstNonplayingDeck == nullptr) {
                    pFirstNonplayingDeck = pChannel;
                }
            }
        }
    }

    // No playing decks have a BPM. Go with the first deck that was stopped but
    // had a BPM.
    return pFirstNonplayingDeck;
}

bool EngineSync::otherSyncedPlaying(const QString& group) {
    bool othersInSync = false;
    for (Syncable* theSyncable : m_syncables) {
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
