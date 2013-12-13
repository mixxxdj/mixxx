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
        : BaseSyncableListener(pConfig),
          m_pScratchingPreviousMaster(NULL),
          m_bExplicitMasterSelected(false) {
}

EngineSync::~EngineSync() {
}

void EngineSync::requestSyncMode(Syncable* pSyncable, SyncMode mode) {
    qDebug() << "EngineSync::requestSyncMode" << pSyncable->getGroup() << mode;
    // Based on the call hierarchy I don't think this is possible. (Famous last words.)
    Q_ASSERT(pSyncable);

    const bool channelIsMaster = m_pMasterSyncable == pSyncable;

    if (mode == SYNC_MASTER) {
        // Syncable is explicitly requesting master, so we'll honor that.
        m_bExplicitMasterSelected = true;
        activateMaster(pSyncable);
    } else if (mode == SYNC_FOLLOWER) {
        // Was this deck master before?  If so do a handoff.
        if (channelIsMaster) {
            m_pMasterSyncable = NULL;
            activateFollower(pSyncable);
            // Choose a new master, but don't pick the current one.
            findNewMaster(pSyncable);
        } else if (m_bExplicitMasterSelected) {
            // Do nothing.
            activateFollower(pSyncable);
            return;
        }

        if (m_pMasterSyncable == NULL) {
            // If there is no current master, set to master.
            // TODO(rryan): Really? User asked to become a follower. We should
            // probably just enable internal clock, no?
            activateMaster(pSyncable);
        } else if (!m_bExplicitMasterSelected) {
            if (m_pMasterSyncable == m_pInternalClock) {
                if (playingSyncDeckCount() == 1) {
                    // We should be master now.
                    activateMaster(pSyncable);
                }
            } else {
                // If there was a deck master, set to internal clock.
                if (playingSyncDeckCount() > 1) {
                    activateMaster(m_pInternalClock);
                }
            }
        }
    } else {
        pSyncable->notifySyncModeChanged(SYNC_NONE);
        // if we were the master, choose a new one.
        if (channelIsMaster) {
            m_pMasterSyncable = NULL;
            findNewMaster(NULL);
        }
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
            // match the first available bpm even if sync is not enabled,
            // although we will still be a master,
            bool foundTargetBpm = false;
            double targetBpm = 0.0;

            foreach (Syncable* other_deck, m_syncables) {
                if (other_deck == pSyncable) {
                    continue;
                }

                if (other_deck->isPlaying()) {
                    foundTargetBpm = true;
                    targetBpm = other_deck->getBpm();
                    break;
                }
            }

            activateMaster(pSyncable);

            if (foundTargetBpm) {
                setMasterBpm(NULL, targetBpm);
            }
        } else if (m_pMasterSyncable == m_pInternalClock) {
            // If there are no playing decks and the internal clock is master
            // then we take over as master.

            int playing_sync_decks = playingSyncDeckCount();
            if (playing_sync_decks == 0) {
                // We also require that the deck take on the current internal
                // clock BPM.
                double targetBpm = m_pMasterSyncable->getBpm();
                activateMaster(pSyncable);
                setMasterBpm(NULL, targetBpm);
            } else {
                activateFollower(pSyncable);
            }
        } else {
            activateFollower(pSyncable);
        }
    } else {
        pSyncable->notifySyncModeChanged(SYNC_NONE);
        // It was the master.
        if (syncMode == SYNC_MASTER) {
            m_pMasterSyncable = NULL;
            findNewMaster(pSyncable);
        } else if (!m_bExplicitMasterSelected &&
                   m_pMasterSyncable == m_pInternalClock) {
            // If no explicit master exists, we are using the internal clock,
            // and a follower has dropped out (switched to NONE) then we may
            // potentially elect a playing deck as a master.
            m_pMasterSyncable = NULL;
            findNewMaster(pSyncable);
        }
    }
}

void EngineSync::notifyPlaying(Syncable* pSyncable, bool playing) {
    qDebug() << "EngineSync::notifyPlaying" << pSyncable->getGroup() << playing;
    // For now we don't care if the deck is now playing or stopping.
    if (pSyncable->getSyncMode() != SYNC_NONE) {
        int playing_deck_count = playingSyncDeckCount();
        if (!m_bExplicitMasterSelected) {
            if (playing_deck_count == 0) {
                if (playing) {
                    // Nothing was playing, so set self as master
                    activateMaster(pSyncable);
                    // TODO(rryan): What if this fails? Do nothing?
                } else {
                    // Everything has now stopped.
                }
            } else if (playing_deck_count == 1) {
                if (!playing && m_pMasterSyncable == m_pInternalClock) {
                    // If a deck has stopped, and only one deck is now playing,
                    // and we were internal clock, pick a new master (the playing deck).
                    findNewMaster(m_pInternalClock);
                }
            } else {
                // TODO(rryan): playing_deck_count > 1, no master explicitly
                // selected. Why set internal clock?
                activateMaster(m_pInternalClock);
            }
        }
    }
}

void EngineSync::notifyScratching(Syncable* pSyncable, bool scratching) {
    SyncMode mode = pSyncable->getSyncMode();

    // Ignore decks that aren't part of the sync group.
    if (mode == SYNC_NONE) {
        return;
    }

    // Syncable started scratching.
    if (scratching) {
        // If there is no explicit master.
        if (!m_bExplicitMasterSelected) {
            // If the syncable is not the master, become the master.
            if (mode != SYNC_MASTER) {
                // TODO(rryan): This is kind of janky when multiple decks
                // scratch at once. For now, the last "previous master" always
                // wins.
                m_pScratchingPreviousMaster = m_pMasterSyncable;
                activateMaster(pSyncable);
            }
        } else {
            // The master is explicitly not us. Don't do anything.
        }
    } else {
        // Syncable stopped scratching.
        if (!m_bExplicitMasterSelected) {
            // There was a previous master.
            if (m_pScratchingPreviousMaster) {
                activateMaster(m_pScratchingPreviousMaster);
                m_pScratchingPreviousMaster = NULL;
            }
        } else {
            // The master is explicitly set. Don't do anything.
        }
    }
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

    // Only consider channels that have a track loaded and are in the master
    // mix.
    // TODO(rryan): We don't actually do what this comment describes.
    qDebug() << "Setting up master " << pSyncable->getGroup();
    m_pMasterSyncable = pSyncable;
    pSyncable->notifySyncModeChanged(SYNC_MASTER);
    // TODO(rryan): Iffy? We should not be calling these methods. But there's no
    // other method that does exactly this.
    notifyBpmChanged(pSyncable, pSyncable->getBpm());
    notifyBeatDistanceChanged(pSyncable, pSyncable->getBeatDistance());
}

void EngineSync::findNewMaster(Syncable* pDontPick) {
    qDebug() << "EngineSync::findNewMaster" << (pDontPick ? pDontPick->getGroup() : "(null)");
    int playing_sync_decks = 0;
    int paused_sync_decks = 0;
    Syncable *new_master = NULL;

    if (m_pMasterSyncable != NULL) {
        qDebug() << "WARNING: Logic Error: findNewMaster called when a master is selected.";
    }

    foreach (Syncable* pSyncable, m_syncables) {
        if (pSyncable == pDontPick) {
            qDebug() << "findNewMaster: Skipping" << pSyncable->getGroup() << "because DONTPICK";
            continue;
        }

        SyncMode sync_mode = pSyncable->getSyncMode();
        if (sync_mode == SYNC_NONE) {
            qDebug() << "findNewMaster: Skipping" << pSyncable->getGroup() << "because SYNC_NONE";
            continue;
        }

        if (sync_mode == SYNC_MASTER) {
            qDebug() << "WARNING: Logic Error: findNewMaster: A Syncable with SYNC_MASTER exists.";
            return;
        }

        if (pSyncable->isPlaying()) {
            ++playing_sync_decks;
            new_master = pSyncable;
        } else {
            ++paused_sync_decks;
        }
    }

    if (playing_sync_decks == 1) {
        if (new_master != NULL) {
            activateMaster(new_master);
        }
    } else if (pDontPick != m_pInternalClock) {
        // If there are no more synced decks, there is no need for a master.
        if (playing_sync_decks + paused_sync_decks > 0) {
            activateMaster(m_pInternalClock);
        }
    } else {
        // Clock master was specifically disabled. Just go with new_master if it
        // exists, otherwise give up and pick nothing.
        if (new_master != NULL) {
            activateMaster(new_master);
        }
    }
    // Even if we didn't successfully find a new master, unset this value.
    m_bExplicitMasterSelected = false;
}
