#ifndef VISUALPLAYPOSITION_H
#define VISUALPLAYPOSITION_H

#include <portaudio.h>
#include "util/performancetimer.h"
#include "control/controlvalue.h"

#include <QMutex>
#include <QTime>
#include <QMap>
#include <QAtomicPointer>

class ControlObjectSlave;
class VSyncThread;

// This class is for synchronizing the sound device DAC time with the waveforms, displayed on the
// graphic device, using the CPU time
//
// DAC: ------|--------------|-------|-------------------|-----------------------|-----
//            ^Audio Callback Entry  |                   |                       ^Last Sample to DAC
//            |              ^Buffer prepared            ^Waveform sample X
//            |                      ^First sample transfered to DAC
// CPU: ------|-------------------------------------------------------------------------
//            ^Start m_timeInfoTime                      |
//                                                       |
// GPU: ---------|----------------------------------- |--|-------------------------------
//               ^Render Waveform sample X            |  ^VSync (New waveform is displayed
//                by use usFromTimerToNextSync        ^swap Buffer

class VisualPlayPositionData {
  public:
    PerformanceTimer m_referenceTime;
    int m_callbackEntrytoDac; // Time from Audio Callback Entry to first sample of Buffer is transfered to DAC
    double m_enginePlayPos; // Play position of fist Sample in Buffer
    double m_rate;
    double m_positionStep;
    double m_pSlipPosition;
};


class VisualPlayPosition
{
  public:
    VisualPlayPosition();
    ~VisualPlayPosition();

    void set(double playPos, double rate, double positionStep, double pSlipPosition);
    double getAtNextVSync(VSyncThread* vsyncThread);
    void getPlaySlipAt(int usFromNow, double* playPosition, double* slipPosition);
    double getEnginePlayPos();
    static VisualPlayPosition* getVisualPlayPosition(QString group);
    static void setTimeInfo(const PaStreamCallbackTimeInfo *timeInfo);
    void setInvalid() { m_valid = false; };


  private:
    ControlValueAtomic<VisualPlayPositionData> m_data;
    double m_playPosOld;
    int m_deltatime;
    ControlObjectSlave* m_audioBufferSize;
    PaTime m_outputBufferDacTime;
    bool m_valid;

    static QMap<QString, VisualPlayPosition*> m_listVisualPlayPosition;
    // Time info from the Sound device, updated just after audio callback is called
    static const PaStreamCallbackTimeInfo* m_timeInfo;
    // Time stamp for m_timeInfo in main CPU time
    static PerformanceTimer m_timeInfoTime;

};

#endif // VISUALPLAYPOSITION_H

