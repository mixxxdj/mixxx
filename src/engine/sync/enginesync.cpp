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

#include "engine/sync/internalclock.h"
#include "util/assert.h"

EngineSync::EngineSync(ConfigObject<ConfigValue>* pConfig)
        : BaseSyncableListener(pConfig) {
}

EngineSync::~EngineSync() {
}

void EngineSync::requestSyncMode(Syncable* pSyncable, SyncMode mode) {
    //qDebug() << "EngineSync::requestSyncMode" << pSyncable->getGroup() << mode;
    // Based on the call hierarchy I don't think this is possible. (Famous last words.)
    DEBUG_ASSERT_AND_HANDLE(pSyncable) {
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

void EngineSync::requestEnableSync(Syncable* pSyncable, bool bEnabled) {
    //qDebug() << "EngineSync::requestEnableSync" << pSyncable->getGroup() << bEnabled;
    if (bEnabled) {
        bool foundPlayingDeck = false;
        if (m_pMasterSyncable == NULL) {
            // There is no master. If any other deck is playing we will match
            // the first available bpm -- sync won't be enabled on these decks,
            // otherwise there would have been a master.

            bool foundTargetBpm = false;
            double targetBpm = 0.0;
            double targetBeatDistance = 0.0;
            double targetBaseBpm = 0.0;

            foreach (const Syncable* other_deck, m_syncables) {
                if (other_deck == pSyncable) {
                    // skip this deck
                    continue;
                }
                if (!other_deck->getChannel()->isMaster()) {
                    // skip non-master decks, like preview decks.
                    continue;
                }

                double otherDeckBpm = other_deck->getBpm();
                if (otherDeckBpm > 0.0) {
                    // If the requesting deck is playing, but the other deck
                    // is not, do not sync.
                    if (pSyncable->isPlaying() && !other_deck->isPlaying()) {
                        continue;
                    }
                    foundTargetBpm = true;
                    targetBpm = otherDeckBpm;
                    targetBaseBpm = other_deck->getBaseBpm();
                    targetBeatDistance = other_deck->getBeatDistance();

                    // If the other deck is playing we stop looking
                    // immediately. Otherwise continue looking for a playing
                    // deck with bpm > 0.0.
                    if (other_deck->isPlaying()) {
                        foundPlayingDeck = true;
                        break;
                    }
                }
            }

            activateMaster(m_pInternalClock);

            if (foundTargetBpm) {
                setMasterParams(pSyncable, targetBeatDistance,
                                targetBaseBpm, targetBpm);
            } else if (pSyncable->getBaseBpm() > 0) {
                setMasterParams(pSyncable, pSyncable->getBeatDistance(),
                                pSyncable->getBaseBpm(), pSyncable->getBpm());
            }
        } else if (m_pMasterSyncable == m_pInternalClock) {
            if (!syncDeckExists() && pSyncable->getBaseBpm() > 0) {
                // If there are no active sync decks, reset the internal clock bpm
                // and beat distance.
                setMasterParams(pSyncable, pSyncable->getBeatDistance(),
                                pSyncable->getBaseBpm(), pSyncable->getBpm());
            }
            if (playingSyncDeckCount() > 0) {
                foundPlayingDeck = true;
            }
        } else if (m_pMasterSyncable != NULL) {
            foundPlayingDeck = true;
        }
        activateFollower(pSyncable);
        if (foundPlayingDeck && pSyncable->isPlaying()) {
            // Users also expect phase to be aligned when they press the sync button.
            pSyncable->requestSyncPhase();
        }
    } else {
        deactivateSync(pSyncable);
    }
    checkUniquePlayingSyncable();
}

void EngineSync::notifyPlaying(Syncable* pSyncable, bool playing) {
    Q_UNUSED(playing);
    //qDebug() << "EngineSync::notifyPlaying" << pSyncable->getGroup() << playing;
    // For now we don't care if the deck is now playing or stopping.
    if (pSyncable->getSyncMode() == SYNC_NONE) {
        return;
    }

    if (m_pMasterSyncable == m_pInternalClock) {
        // If there is only one deck playing, set internal clock beat distance
        // to match it, unless there is a single other playing deck, in which
        // case we should match that.
        Syncable* uniqueSyncEnabled = NULL;
        const Syncable* uniqueSyncDisabled = NULL;
        int playing_sync_decks = 0;
        int playing_nonsync_decks = 0;
        foreach (Syncable* pOtherSyncable, m_syncables) {
            if (pOtherSyncable->isPlaying()) {
                if (pOtherSyncable->getSyncMode() != SYNC_NONE) {
                    uniqueSyncEnabled = pOtherSyncable;
                    ++playing_sync_decks;
                } else {
                    uniqueSyncDisabled = pOtherSyncable;
                    ++playing_nonsync_decks;
                }
            }
        }
        if (playing_sync_decks == 1) {
            uniqueSyncEnabled->notifyOnlyPlayingSyncable();
            if (playing_nonsync_decks == 1) {
                m_pInternalClock->setMasterBeatDistance(uniqueSyncDisabled->getBeatDistance());
            } else {
                m_pInternalClock->setMasterBeatDistance(uniqueSyncEnabled->getBeatDistance());
            }
        }
    }
}

void EngineSync::notifyTrackLoaded(Syncable* pSyncable) {
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
        if (pOtherSyncable->getSyncMode() != SYNC_NONE && pOtherSyncable->getBpm() != 0) {
            sync_deck_exists = true;
            break;
        }
    }

    if (!sync_deck_exists) {
        setMasterBpm(pSyncable, pSyncable->getBpm());
    } else {
        pSyncable->setMasterBpm(masterBpm());
    }
}

void EngineSync::notifyScratching(Syncable* pSyncable, bool scratching) {
    // No special behavior for now.
    Q_UNUSED(pSyncable);
    Q_UNUSED(scratching);
}

void EngineSync::notifyBpmChanged(Syncable* pSyncable, double bpm, bool fileChanged) {
    //qDebug() << "EngineSync::notifyBpmChanged" << pSyncable->getGroup() << bpm;

    SyncMode syncMode = pSyncable->getSyncMode();
    if (syncMode == SYNC_NONE) {
        return;
    }

    // EngineSyncTest.SlaveRateChange dictates this must not happen in general,
    // but it is required when the file BPM changes because it's not a true BPM
    // change, so we set the follower back to the master BPM.
    if (syncMode == SYNC_FOLLOWER && fileChanged) {
        double mbaseBpm = masterBaseBpm();
        double mbpm = masterBpm();
        // TODO(owilliams): Figure out why the master bpm is getting set to
        // zero in the first place, that's the real bug that's being worked
        // around.
        if (mbaseBpm != 0.0 && mbpm != 0.0) {
            pSyncable->setMasterBaseBpm(mbaseBpm);
            pSyncable->setMasterBpm(mbpm);
            return;
        }
    }

    // Master Base BPM shouldn't be updated for every random deck that twiddles
    // the rate.
    setMasterBpm(pSyncable, bpm);
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

    // It is up to callers of this function to initialize bpm and beat_distance
    // if necessary.
}

void EngineSync::deactivateSync(Syncable* pSyncable) {
    bool wasMaster = pSyncable->getSyncMode() == SYNC_MASTER;
    if (wasMaster) {
        m_pMasterSyncable = NULL;
    }

    // Notifications happen after-the-fact.
    pSyncable->notifySyncModeChanged(SYNC_NONE);

    bool bSyncDeckExists = syncDeckExists();

    if (pSyncable != m_pInternalClock) {
        if (bSyncDeckExists) {
            if (wasMaster) {
                // Hand off to internal clock
                activateMaster(m_pInternalClock);
            }
        } else {
            // Deactivate the internal clock if there are no more sync decks left.
            m_pMasterSyncable = NULL;
            m_pInternalClock->notifySyncModeChanged(SYNC_NONE);
        }
    }
}
