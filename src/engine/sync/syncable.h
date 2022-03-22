#pragma once

#include <QDebug>
#include <QString>

#include "audio/frame.h"
#include "track/bpm.h"

class EngineChannel;

enum class SyncMode {
    Invalid = -1,
    None = 0,
    Follower = 1,
    // LeaderSoft is a leader that Mixxx has chosen automatically.
    // depending on how decks stop and start, it may reassign soft leader at will.
    LeaderSoft = 2,
    // LeaderExplicit represents an explicit request that the syncable be
    // leader. Mixxx will only remove a LeaderSoft if the track is stopped or
    // ejected.
    LeaderExplicit = 3,
    NumModes
};

inline QDebug operator<<(QDebug debug, const SyncMode& mode) {
    switch (mode) {
    case SyncMode::Invalid:
        return debug << "SyncMode::Invalid";
    case SyncMode::None:
        return debug << "SyncMode::None";
    case SyncMode::Follower:
        return debug << "SyncMode::Follower";
    case SyncMode::LeaderSoft:
        return debug << "SyncMode::LeaderSoft";
    case SyncMode::LeaderExplicit:
        return debug << "SyncMode::LeaderExplicit";
    case SyncMode::NumModes:
        return debug << "SyncMode::NumModes";
    }
    return debug << "SyncMode::Invalid (not in switch/case)";
}
inline SyncMode syncModeFromDouble(double value) {
    // msvs does not allow to cast from double to an enum
    SyncMode mode = static_cast<SyncMode>(int(value));
    if (mode >= SyncMode::NumModes || mode == SyncMode::Invalid) {
        return SyncMode::None;
    }
    return mode;
}

inline bool toSynchronized(SyncMode mode) {
    return mode > SyncMode::None;
}

inline bool isFollower(SyncMode mode) {
    return (mode == SyncMode::Follower);
}

inline bool isLeader(SyncMode mode) {
    return (mode == SyncMode::LeaderSoft || mode == SyncMode::LeaderExplicit);
}

enum class SyncLeaderLight {
    Invalid = -1,
    Off = 0,
    Soft = 1,
    Explicit = 2,
};

inline SyncLeaderLight SyncModeToLeaderLight(SyncMode mode) {
    switch (mode) {
    case SyncMode::Invalid:
    case SyncMode::None:
    case SyncMode::Follower:
        return SyncLeaderLight::Off;
    case SyncMode::LeaderSoft:
        return SyncLeaderLight::Soft;
    case SyncMode::LeaderExplicit:
        return SyncLeaderLight::Explicit;
        break;
    case SyncMode::NumModes:
        break;
    }
    return SyncLeaderLight::Invalid;
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
    virtual bool isQuantized() const = 0;

    // Gets the current speed of the syncable in bpm (bpm * rate slider), doesn't
    // include scratch or FF/REW values.
    virtual mixxx::Bpm getBpm() const = 0;

    // Gets the beat distance as a fraction from 0 to 1
    virtual double getBeatDistance() const = 0;
    // Gets the speed of the syncable if it was playing at 1.0 rate.
    virtual mixxx::Bpm getBaseBpm() const = 0;

    // The following functions are used to tell syncables about the state of the
    // current Sync Leader.
    // Must never result in a call to
    // SyncableListener::notifyBeatDistanceChanged or signal loops could occur.
    virtual void updateLeaderBeatDistance(double beatDistance) = 0;

    // Update the current playback speed (not including scratch values)
    // of the current leader.
    // Must never result in a call to SyncableListener::notifyBpmChanged or
    // signal loops could occur.
    virtual void updateLeaderBpm(mixxx::Bpm bpm) = 0;

    // Tells a Syncable that it's going to be used as a source for leader
    // params. This is a gross hack so that the SyncControl can undo its
    // half/double adjustment so bpms are initialized correctly.
    virtual void notifyLeaderParamSource() = 0;

    // Perform a reset of Leader parameters. This function also triggers recalculation
    // of half-double multiplier.
    virtual void reinitLeaderParams(double beatDistance, mixxx::Bpm baseBpm, mixxx::Bpm bpm) = 0;

    // Update the playback speed of the leader, including scratch values.
    // Must never result in a call to
    // SyncableListener::notifyInstantaneousBpmChanged or signal loops could
    // occur.
    virtual void updateInstantaneousBpm(mixxx::Bpm bpm) = 0;
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
    virtual void notifyBaseBpmChanged(Syncable* pSyncable, mixxx::Bpm bpm) = 0;
    virtual void notifyRateChanged(Syncable* pSyncable, mixxx::Bpm bpm) = 0;
    virtual void requestBpmUpdate(Syncable* pSyncable, mixxx::Bpm bpm) = 0;

    // Syncables notify EngineSync directly about various events. EngineSync
    // does not have a say in whether these succeed or not, they are simply
    // notifications.
    virtual void notifyInstantaneousBpmChanged(Syncable* pSyncable, mixxx::Bpm bpm) = 0;

    // Notify Syncable that the Syncable's scratching state changed.
    virtual void notifyScratching(Syncable* pSyncable, bool scratching) = 0;

    // Notify that the Syncable has seeked.
    virtual void notifySeek(Syncable* pSyncable, mixxx::audio::FramePos position) = 0;

    // A Syncable must never call notifyBeatDistanceChanged in response to a
    // setBeatDistance() call.
    virtual void notifyBeatDistanceChanged(
            Syncable* pSyncable, double beatDistance) = 0;

    virtual void notifyPlayingAudible(Syncable* pSyncable, bool playingAudible) = 0;

    virtual Syncable* getLeaderSyncable() = 0;
};
