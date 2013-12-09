#ifndef INTERNALCLOCK_H
#define INTERNALCLOCK_H

#include <QObject>
#include <QString>

#include "engine/clock.h"

class ControlObject;

class InternalClock : public QObject, public Clock {
  public:
    InternalClock(const char* pGroup);
    virtual ~InternalClock();

    void onCallbackStart(int sampleRate, int bufferSize);

    double getBeatDistance() const;
    void setBeatDistance(double beatDistance);

    void setBpm(double bpm);
    double getBpm() const;

  private slots:
    void slotBpmChanged(double bpm);

  private:
    void updateBeatLength(int sampleRate, double bpm);

    QString m_group;
    ControlObject* m_pClockBpm;

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
