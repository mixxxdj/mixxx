#pragma once

#include "preferences/usersettings.h"
#include "engine/sync/syncable.h"
#include "engine/sync/basesyncablelistener.h"

/// EngineSync is the heart of the Mixxx Master Sync engine.  It knows which objects
/// (Decks, Internal Clock, etc) are participating in Sync and what their statuses
/// are. It also orchestrates sync handoffs between different decks as they play,
/// stop, or request their status to change.
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

    // Used to pick a sync target for cases where master sync mode is not sufficient.
    // Guaranteed to pick a Syncable that is a real deck and has an EngineBuffer,
    // but can return nullptr if there are no choices.
    // First choice is master sync, if it's a real deck,
    // then it will fall back to the first playing syncable deck,
    // then it will fall back to the first playing deck,
    // then it will fall back to the first non-playing deck.
    // If there is literally nothing loaded, returns nullptr.
    Syncable* pickNonSyncSyncTarget(EngineChannel* pDontPick) const;

    // Used to test whether changing the rate of a Syncable would change the rate
    // of other Syncables that are playing
    bool otherSyncedPlaying(const QString& group);

  private:
    // Iterate over decks, and based on sync and play status, pick a new master.
    // if enabling_syncable is not null, we treat it as if it were enabled because we may
    // be in the process of enabling it.
    Syncable* pickMaster(Syncable* enabling_syncable);

    // Find a deck to match against, used in the case where there is no sync master.
    // Looks first for a playing deck, and falls back to the first non-playing deck.
    // Returns nullptr if none can be found.
    Syncable* findBpmMatchTarget(Syncable* requester);

    // Activate a specific syncable as master.
    void activateMaster(Syncable* pSyncable, bool explicitMaster);

    // Activate a specific channel as Follower. Sets the syncable's bpm and
    // beat_distance to match the master.
    void activateFollower(Syncable* pSyncable);

    // Unsets all sync state on a Syncable.
    void deactivateSync(Syncable* pSyncable);
};
