#ifndef INTERNALCLOCK_H
#define INTERNALCLOCK_H

#include <QObject>
#include <QString>
#include <QScopedPointer>

#include "engine/sync/clock.h"
#include "engine/sync/syncable.h"
#include "engine/enginechannel.h"

class ControlObject;
class ControlPushButton;
class EngineSync;

class InternalClock : public QObject, public Clock, public Syncable {
    Q_OBJECT
  public:
    InternalClock(const char* pGroup, SyncableListener* pEngineSync);
    virtual ~InternalClock();

    const QString& getGroup() const {
        return m_group;
    }
    EngineChannel* getChannel() const {
        return NULL;
    }

    void notifySyncModeChanged(SyncMode mode);
    SyncMode getSyncMode() const {
        return m_mode;
    }

    // The clock is always "playing" in a sense but this specifically refers to
    // decks so always return false.
    bool isPlaying() const {
        return false;
    }

    double getBeatDistance() const;
    void setBeatDistance(double beatDistance);

    void setBpm(double bpm);
    double getBpm() const;
    void setInstantaneousBpm(double bpm);

    void onCallbackStart(int sampleRate, int bufferSize);

  private slots:
    void slotBpmChanged(double bpm);
    void slotBeatDistanceChanged(double beat_distance);
    void slotSyncMasterEnabledChangeRequest(double state);

  private:
    void updateBeatLength(int sampleRate, double bpm);

    QString m_group;
    SyncableListener* m_pEngineSync;
    QScopedPointer<ControlObject> m_pClockBpm;
    QScopedPointer<ControlObject> m_pClockBeatDistance;
    QScopedPointer<ControlPushButton> m_pSyncMasterEnabled;
    SyncMode m_mode;

    int m_iOldSampleRate;
    double m_dOldBpm;

    // The internal clock rate is stored in terms of samples per beat.
    // Fractional values are allowed.
    double m_dBeatLength;

    // The current number of frames accumulated since the last beat (e.g. beat
    // distance is m_dClockPosition / m_dBeatLength).
    double m_dClockPosition;
};

#endif /* INTERNALCLOCK_H */
