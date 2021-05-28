#pragma once

#include <QString>

class EngineChannel;

enum SyncMode {
    SYNC_INVALID = -1,
    SYNC_NONE = 0,
    SYNC_FOLLOWER = 1,
    // SYNC_LEADER_SOFT is a leader that Mixxx has chosen automatically.
    // depending on how decks stop and start, it may reassign soft leader at will.
    SYNC_LEADER_SOFT = 2,
    // SYNC_LEADER_EXPLICIT represents an explicit request that the synacable be
    // leader. Mixxx will only remove a SYNC_LEADER_SOFT if the track is stopped or
    // ejected.
    SYNC_LEADER_EXPLICIT = 3,
    SYNC_NUM_MODES
};

inline SyncMode syncModeFromDouble(double value) {
    // msvs does not allow to cast from double to an enum
    SyncMode mode = static_cast<SyncMode>(int(value));
    if (mode >= SYNC_NUM_MODES || mode < 0) {
        return SYNC_NONE;
    }
    return mode;
}

inline bool toSynchronized(SyncMode mode) {
    return mode > SYNC_NONE;
}

inline bool isFollower(SyncMode mode) {
    return (mode == SYNC_FOLLOWER);
}

inline bool isLeader(SyncMode mode) {
    return (mode == SYNC_LEADER_SOFT || mode == SYNC_LEADER_EXPLICIT);
}

enum SyncLeaderLight {
    LEADER_INVALID = -1,
    LEADER_OFF = 0,
    LEADER_SOFT = 1,
    LEADER_EXPLICIT = 2,
};

inline SyncLeaderLight SyncModeToLeaderLight(SyncMode mode) {
    switch (mode) {
    case SYNC_INVALID:
    case SYNC_NONE:
    case SYNC_FOLLOWER:
        return LEADER_OFF;
    case SYNC_LEADER_SOFT:
        return LEADER_SOFT;
    case SYNC_LEADER_EXPLICIT:
        return LEADER_EXPLICIT;
        break;
    case SYNC_NUM_MODES:
        break;
    }
    return LEADER_INVALID;
}

/// Syncable is an abstract base class for any object that wants to participate
/// in Sync Lock.
class Syncable {
  public:
    virtual ~Syncable() = default;
    virtual const QString& getGroup() const = 0;
    virtual EngineChannel* getChannel() const = 0;

    // Notify a Syncable that their mode has changed. The Syncable must record
    // this mode and return the latest mode in response to getMode().
    virtual void setSyncMode(SyncMode mode) = 0;

    // Notify a Syncable that it is now the only currently-playing syncable.
    virtual void notifyUniquePlaying() = 0;

    // Notify a Syncable that they should sync phase.
    virtual void requestSync() = 0;

    // Must NEVER return a mode that was not set directly via
    // notifySyncModeChanged.
    virtual SyncMode getSyncMode() const = 0;

    inline bool isSynchronized() const {
        return toSynchronized(getSyncMode());
    }

    // Only relevant for player Syncables.
    virtual bool isPlaying() const = 0;
    virtual bool isAudible() const = 0;

    // Gets the current speed of the syncable in bpm (bpm * rate slider), doesn't
    // include scratch or FF/REW values.
    virtual double getBpm() const = 0;

    // Gets the beat distance as a fraction from 0 to 1
    virtual double getBeatDistance() const = 0;
    // Gets the speed of the syncable if it was playing at 1.0 rate.
    virtual double getBaseBpm() const = 0;

    // The following functions are used to tell syncables about the state of the
    // current Sync Leader.
    // Must never result in a call to
    // SyncableListener::notifyBeatDistanceChanged or signal loops could occur.
    virtual void updateLeaderBeatDistance(double beatDistance) = 0;

    // Update the current playback speed (not including scratch values)
    // of the current leader.
    // Must never result in a call to SyncableListener::notifyBpmChanged or
    // signal loops could occur.
    virtual void updateLeaderBpm(double bpm) = 0;

    // Tells a Syncable that it's going to be used as a source for leader
    // params. This is a gross hack so that the SyncControl can undo its
    // half/double adjustment so bpms are initialized correctly.
    virtual void notifyLeaderParamSource() = 0;

    // Perform a reset of Leader parameters. This function also triggers recalculation
    // of half-double multiplier.
    virtual void reinitLeaderParams(double beatDistance, double baseBpm, double bpm) = 0;

    // Update the playback speed of the master, including scratch values.
    // Must never result in a call to
    // SyncableListener::notifyInstantaneousBpmChanged or signal loops could
    // occur.
    virtual void updateInstantaneousBpm(double bpm) = 0;
};

/// SyncableListener is an interface class used by EngineSync to receive
/// information about sync change requests.
class SyncableListener {
  public:
    virtual ~SyncableListener() = default;

    // Used by Syncables to tell EngineSync it wants to be enabled in a
    // specific mode. If the state change is accepted, EngineSync calls
    // Syncable::notifySyncModeChanged.
    virtual void requestSyncMode(Syncable* pSyncable, SyncMode mode) = 0;

    // A Syncable must never call notifyBpmChanged in response to a updateLeaderBpm()
    // call.
    virtual void notifyBaseBpmChanged(Syncable* pSyncable, double bpm) = 0;
    virtual void notifyRateChanged(Syncable* pSyncable, double bpm) = 0;
    virtual void requestBpmUpdate(Syncable* pSyncable, double bpm) = 0;

    // Syncables notify EngineSync directly about various events. EngineSync
    // does not have a say in whether these succeed or not, they are simply
    // notifications.
    virtual void notifyInstantaneousBpmChanged(Syncable* pSyncable, double bpm) = 0;

    // Notify Syncable that the Syncable's scratching state changed.
    virtual void notifyScratching(Syncable* pSyncable, bool scratching) = 0;

    // A Syncable must never call notifyBeatDistanceChanged in response to a
    // setBeatDistance() call.
    virtual void notifyBeatDistanceChanged(
            Syncable* pSyncable, double beatDistance) = 0;

    virtual void notifyPlayingAudible(Syncable* pSyncable, bool playingAudible) = 0;

    virtual Syncable* getLeaderSyncable() = 0;
};
