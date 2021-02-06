// MidiMasterClock provides a sync master from external midi clock input.
// It reads control objects set by midicontroller.cpp, which uses MidiSourceClock.cpp.
// It does not provide midi clock output.

#ifndef MIDIMASTER_H
#define MIDIMASTER_H

#include <QObject>
#include <QString>
#include <memory>

#include "engine/channels/enginechannel.h"
#include "engine/sync/clock.h"
#include "engine/sync/syncable.h"

class ControlLinPotmeter;
class ControlObject;
class ControlPushButton;
class EngineSync;

class MidiMasterClock : public QObject, public Clock, public Syncable {
    Q_OBJECT
  public:
    MidiMasterClock(const QString& group, SyncableListener* pEngineSync);
    virtual ~MidiMasterClock();

    const QString& getGroup() const {
        return m_group;
    }
    EngineChannel* getChannel() const {
        return nullptr;
    }

    void notifySyncModeChanged(SyncMode mode);
    void notifyOnlyPlayingSyncable();
    void requestSync();
    void requestSyncPhase();
    void setSyncMode(SyncMode mode) {
        // TODO: Does this implementation suffice?
        m_mode = mode;
    }
    SyncMode getSyncMode() const {
        return m_mode;
    }

    bool isPlaying() const;

    double getBeatDistance() const;
    void setMasterBeatDistance(double beatDistance);

    double getBaseBpm() const;
    void setMasterBaseBpm(double);
    void setMasterBpm(double bpm);
    double getBpm() const;
    void setInstantaneousBpm(double bpm);
    void setMasterParams(double beatDistance, double baseBpm, double bpm);

    void onCallbackStart(int sampleRate, int bufferSize);
    void onCallbackEnd(int sampleRate, int bufferSize);

  private slots:
    void slotSyncMasterEnabledChangeRequest(double state);

  private:
    QString m_group;
    SyncableListener* m_pEngineSync;
    std::unique_ptr<ControlPushButton> m_pSyncMasterEnabled;

    std::unique_ptr<ControlObject> m_pMidiSourceClockBpm;
    std::unique_ptr<ControlObject> m_pMidiSourceClockLastBeatTime;
    std::unique_ptr<ControlObject> m_pMidiSourceClockBeatDistance;
    // Indicates if the midi clock is active or stopped.
    std::unique_ptr<ControlPushButton> m_pMidiSourceClockRunning;
    // Since there may be differences in latency between other midi devices and
    // Mixxx, allow for manual adjustment of the beat percentage value.
    std::unique_ptr<ControlLinPotmeter> m_pMidiSourceClockSyncAdjust;

    SyncMode m_mode;
    double m_dOldBpm;
};

#endif // MIDIMASTER_H
