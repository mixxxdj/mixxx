#ifndef SYNCABLE_H
#define SYNCABLE_H

#include <QString>

class EngineChannel;

enum SyncMode {
    SYNC_INVALID = -1,
    SYNC_NONE = 0,
    SYNC_FOLLOWER = 1,
    SYNC_MASTER = 2,
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

class Syncable {
  public:
    virtual ~Syncable() { }
    virtual const QString& getGroup() const = 0;
    virtual EngineChannel* getChannel() const = 0;

    // Notify a Syncable that their mode has changed. The Syncable must record
    // this mode and return the latest mode received via notifySyncModeChanged
    // in response to getMode().
    virtual void notifySyncModeChanged(SyncMode mode) = 0;

    // Notify a Syncable that it is now the only currently-playing syncable.
    virtual void notifyOnlyPlayingSyncable() = 0;

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

    // Gets the current speed of the syncable in bpm (bpm * rate slider), doesn't
    // include scratch or FF/REW values.
    virtual double getBpm() const = 0;

    virtual double getBeatDistance() const = 0;
    // Gets the speed of the syncable if it was playing at 1.0 rate.
    virtual double getBaseBpm() const = 0;

    // The following functions are used to tell syncables about the state of the
    // current Sync Master.
    // Must never result in a call to
    // SyncableListener::notifyBeatDistanceChanged or signal loops could occur.
    virtual void setMasterBeatDistance(double beatDistance) = 0;
    // Reports what the bpm of the master would be if it were playing back at
    // a rate of 1.0x.  This is used by syncables to decide if they should
    // match rates at x2 or /2 speed.  If we were to use the regular BPM, the
    // change of a rate slider might suddenly change the sync multiplier.
    virtual void setMasterBaseBpm(double) = 0;
    // Must never result in a call to SyncableListener::notifyBpmChanged or
    // signal loops could occur.
    virtual void setMasterBpm(double bpm) = 0;

    // Combines the above three calls into one, since they are often set
    // simultaneously.  Avoids redundant recalculation that would occur by
    // using the three calls separately.
    virtual void setMasterParams(double beatDistance, double baseBpm, double bpm) = 0;

    // Must never result in a call to
    // SyncableListener::notifyInstantaneousBpmChanged or signal loops could
    // occur.
    virtual void setInstantaneousBpm(double bpm) = 0;
};

class SyncableListener {
  public:
    virtual ~SyncableListener() {}

    // Used by Syncables to tell EngineSync it wants to be enabled in a
    // specific mode. If the state change is accepted, EngineSync calls
    // Syncable::notifySyncModeChanged.
    virtual void requestSyncMode(Syncable* pSyncable, SyncMode mode) = 0;

    // Used by Syncables to tell EngineSync it wants to be enabled in any mode
    // (master/follower).
    virtual void requestEnableSync(Syncable* pSyncable, bool enabled) = 0;

    // A Syncable must never call notifyBpmChanged in response to a setMasterBpm()
    // call.
    virtual void notifyBpmChanged(Syncable* pSyncable, double bpm) = 0;
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

    virtual void notifyPlaying(Syncable* pSyncable, bool playing) = 0;
    // A syncable can notify that a track has been loaded, and passes in the bpm
    // that it would be set at if the rate slider were left alone.  This allows
    // the master sync engine to either use that rate, if it pleases, or sets
    // the syncable to the existing master bpm.
    virtual void notifyTrackLoaded(Syncable* pSyncable, double suggested_bpm) = 0;
};

#endif /* SYNCABLE_H */
