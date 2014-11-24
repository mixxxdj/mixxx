#ifndef BASESYNCABLELISTENER_H
#define BASESYNCABLELISTENER_H

#include "engine/sync/syncable.h"
#include "configobject.h"

class InternalClock;
class EngineChannel;

class BaseSyncableListener : public SyncableListener {
  public:
    BaseSyncableListener(ConfigObject<ConfigValue>* pConfig);
    virtual ~BaseSyncableListener();

    void addSyncableDeck(Syncable* pSyncable);
    EngineChannel* getMaster() const;
    void onCallbackStart(int sampleRate, int bufferSize);
    void onCallbackEnd(int sampleRate, int bufferSize);

    // Only for testing. Do not use.
    Syncable* getSyncableForGroup(const QString& group);
    Syncable* getMasterSyncable() {
        return m_pMasterSyncable;
    }

    // Used by Syncables to tell EngineSync it wants to be enabled in a
    // specific mode. If the state change is accepted, EngineSync calls
    // Syncable::notifySyncModeChanged.
    virtual void requestSyncMode(Syncable* pSyncable, SyncMode state) = 0;

    // Used by Syncables to tell EngineSync it wants to be enabled in any mode
    // (master/follower).
    virtual void requestEnableSync(Syncable* pSyncable, bool enabled) = 0;

    // Syncables notify EngineSync directly about various events. EngineSync
    // does not have a say in whether these succeed or not, they are simply
    // notifications.
    virtual void notifyBpmChanged(Syncable* pSyncable, double bpm, bool fileChanged=false) = 0;
    virtual void notifyInstantaneousBpmChanged(Syncable* pSyncable, double bpm) = 0;
    virtual void notifyBeatDistanceChanged(Syncable* pSyncable, double beatDistance) = 0;
    virtual void notifyPlaying(Syncable* pSyncable, bool playing) = 0;
    virtual void notifyTrackLoaded(Syncable* pSyncable) = 0;

  protected:
    // Choices about master selection can hinge on if any decks have sync
    // mode enabled.  This utility method returns true if it finds a deck
    // not in SYNC_NONE mode.
    bool syncDeckExists() const;

    // Choices about master selection can hinge on how many decks are playing
    // back. This utility method counts the number of decks not in SYNC_NONE
    // mode that are playing.
    int playingSyncDeckCount() const;

    // Return the current BPM of the master Syncable. If no master syncable is
    // set then returns the BPM of the internal clock.
    double masterBpm() const;

    // Returns the current beat distance of the master Syncable. If no master
    // Syncable is set, then returns the beat distance of the internal clock.
    double masterBeatDistance() const;

    // Returns the current BPM of the master Syncable if it were playing
    // at 1.0 rate.
    double masterBaseBpm() const;

    // Set the BPM on every sync-enabled Syncable except pSource.
    void setMasterBpm(Syncable* pSource, double bpm);

    // Set the master instantaneous BPM on every sync-enabled Syncable except
    // pSource.
    void setMasterInstantaneousBpm(Syncable* pSource, double bpm);

    // Set the master base bpm, which is what the bpm would be if the syncable
    // were playing at 1.0x speed
    void setMasterBaseBpm(Syncable* pSource, double bpm);

    // Set the master beat distance on every sync-enabled Syncable except
    // pSource.
    void setMasterBeatDistance(Syncable* pSource, double beat_distance);

    void setMasterParams(Syncable* pSource, double beat_distance,
                         double base_bpm, double bpm);

    // Check if there is only one playing syncable deck, and notify it if so.
    void checkUniquePlayingSyncable();

    ConfigObject<ConfigValue>* m_pConfig;
    // The InternalClock syncable.
    InternalClock* m_pInternalClock;
    // The current Syncable that is the master.
    Syncable* m_pMasterSyncable;
    // The list of all Syncables registered with BaseSyncableListener via
    // addSyncableDeck.
    QList<Syncable*> m_syncables;
};

#endif /* BASESYNCABLELISTENER_H */
