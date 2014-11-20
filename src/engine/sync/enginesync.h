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

#include "configobject.h"
#include "engine/sync/syncable.h"
#include "engine/sync/basesyncablelistener.h"

class EngineSync : public BaseSyncableListener {
  public:
    explicit EngineSync(ConfigObject<ConfigValue>* pConfig);
    virtual ~EngineSync();

    // Used by Syncables to tell EngineSync it wants to be enabled in a
    // specific mode. If the state change is accepted, EngineSync calls
    // Syncable::notifySyncModeChanged.
    void requestSyncMode(Syncable* pSyncable, SyncMode state);

    // Used by Syncables to tell EngineSync it wants to be enabled in any mode
    // (master/follower).
    void requestEnableSync(Syncable* pSyncable, bool enabled);

    // Syncables notify EngineSync directly about various events. EngineSync
    // does not have a say in whether these succeed or not, they are simply
    // notifications.
    void notifyBpmChanged(Syncable* pSyncable, double bpm, bool fileChanged=false);
    void notifyInstantaneousBpmChanged(Syncable* pSyncable, double bpm);
    void notifyBeatDistanceChanged(Syncable* pSyncable, double beatDistance);
    void notifyPlaying(Syncable* pSyncable, bool playing);
    void notifyScratching(Syncable* pSyncable, bool scratching);
    void notifyTrackLoaded(Syncable* pSyncable);

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
