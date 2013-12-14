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

EngineSync::EngineSync(ConfigObject<ConfigValue>* pConfig)
        : BaseSyncableListener(pConfig) {
}

EngineSync::~EngineSync() {
}

void EngineSync::requestSyncMode(Syncable* pSyncable, SyncMode mode) {
    qDebug() << "EngineSync::requestSyncMode" << pSyncable->getGroup() << mode;
    // Based on the call hierarchy I don't think this is possible. (Famous last words.)
    Q_ASSERT(pSyncable);

    const bool channelIsMaster = m_pMasterSyncable == pSyncable;

    if (mode == SYNC_MASTER) {
        activateMaster(pSyncable);
        setMasterBpm(pSyncable, pSyncable->getBpm());
        setMasterBeatDistance(pSyncable, pSyncable->getBeatDistance());
    } else if (mode == SYNC_FOLLOWER) {
        if (pSyncable == m_pInternalClock && m_pMasterSyncable == m_pInternalClock &&
            syncDeckExists()) {
           // Internal cannot be set to follower if there are other decks with sync on.
           return;
        }
        // Was this deck master before?  If so do a handoff.
        if (channelIsMaster) {
            m_pMasterSyncable = NULL;
            activateFollower(pSyncable);
            // Hand off to the internal clock.
            activateMaster(m_pInternalClock);
        } else if (m_pMasterSyncable == NULL) {
            // If no master active, activate the internal clock.
            activateMaster(m_pInternalClock);
            setMasterBpm(pSyncable, pSyncable->getBpm());
            setMasterBeatDistance(pSyncable, pSyncable->getBeatDistance());
        }
        activateFollower(pSyncable);
    } else {
        if (pSyncable == m_pInternalClock && m_pMasterSyncable == m_pInternalClock &&
           syncDeckExists()) {
           // Internal cannot be disabled if there are other decks with sync on.
           return;
        }
        deactivateSync(pSyncable);
    }
}

void EngineSync::requestEnableSync(Syncable* pSyncable, bool bEnabled) {
    qDebug() << "EngineSync::requestEnableSync" << pSyncable->getGroup() << bEnabled;

    SyncMode syncMode = pSyncable->getSyncMode();
    bool syncEnabled = syncMode != SYNC_NONE;
    // Already enabled.
    if (syncEnabled == bEnabled) {
        return;
    }

    if (bEnabled) {
        if (m_pMasterSyncable == NULL) {
            // There is no sync source.  If any other deck is playing we will
            // match the first available bpm -- sync won't be enabled on these decks,
            // otherwise there would have been a sync source.

            bool foundTargetBpm = false;
            double targetBpm = 0.0;
            double targetBeatDistance = 0.0;

            foreach (Syncable* other_deck, m_syncables) {
                if (other_deck == pSyncable) {
                    continue;
                }

                if (other_deck->isPlaying()) {
                    foundTargetBpm = true;
                    targetBpm = other_deck->getBpm();
                    targetBeatDistance = other_deck->getBeatDistance();
                    break;
                } else if (other_deck->getBpm() > 0) {
                    // Last ditch effort -- pick ANYTHING with a pulse.
                    foundTargetBpm = true;
                    targetBpm = other_deck->getBpm();
                    targetBeatDistance = other_deck->getBeatDistance();
                }
            }

            activateMaster(m_pInternalClock);

            if (foundTargetBpm) {
                setMasterBpm(NULL, targetBpm);
                setMasterBeatDistance(NULL, targetBeatDistance);
            } else {
                setMasterBpm(pSyncable, pSyncable->getBpm());
                setMasterBeatDistance(pSyncable, pSyncable->getBeatDistance());
            }
        } else if (m_pMasterSyncable == m_pInternalClock && playingSyncDeckCount() == 0) {
            // If there are no active followers, reset the internal clock beat distance.
            setMasterBeatDistance(pSyncable, pSyncable->getBeatDistance());
        }
        activateFollower(pSyncable);
    } else {
        deactivateSync(pSyncable);
    }
}

void EngineSync::notifyPlaying(Syncable* pSyncable, bool playing) {
    qDebug() << "EngineSync::notifyPlaying" << pSyncable->getGroup() << playing;
    // For now we don't care if the deck is now playing or stopping.
    if (pSyncable->getSyncMode() == SYNC_NONE) {
        return;
    }

    if (m_pMasterSyncable == m_pInternalClock) {
        // If there is only one deck playing, set internal clock beat distance
        // to match it.
        const Syncable* uniqueSyncable = NULL;
        int playing_sync_decks = 0;
        foreach (const Syncable* pOtherSyncable, m_syncables) {
            if (pOtherSyncable->getSyncMode() != SYNC_NONE &&
                    pOtherSyncable->isPlaying()) {
                uniqueSyncable = pOtherSyncable;
                ++playing_sync_decks;
            }
        }
        if (playing_sync_decks == 1) {
            m_pInternalClock->setBeatDistance(uniqueSyncable->getBeatDistance());
        }
    }
}

void EngineSync::notifyScratching(Syncable* pSyncable, bool scratching) {
    // No special behavior for now.
    Q_UNUSED(pSyncable);
    Q_UNUSED(scratching);
}

void EngineSync::notifyBpmChanged(Syncable* pSyncable, double bpm, bool fileChanged) {
    qDebug() << "EngineSync::notifyBpmChanged" << pSyncable->getGroup() << bpm;

    SyncMode syncMode = pSyncable->getSyncMode();
    if (syncMode == SYNC_NONE) {
        return;
    }

    // EngineSyncTest.SlaveRateChange dictates this must not happen in general,
    // but it is required when the file BPM changes because it's not a true BPM
    // change, so we set the follower back to the master BPM.
    if (syncMode == SYNC_FOLLOWER && fileChanged) {
        pSyncable->setBpm(masterBpm());
        return;
    }

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
    pSyncable->notifySyncModeChanged(SYNC_FOLLOWER);
    pSyncable->setBpm(masterBpm());
    pSyncable->setBeatDistance(masterBeatDistance());
}

void EngineSync::activateMaster(Syncable* pSyncable) {
    if (pSyncable == NULL) {
        qDebug() << "WARNING: Logic Error: Called activateMaster on a NULL Syncable.";
        return;
    }

    // Already master, no need to do anything.
    if (m_pMasterSyncable == pSyncable) {
        // Sanity check.
        if (m_pMasterSyncable->getSyncMode() != SYNC_MASTER) {
            qDebug() << "WARNING: Logic Error: m_pMasterSyncable is a syncable that does not think it is master.";
        }
        return;
    }

    // If a channel is master, disable it.
    Syncable* pOldChannelMaster = m_pMasterSyncable;

    m_pMasterSyncable = NULL;
    if (pOldChannelMaster) {
        activateFollower(pOldChannelMaster);
    }

    qDebug() << "Setting up master " << pSyncable->getGroup();
    m_pMasterSyncable = pSyncable;
    pSyncable->notifySyncModeChanged(SYNC_MASTER);

    // It is up to callers of this function to initialize bpm and beat_distance if necessary.
}

void EngineSync::deactivateSync(Syncable* pSyncable) {
    SyncMode prev_mode = pSyncable->getSyncMode();

    pSyncable->notifySyncModeChanged(SYNC_NONE);

    if (prev_mode == SYNC_MASTER) {
        m_pMasterSyncable = NULL;
        if (pSyncable != m_pInternalClock && syncDeckExists()) {
            // Hand off to internal clock
            activateMaster(m_pInternalClock);
            return;
        }
    }

    if (pSyncable != m_pInternalClock && !syncDeckExists()) {
        // Also deactivate the internal clock if there are no more sync decks left.
        m_pMasterSyncable = NULL;
        m_pInternalClock->notifySyncModeChanged(SYNC_NONE);
    }
}
