#pragma once

#include <QObject>
#include <QScopedPointer>
#include <QString>

#include "audio/types.h"
#include "engine/sync/clock.h"
#include "engine/sync/syncable.h"

class ControlObject;
class ControlLinPotmeter;
class ControlPushButton;
class EngineSync;
class EngineChannel;

/// Internal Clock is a Sync Lock object that provides a source of constant
/// tempo when needed.  The EngineSync will decide when to make the Internal
/// Clock leader.  The Internal Clock should not be given any new sources of
/// bpm clock.  If someone wants to write a Midi Clock source, it should be
/// a separate Syncable object that can become ledaer.
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
    bool isQuantized() const override {
        return true;
    }

    double getBeatDistance() const override;
    void updateLeaderBeatDistance(double beatDistance) override;

    mixxx::Bpm getBaseBpm() const override;
    void updateLeaderBpm(mixxx::Bpm bpm) override;
    void notifyLeaderParamSource() override;
    mixxx::Bpm getBpm() const override;
    void updateInstantaneousBpm(mixxx::Bpm bpm) override;
    void reinitLeaderParams(double beatDistance, mixxx::Bpm baseBpm, mixxx::Bpm bpm) override;

    void onCallbackStart(mixxx::audio::SampleRate sampleRate, int bufferSize);
    void onCallbackEnd(mixxx::audio::SampleRate sampleRate, int bufferSize);

  private slots:
    void slotBpmChanged(double bpm);
    void slotBeatDistanceChanged(double beatDistance);
    void slotSyncLeaderEnabledChangeRequest(double state);

  private:
    void updateBeatLength(mixxx::audio::SampleRate sampleRate, mixxx::Bpm bpm);

    const QString m_group;
    SyncableListener* m_pEngineSync;
    QScopedPointer<ControlLinPotmeter> m_pClockBpm;
    QScopedPointer<ControlObject> m_pClockBeatDistance;
    QScopedPointer<ControlPushButton> m_pSyncLeaderEnabled;
    SyncMode m_mode;

    mixxx::audio::SampleRate m_oldSampleRate;
    mixxx::Bpm m_oldBpm;

    // This is the BPM value at unity adopted when sync is enabled.
    // It is used to relate the followers and must not change when
    // the bpm is adjusted to avoid sudden double/half rate changes.
    mixxx::Bpm m_baseBpm;

    // The internal clock rate is stored in terms of samples per beat.
    // Fractional values are allowed.
    double m_dBeatLength;

    // The current number of frames accumulated since the last beat (e.g. beat
    // distance is m_dClockPosition / m_dBeatLength).
    double m_dClockPosition;
};
