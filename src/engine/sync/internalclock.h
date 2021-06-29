#pragma once

#include <QObject>
#include <QString>
#include <QScopedPointer>

#include "engine/sync/clock.h"
#include "engine/sync/syncable.h"

class ControlObject;
class ControlLinPotmeter;
class ControlPushButton;
class EngineSync;
class EngineChannel;

/// Internal Clock is a Master Sync object that provides a source of constant
/// tempo when needed.  The EngineSync will decide when to make the Internal
/// Clock master.  The Internal Clock should not be given any new sources of
/// bpm clock.  If someone wants to write a Midi Clock source, it should be
/// a separate Syncable object that can become master.
class InternalClock : public QObject, public Clock, public Syncable {
    Q_OBJECT
  public:
    InternalClock(const QString& group, SyncableListener* pEngineSync);
    ~InternalClock() override;

    const QString& getGroup() const override {
        return m_group;
    }
    EngineChannel* getChannel() const override {
        return nullptr;
    }

    void setSyncMode(SyncMode mode) override;
    void notifyUniquePlaying() override;
    void requestSync() override;
    SyncMode getSyncMode() const override {
        return m_mode;
    }

    // The clock is always "playing" in a sense but this specifically refers to
    // decks so always return false.
    bool isPlaying() const override {
        return false;
    }
    bool isAudible() const override {
        return false;
    }

    double getBeatDistance() const override;
    void updateMasterBeatDistance(double beatDistance) override;

    double getBaseBpm() const override;
    void updateMasterBpm(double bpm) override;
    void notifyMasterParamSource() override;
    double getBpm() const override;
    void updateInstantaneousBpm(double bpm) override;
    void reinitMasterParams(double beatDistance, double baseBpm, double bpm) override;

    void onCallbackStart(int sampleRate, int bufferSize);
    void onCallbackEnd(int sampleRate, int bufferSize);

  private slots:
    void slotBaseBpmChanged(double baseBpm);
    void slotBeatDistanceChanged(double beatDistance);
    void slotSyncMasterEnabledChangeRequest(double state);

  private:
    void updateBeatLength(int sampleRate, double bpm);

    const QString m_group;
    SyncableListener* m_pEngineSync;
    QScopedPointer<ControlLinPotmeter> m_pClockBpm;
    QScopedPointer<ControlObject> m_pClockBeatDistance;
    QScopedPointer<ControlPushButton> m_pSyncMasterEnabled;
    SyncMode m_mode;

    int m_iOldSampleRate;
    double m_dOldBpm;

    // This is the BPM value at unity adopted when sync is enabled.
    // It is used to relate the followers and must not change when
    // the bpm is adjusted to avoid sudden double/half rate changes.
    double m_dBaseBpm;

    // The internal clock rate is stored in terms of samples per beat.
    // Fractional values are allowed.
    double m_dBeatLength;

    // The current number of frames accumulated since the last beat (e.g. beat
    // distance is m_dClockPosition / m_dBeatLength).
    double m_dClockPosition;
};
