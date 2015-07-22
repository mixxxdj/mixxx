#ifndef SYNCABLE_H
#define SYNCABLE_H

#include <QString>

class EngineChannel;

enum SyncMode {
    SYNC_NONE = 0,
    SYNC_FOLLOWER = 1,
    SYNC_MASTER = 2,
    SYNC_NUM_MODES
};

inline SyncMode syncModeFromDouble(double value) {
    SyncMode mode = static_cast<SyncMode>(value);
    if (mode >= SYNC_NUM_MODES || mode < 0) {
        return SYNC_NONE;
    }
    return mode;
}

class Syncable {
  public:
    virtual const QString& getGroup() const = 0;
    virtual EngineChannel* getChannel() const = 0;

    // Notify a Syncable that their mode has changed. The Syncable must record
    // this mode and return the latest mode received via notifySyncModeChanged
    // in response to getMode().
    virtual void notifySyncModeChanged(SyncMode mode) = 0;

    // Must NEVER return a mode that was not set directly via
    // notifySyncModeChanged.
    virtual SyncMode getSyncMode() const = 0;

    // Only relevant for player Syncables.
    virtual bool isPlaying() const = 0;

    virtual double getBeatDistance() const = 0;
    // Must never result in a call to
    // SyncableListener::notifyBeatDistanceChanged or signal loops could occur.
    virtual void setBeatDistance(double beatDistance) = 0;

    virtual double getBpm() const = 0;
    // Must never result in a call to SyncableListener::notifyBpmChanged or
    // signal loops could occur.
    virtual void setBpm(double bpm) = 0;

    // Must never result in a call to
    // SyncableListener::notifyInstantaneousBpmChanged or signal loops could
    // occur.
    virtual void setInstantaneousBpm(double bpm) = 0;
};

class SyncableListener {
  public:
    virtual void requestSyncMode(Syncable* pSyncable, SyncMode mode) = 0;
    virtual void requestEnableSync(Syncable* pSyncable, bool enabled) = 0;

    // A Syncable must never call notifyBpmChanged in respnse to a setBpm()
    // call.
    virtual void notifyBpmChanged(Syncable* pSyncable, double bpm,
                                  bool fileChanged=false) = 0;
    virtual void notifyInstantaneousBpmChanged(Syncable* pSyncable, double bpm) = 0;

    // Notify Syncable that the Syncable's scratching state changed.
    virtual void notifyScratching(Syncable* pSyncable, bool scratching) = 0;

    // A Syncable must never call notifyBeatDistanceChanged in respnse to a
    // setBeatDistance() call.
    virtual void notifyBeatDistanceChanged(
        Syncable* pSyncable, double beatDistance) = 0;

    virtual void notifyPlaying(Syncable* pSyncable, bool playing) = 0;
};

#endif /* SYNCABLE_H */
