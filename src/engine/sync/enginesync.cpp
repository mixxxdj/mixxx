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

#include "engine/enginebuffer.h"
#include "engine/channels/enginechannel.h"
#include "engine/sync/internalclock.h"
#include "util/assert.h"

EngineSync::EngineSync(UserSettingsPointer pConfig)
        : BaseSyncableListener(pConfig) {
}

EngineSync::~EngineSync() {
}

Syncable* EngineSync::pickMaster(Syncable* enabling_syncable) {
    std::vector<Syncable*> stopped_sync_decks;
    std::vector<Syncable*> playing_sync_decks;

    qDebug() << "pickmaster";
    if (enabling_syncable != nullptr) {
        qDebug() << "enabling syncable" << enabling_syncable->getGroup() << enabling_syncable->getBaseBpm();
    }

    if (enabling_syncable != nullptr && enabling_syncable->getBaseBpm() != 0.0) {
        if (enabling_syncable->isPlaying()) {
            playing_sync_decks.push_back(enabling_syncable);
        } else {
            stopped_sync_decks.push_back(enabling_syncable);
        }
    }

    for (const auto& pSyncable: m_syncables) {
        qDebug() << "consiudering" << pSyncable->getGroup();
        if (pSyncable == enabling_syncable) {
            qDebug() << "it's the enabling syncable" << pSyncable->getGroup();
            continue;
        }
        // This is a string comparison, but this function is not hot.
        if (!pSyncable->getGroup().contains("Channel")) {
            qDebug() << "not a primary deck, skipping" << pSyncable->getGroup();
            continue;
        }
        if (!pSyncable->isSynchronized()) {
            qDebug() << "not synced" << pSyncable->getGroup();
            continue;
        }
        if (pSyncable->getBaseBpm() == 0.0) {
            qDebug() << "lol 0 bpm" << pSyncable->getGroup();
            continue;
        }

        if (pSyncable->isPlaying()) {
            playing_sync_decks.push_back(pSyncable);
        } else {
            stopped_sync_decks.push_back(pSyncable);
        }
    }

    if (playing_sync_decks.size() == 1) {
        qDebug() << "one playing sync deck" << playing_sync_decks.front()->getGroup();
        return playing_sync_decks.front();
    } else if (playing_sync_decks.size() > 1) {
        qDebug() << "multiple playing sync deck, internal clock";
        return m_pInternalClock;
    }

    // No valid playing sync decks
    if (stopped_sync_decks.size() == 1) {
        qDebug() << "one stopped sync deck" << stopped_sync_decks.front()->getGroup();
        return stopped_sync_decks.front();
    } else if (stopped_sync_decks.size() > 1) {
        qDebug() << "multiple stopped sync deck, internal clock";
        return m_pInternalClock;
    }

    // No valid stopped sync decks
    qDebug() << "no stopped or playing sync deck, nullptr";
    return nullptr;
}

void EngineSync::requestSyncMode(Syncable* pSyncable, SyncMode mode) {
    qDebug() << "EngineSync::requestSyncMode" << pSyncable->getGroup() << mode;
    // Based on the call hierarchy I don't think this is possible. (Famous last words.)
    VERIFY_OR_DEBUG_ASSERT(pSyncable) {
        return;
    }

    const bool channelIsMaster = m_pMasterSyncable == pSyncable;

    if (mode == SYNC_MASTER) {
        activateMaster(pSyncable);
        if (pSyncable->getBaseBpm() > 0) {
            setMasterParams(pSyncable, pSyncable->getBeatDistance(),
                            pSyncable->getBaseBpm(), pSyncable->getBpm());
        }
    } else if (mode == SYNC_FOLLOWER) {
        if (pSyncable == m_pInternalClock && channelIsMaster) {
            if (syncDeckExists()) {
                // Internal clock cannot be set to follower if there are other decks
                // with sync on. Notify them that their mode has not changed.
                pSyncable->notifySyncModeChanged(SYNC_MASTER);
            } else {
                // No sync deck exists. Allow the clock to go inactive. This
                // case does not happen in practice since the logic in
                // deactivateSync also deactivates the internal clock when the
                // last sync deck deactivates but leave this in for good
                // measure.
                pSyncable->notifySyncModeChanged(SYNC_FOLLOWER);
            }
        } else if (channelIsMaster) {
            // Was this deck master before? If so do a handoff.
            m_pMasterSyncable = NULL;
            activateFollower(pSyncable);
            // Hand off to the internal clock and keep the current BPM and beat
            // distance.
            activateMaster(m_pInternalClock);
        } else if (m_pMasterSyncable == NULL) {
            // If no master active, activate the internal clock.
            activateMaster(m_pInternalClock);
            if (pSyncable->getBaseBpm() > 0) {
                setMasterParams(pSyncable, pSyncable->getBeatDistance(),
                                pSyncable->getBaseBpm(), pSyncable->getBpm());
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
            pSyncable->notifySyncModeChanged(SYNC_MASTER);
        } else {
            deactivateSync(pSyncable);
        }
    }
    checkUniquePlayingSyncable();
}

Syncable* EngineSync::findBpmMatchTarget(Syncable* requester) {
    Syncable* stoppedTarget = nullptr;
    bool foundTargetBpm = false;

    for (const auto& pOtherSyncable: m_syncables) {
        if (pOtherSyncable == requester) {
            continue;
        }
        // skip non-master decks, like preview decks.
        if (!pOtherSyncable->getChannel()->isMasterEnabled()) {
            continue;
        }
        // Also skip non-primary decks like samplers.
        // This is a string comparison, but this function is not hot.
        if (!pOtherSyncable->getGroup().contains("Channel")) {
            continue;
        }
        if (pOtherSyncable->getBaseBpm() == 0.0) {
            continue;
        }

        // If the other deck is playing we stop looking immediately. Otherwise continue looking
        // for a playing deck with bpm > 0.0.
        if (pOtherSyncable->isPlaying()) {
            qDebug() << "found playing other syncable, returning it" << pOtherSyncable->getGroup();
            return pOtherSyncable;
        }

        // The target is not playing. If the requesting deck is playing, or we have already a
        // non playing deck found, skip.
        if (foundTargetBpm || requester->isPlaying()) {
            continue;
        }

        // Take the first non-playing non-sync deck.
        qDebug() << "found a non playing non sync deck";
        foundTargetBpm = true;
        stoppedTarget = pOtherSyncable;
    }

    if (stoppedTarget != nullptr) {
        qDebug() << "returning the stopped target" << stoppedTarget->getGroup();
    } else {
        qDebug() << "no bpm target at all";
    }
    return stoppedTarget;
}

void EngineSync::requestEnableSync(Syncable* pSyncable, bool bEnabled) {
    qDebug() << "request enable sync " << pSyncable->getGroup() << bEnabled;
    // Sync disable request, hand off to a different function
    if (!bEnabled) {
        // Already disabled?  Do nothing.
        if (!pSyncable->isSynchronized()) {
            return;
        }
        deactivateSync(pSyncable);
        return;
    }

    qDebug() << "EngineSync::requestEnableSync" << pSyncable->getGroup() << bEnabled;
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
        qDebug() << "(sync-enable) master changed, activating it" << newMaster->getGroup();
        activateMaster(newMaster);
        if (targetSyncable == nullptr) {
            setMasterParams(newMaster, newMaster->getBeatDistance(), newMaster->getBaseBpm(),
                            newMaster->getBpm());
        }
    }

    if (newMaster != pSyncable) {
        qDebug() << "(syncenable) psyncable isn't new master, making follower";
        activateFollower(pSyncable);
        pSyncable->requestSync();
    }

    if (targetSyncable != nullptr) {
        qDebug() << "we have a bpm match target" << targetSyncable->getGroup();
        setMasterParams(targetSyncable, targetSyncable->getBeatDistance(),
                        targetSyncable->getBaseBpm(), targetSyncable->getBpm());
        if (targetSyncable != pSyncable) {
            pSyncable->requestSync();
        }
    }
}

void EngineSync::notifyPlaying(Syncable* pSyncable, bool playing) {
    Q_UNUSED(playing);
    qDebug() << "EngineSync::notifyPlaying" << pSyncable->getGroup() << playing;
    // For now we don't care if the deck is now playing or stopping.
    if (!pSyncable->isSynchronized()) {
        return;
    }

    // similar to enablesync -- we pick a new master and maybe reinit.
    Syncable* newMaster = pickMaster(playing ? pSyncable : nullptr);
    // At the very least we should pick ourselves
    VERIFY_OR_DEBUG_ASSERT(newMaster != nullptr) {
        return;
    }

    if (m_pMasterSyncable != newMaster) {
        qDebug() << "(playing) master changed, activating it" << newMaster->getGroup();
        activateMaster(newMaster);
        setMasterParams(newMaster, newMaster->getBeatDistance(), newMaster->getBaseBpm(),
                        newMaster->getBpm());
    }

    pSyncable->requestSync();
}

void EngineSync::notifyTrackLoaded(Syncable* pSyncable, double suggested_bpm) {
    qDebug() << "EngineSync::notifyTrackLoaded";
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
        pSyncable->setMasterBpm(masterBpm());
    }
}

void EngineSync::notifyScratching(Syncable* pSyncable, bool scratching) {
    // No special behavior for now.
    Q_UNUSED(pSyncable);
    Q_UNUSED(scratching);
}

void EngineSync::notifyBpmChanged(Syncable* pSyncable, double bpm) {
    qDebug() << "EngineSync::notifyBpmChanged" << pSyncable->getGroup() << bpm;

    // Master Base BPM shouldn't be updated for every random deck that twiddles
    // the rate.
    setMasterBpm(pSyncable, bpm);
}

void EngineSync::requestBpmUpdate(Syncable* pSyncable, double bpm) {
    qDebug() << "EngineSync::requestBpmUpdate" << pSyncable->getGroup() << bpm;

    double mbaseBpm = masterBaseBpm();
    double mbpm = masterBpm();
    if (mbaseBpm != 0.0 && mbpm != 0.0) {
        // resync to current master
        qDebug() << mbaseBpm << mbpm;
        pSyncable->setMasterBaseBpm(mbaseBpm);
        pSyncable->setMasterBpm(mbpm);
    } else {
        // There is no other master, adopt this bpm as master
        setMasterBpm(pSyncable, bpm);
    }
}

void EngineSync::notifyInstantaneousBpmChanged(Syncable* pSyncable, double bpm) {
    //qDebug() << "EngineSync::notifyInstantaneousBpmChanged" << pSyncable->getGroup() << bpm;
    if (pSyncable->getSyncMode() != SYNC_MASTER) {
        return;
    }

    // Do not update the master rate slider because instantaneous changes are
    // not user visible.
    setMasterInstantaneousBpm(pSyncable, bpm);
}

void EngineSync::notifyBeatDistanceChanged(Syncable* pSyncable, double beat_distance) {
    //qDebug() << "EngineSync::notifyBeatDistanceChanged" << pSyncable->getGroup() << beat_distance;
    if (pSyncable->getSyncMode() != SYNC_MASTER) {
        return;
    }

    setMasterBeatDistance(pSyncable, beat_distance);
}

void EngineSync::activateFollower(Syncable* pSyncable) {
    if (pSyncable == NULL) {
        qWarning() << "WARNING: Logic Error: Called activateFollower on a NULL Syncable.";
        return;
    }

    pSyncable->notifySyncModeChanged(SYNC_FOLLOWER);
    pSyncable->setMasterParams(masterBeatDistance(), masterBaseBpm(), masterBpm());
}

void EngineSync::activateMaster(Syncable* pSyncable) {
    if (pSyncable == NULL) {
        qWarning() << "WARNING: Logic Error: Called activateMaster on a NULL Syncable.";
        return;
    }

    // Already master, no need to do anything.
    if (m_pMasterSyncable == pSyncable) {
        // Sanity check.
        if (m_pMasterSyncable->getSyncMode() != SYNC_MASTER) {
            qWarning() << "WARNING: Logic Error: m_pMasterSyncable is a syncable that does not think it is master.";
        }
        return;
    }

    // If a channel is master, disable it.
    Syncable* pOldChannelMaster = m_pMasterSyncable;

    m_pMasterSyncable = NULL;
    if (pOldChannelMaster) {
        activateFollower(pOldChannelMaster);
    }

    //qDebug() << "Setting up master " << pSyncable->getGroup();
    m_pMasterSyncable = pSyncable;
    pSyncable->notifySyncModeChanged(SYNC_MASTER);
    if (m_pMasterSyncable != m_pInternalClock) {
        activateFollower(m_pInternalClock);
    }

    // It is up to callers of this function to initialize bpm and beat_distance
    // if necessary.
}

void EngineSync::deactivateSync(Syncable* pSyncable) {
    qDebug() << "deactivate sync" << pSyncable->getGroup();
    bool wasMaster = pSyncable->getSyncMode() == SYNC_MASTER;
    if (wasMaster) {
        m_pMasterSyncable = NULL;
    }

    // Notifications happen after-the-fact.
    pSyncable->notifySyncModeChanged(SYNC_NONE);

    bool bSyncDeckExists = syncDeckExists();

    if (pSyncable != m_pInternalClock && !bSyncDeckExists) {
        // Deactivate the internal clock if there are no more sync decks left.
        m_pMasterSyncable = NULL;
        m_pInternalClock->notifySyncModeChanged(SYNC_NONE);
    }

    // checkUniquePlayingSyncable();

    qDebug() << "picking new master after deactivation";
    Syncable* newMaster = pickMaster(nullptr);

    if (newMaster != nullptr && m_pMasterSyncable != newMaster) {
        qDebug() << "(deactivatesync) master changed, activating it" << newMaster->getGroup();
        activateMaster(newMaster);
    }

}

EngineChannel* EngineSync::pickNonSyncSyncTarget(EngineChannel* pDontPick) const {
    EngineChannel* pFirstNonplayingDeck = NULL;
    foreach (Syncable* pSyncable, m_syncables) {
        EngineChannel* pChannel = pSyncable->getChannel();
        if (pChannel == NULL || pChannel == pDontPick) {
            continue;
        }

        // Only consider channels that have a track loaded and are in the master
        // mix.
        if (pChannel->isActive() && pChannel->isMasterEnabled()) {
            EngineBuffer* pBuffer = pChannel->getEngineBuffer();
            if (pBuffer && pBuffer->getBpm() > 0) {
                // If the deck is playing then go with it immediately.
                if (pBuffer->getSpeed() != 0.0) {
                    qDebug() << "going with this one!" << pChannel->getGroup();
                    return pChannel;
                }
                // Otherwise hold out for a deck that might be playing but
                // remember the first deck that matched our criteria.
                if (pFirstNonplayingDeck == NULL) {
                    qDebug() << "backup option" << pChannel->getGroup();
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
