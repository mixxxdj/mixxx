#ifndef __BEATJUGGLING_CONTROL__
#define __BEATJUGGLING_CONTROL__

#include <QObject>
#include <QSignalMapper>

#include "configobject.h"
#include "engine/enginecontrol.h"

#include "trackinfoobject.h"
#include "beats.h"

class ControlPushButton;
class ControlObject;
class ControlObjectThread;
class CachingReader;


class BeatControl : public EngineControl {
    Q_OBJECT
public:
    BeatControl(const char * _group, ConfigObject<ConfigValue> * _config, CachingReader *, double beats = 0);
    virtual ~BeatControl();
    double process(const double dRate,
                   const double currentSample,
                   const double totalSamples,
                   const int iBufferSize);

public slots:
    void slotTrackLoaded(TrackPointer tio, int iSampleRate, int iNumSamples);
    void slotUpdatedTrackBeats(int);
    void slotBeatLoop(double);
    void slotBeatSeek(double);
    void slotBeatLoopSize(int);
    void slotBeatSeekSize(int);

private:
    ConfigKey keyForControl(const char * _group, QString ctrlName, double num);

    int m_iCurrentSample;

    ControlObject* m_pCOBeatLoop;
    QList<ControlPushButton*> m_pCOBeatLoops;
    QSignalMapper* m_smBeatLoop;

    CachingReader *m_pReader;

    TrackPointer m_pTrack;
    BeatsPointer m_pBeats;

    ControlObjectThread *m_pCOLoopStart;
    ControlObjectThread *m_pCOLoopEnd;
    ControlObjectThread *m_pCOLoopEnabled;

    ControlObject *m_pCOBeatSeek;
    QList<ControlPushButton*> m_pCOBeatSeeks;
    QSignalMapper* m_smBeatSeek;

    // The next beat to jump from
    double m_iNextJump;
    // The next beat to jump to
    double m_iJumpBeat;
    // Different sizes for Beat Loops/Seeks
    static double s_dBeatSizes[];
};


#endif // __BEAT_CONTROL__
