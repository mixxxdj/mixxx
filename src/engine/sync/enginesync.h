#pragma once

#include <gtest/gtest_prod.h>

#include "engine/sync/syncable.h"
#include "preferences/usersettings.h"

class InternalClock;
class EngineChannel;

/// EngineSync is the heart of the Mixxx Sync Lock engine.  It knows which objects
/// (Decks, Internal Clock, etc) are participating in Sync and what their statuses
/// are. It also orchestrates sync handoffs between different decks as they play,
/// stop, or request their status to change.
class EngineSync : public SyncableListener {
  public:
    explicit EngineSync(UserSettingsPointer pConfig);
    ~EngineSync() override;

    /// Used by Syncables to tell EngineSync it wants to be enabled in a
    /// specific mode. If the state change is accepted, EngineSync calls
    /// Syncable::notifySyncModeChanged.
    void requestSyncMode(Syncable* pSyncable, SyncMode state) override;

    /// Syncables notify EngineSync directly about various events. EngineSync
    /// does not have a say in whether these succeed or not, they are simply
    /// notifications.
    void notifyBaseBpmChanged(Syncable* pSyncable, double bpm) override;
    void notifyRateChanged(Syncable* pSyncable, double bpm) override;
    void requestBpmUpdate(Syncable* pSyncable, double bpm) override;

    /// Instantaneous BPM refers to the actual, honest-to-god speed of playback
    /// at any moment, including any scratching that may be happening.
    void notifyInstantaneousBpmChanged(Syncable* pSyncable, double bpm) override;

    /// the beat distance is updated on every callback.
    void notifyBeatDistanceChanged(Syncable* pSyncable, double beatDistance) override;

    /// Notify the engine that a syncable has started or stopped playing
    void notifyPlayingAudible(Syncable* pSyncable, bool playingAudible) override;
    void notifyScratching(Syncable* pSyncable, bool scratching) override;

    /// Used to pick a sync target for cases where master sync mode is not sufficient.
    /// Guaranteed to pick a Syncable that is a real deck and has an EngineBuffer,
    /// but can return nullptr if there are no choices.
    /// First choice is master sync, if it's a real deck,
    /// then it will fall back to the first playing syncable deck,
    /// then it will fall back to the first playing deck,
    /// then it will fall back to the first non-playing deck.
    /// If there is literally nothing loaded, returns nullptr.
    Syncable* pickNonSyncSyncTarget(EngineChannel* pDontPick) const;

    /// Used to test whether changing the rate of a Syncable would change the rate
    /// of other Syncables that are playing. Returns true even if the other decks
    /// are not audible.
    bool otherSyncedPlaying(const QString& group);

    void addSyncableDeck(Syncable* pSyncable);
    EngineChannel* getMaster() const;
    void onCallbackStart(int sampleRate, int bufferSize);
    void onCallbackEnd(int sampleRate, int bufferSize);

  private:
    /// Iterate over decks, and based on sync and play status, pick a new master.
    /// if enabling_syncable is not null, we treat it as if it were enabled because we may
    /// be in the process of enabling it.
    Syncable* pickMaster(Syncable* enabling_syncable);

    /// Find a deck to match against, used in the case where there is no sync master.
    /// Looks first for a playing deck, and falls back to the first non-playing deck.
    /// If the requester is playing, don't match against a non-playing deck because
    /// that would be strange behavior for the user.
    /// Returns nullptr if none can be found.
    Syncable* findBpmMatchTarget(Syncable* requester);

    /// Activate a specific syncable as master.
    void activateMaster(Syncable* pSyncable, bool explicitMaster);

    /// Activate a specific channel as Follower. Sets the syncable's bpm and
    /// beatDistance to match the master.
    void activateFollower(Syncable* pSyncable);

    // Activate a specific syncable as master, with the appropriate submode.
    void activateMaster(Syncable* pSyncable, SyncMode masterType);

    /// Unsets all sync state on a Syncable.
    void deactivateSync(Syncable* pSyncable);

    /// This utility method returns true if it finds a deck not in SYNC_NONE mode.
    bool syncDeckExists() const;

    /// Return the current BPM of the master Syncable. If no master syncable is
    /// set then returns the BPM of the internal clock.
    double masterBpm() const;

    /// Returns the current beat distance of the master Syncable. If no master
    /// Syncable is set, then returns the beat distance of the internal clock.
    double masterBeatDistance() const;

    /// Returns the overall average BPM of the master Syncable if it were playing
    /// at 1.0 rate. This is used to calculate half/double multipliers and whether
    /// the master has a bpm at all.
    double masterBaseBpm() const;

    /// Set the BPM on every sync-enabled Syncable except pSource.
    void setMasterBpm(Syncable* pSource, double bpm);

    /// Set the master instantaneous BPM on every sync-enabled Syncable except
    /// pSource.
    void setMasterInstantaneousBpm(Syncable* pSource, double bpm);

    /// Set the master beat distance on every sync-enabled Syncable except
    /// pSource.
    void setMasterBeatDistance(Syncable* pSource, double beatDistance);

    void setMasterParams(Syncable* pSource);

    /// Iff there is a single playing syncable in sync mode, return it.
    /// This is used to initialize master params.
    Syncable* getUniquePlayingSyncedDeck() const;

    /// Only for testing. Do not use.
    Syncable* getSyncableForGroup(const QString& group);

    /// Only for testing. Do not use.
    Syncable* getMasterSyncable() override {
        return m_pMasterSyncable;
    }

    bool isSyncMaster(Syncable* pSyncable) {
        if (isMaster(pSyncable->getSyncMode())) {
            DEBUG_ASSERT(m_pMasterSyncable == pSyncable);
            return true;
        }
        return false;
    }

    // TODO: Remove pick algorithms during 2.4 development phase when new beatgrid detection
    // and editing code is committed and we no longer need the lock bpm fallback option.
    // If this code makes it to release we will all be very sad.
    enum SyncLockAlgorithm {
        // New behavior, which should work if beatgrids are reliable.
        PREFER_IMPLICIT_MASTER,
        // Old 2.3 behavior, which works around some issues with bad beatgrid detection, mostly
        // for auto DJ mode.
        PREFER_LOCK_BPM
    };

    FRIEND_TEST(EngineSyncTest, EnableOneDeckInitsMaster);
    FRIEND_TEST(EngineSyncTest, EnableOneDeckInitializesMaster);
    FRIEND_TEST(EngineSyncTest, SyncToNonSyncDeck);
    FRIEND_TEST(EngineSyncTest, SetFileBpmUpdatesLocalBpm);
    FRIEND_TEST(EngineSyncTest, BpmAdjustFactor);
    FRIEND_TEST(EngineSyncTest, MomentarySyncAlgorithmTwo);
    friend class EngineSyncTest;

    UserSettingsPointer m_pConfig;
    /// The InternalClock syncable.
    InternalClock* m_pInternalClock;
    /// The current Syncable that is the leader.
    Syncable* m_pMasterSyncable;
    /// The list of all Syncables registered via addSyncableDeck.
    QList<Syncable*> m_syncables;
};
