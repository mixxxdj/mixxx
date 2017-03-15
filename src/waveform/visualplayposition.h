#ifndef VISUALPLAYPOSITION_H
#define VISUALPLAYPOSITION_H

#include <portaudio.h>

#include <QMutex>
#include <QTime>
#include <QMap>
#include <QAtomicPointer>

#include "util/performancetimer.h"
#include "control/controlvalue.h"

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


class VisualPlayPosition : public QObject {
    Q_OBJECT
  public:
    VisualPlayPosition(const QString& m_key);
    virtual ~VisualPlayPosition();

    // WARNING: Not thread safe. This function must be called only from the
    // engine thread.
    void set(double playPos, double rate, double positionStep, double pSlipPosition);
    double getAtNextVSync(VSyncThread* vsyncThread);
    void getPlaySlipAt(int usFromNow, double* playPosition, double* slipPosition);
    double getEnginePlayPos();

    // WARNING: Not thread safe. This function must only be called from the main
    // thread.
    static QSharedPointer<VisualPlayPosition> getVisualPlayPosition(QString group);

    // This is called by SoundDevicePortAudio just after the callback starts.
    static void setTimeInfo(const PaStreamCallbackTimeInfo *timeInfo);

    void setInvalid() { m_valid = false; };

  private slots:
    void slotAudioBufferSizeChanged(double size);

  private:
    ControlValueAtomic<VisualPlayPositionData> m_data;
    ControlObjectSlave* m_audioBufferSize;
    double m_dAudioBufferSize; // Audio buffer size in ms
    bool m_valid;
    QString m_key;
    bool m_invalidTimeInfoWarned;

    static QMap<QString, QWeakPointer<VisualPlayPosition> > m_listVisualPlayPosition;
    // Time info from the Sound device, updated just after audio callback is called
    static PaStreamCallbackTimeInfo m_timeInfo;
    // Time stamp for m_timeInfo in main CPU time
    static PerformanceTimer m_timeInfoTime;
};

#endif // VISUALPLAYPOSITION_H
