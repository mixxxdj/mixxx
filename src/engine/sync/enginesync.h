/***************************************************************************
                          enginesync.h  -  master sync control for
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

#ifndef ENGINESYNC_H
#define ENGINESYNC_H

#include "preferences/usersettings.h"
#include "engine/sync/syncable.h"
#include "engine/sync/basesyncablelistener.h"

class EngineSync : public BaseSyncableListener {
  public:
    explicit EngineSync(UserSettingsPointer pConfig);
    ~EngineSync() override;

    // Used by Syncables to tell EngineSync it wants to be enabled in a
    // specific mode. If the state change is accepted, EngineSync calls
    // Syncable::notifySyncModeChanged.
    void requestSyncMode(Syncable* pSyncable, SyncMode state) override;

    // Used by Syncables to tell EngineSync it wants to be enabled in any mode
    // (master/follower).
    void requestEnableSync(Syncable* pSyncable, bool enabled) override;

    // Syncables notify EngineSync directly about various events. EngineSync
    // does not have a say in whether these succeed or not, they are simply
    // notifications.
    void notifyBpmChanged(Syncable* pSyncable, double bpm) override;
    void requestBpmUpdate(Syncable* pSyncable, double bpm) override;
    void notifyInstantaneousBpmChanged(Syncable* pSyncable, double bpm) override;
    void notifyBeatDistanceChanged(Syncable* pSyncable, double beatDistance) override;
    void notifyPlaying(Syncable* pSyncable, bool playing) override;
    void notifyScratching(Syncable* pSyncable, bool scratching) override;
    void notifyTrackLoaded(Syncable* pSyncable, double suggested_bpm) override;

    // Used to pick a sync target for non-master-sync mode.
    EngineChannel* pickNonSyncSyncTarget(EngineChannel* pDontPick) const;

    // Used to test whether changing the rate of a Syncable would change the rate
    // of other Syncables that are playing
    bool otherSyncedPlaying(const QString& group);

  private:
    // Activate a specific syncable as master.
    void activateMaster(Syncable* pSyncable);

    // Activate a specific channel as Follower. Sets the syncable's bpm and
    // beat_distance to match the master.
    void activateFollower(Syncable* pSyncable);

    // Unsets all sync state on a Syncable.
    void deactivateSync(Syncable* pSyncable);
};

#endif
