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

#include "engine/enginecontrol.h"
#include "engine/syncable.h"

class EngineChannel;
class ControlObject;
class ControlPushButton;
class ControlPotmeter;
class InternalClock;

class EngineSync : public EngineControl, public SyncableListener {
    Q_OBJECT
  public:
    explicit EngineSync(ConfigObject<ConfigValue>* pConfig);
    virtual ~EngineSync();

    void addSyncableDeck(Syncable* pSyncable);
    EngineChannel* getMaster() const;
    void onCallbackStart(int sampleRate, int bufferSize);

    // Only for testing. Do not use.
    Syncable* getSyncableForGroup(const QString& group);
    Syncable* getMasterSyncable() {
        return m_pMasterSyncable;
    }

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

  private slots:
    void slotSyncRateSliderChanged(double);

  private:
    // Choices about master selection often hinge on how many decks are playing back.
    int playingSyncDeckCount() const;

    // Activate a specific syncable as master.
    void activateMaster(Syncable* pSyncable);

    // Activate a specific channel as Follower. Sets the syncable's bpm and
    // beat_distance to match the master.
    void activateFollower(Syncable* pSyncable);

    // Picks a new master (does not pick pDontPick) and calls
    // activateChannelMaster on it. Clears m_bExplicitMasterSelected because the
    // master it picks is not explicitly selected by the user.
    void findNewMaster(Syncable* pDontPick);

    double masterBpm() const;
    double masterBeatDistance() const;

    // Set the master BPM.
    void setMasterBpm(Syncable* pSource, double bpm);
    // Set the master instantaneous BPM.
    void setMasterInstantaneousBpm(Syncable* pSource, double bpm);
    // Set the master beat distance.
    void setMasterBeatDistance(Syncable* pSource, double beat_distance);

    ConfigObject<ConfigValue>* m_pConfig;
    InternalClock* m_pInternalClock;
    Syncable* m_pMasterSyncable;
    ControlObject* m_pMasterBeatDistance;
    ControlPotmeter* m_pMasterRateSlider;
    QList<Syncable*> m_syncables;
    bool m_bExplicitMasterSelected;
};

#endif
