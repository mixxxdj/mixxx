#ifndef VISUALPLAYPOSITION_H
#define VISUALPLAYPOSITION_H

#include <QMutex>
#include <QTime>
#include <QMap>
#include <QAtomicPointer>

#include "util/performancetimer.h"
#include "control/controlvalue.h"

class ControlProxy;
class VSyncThread;

// This class is for synchronizing the sound device DAC time with the waveforms, displayed on the
// graphic device, using the CPU time
//
// DAC: ------|--------------|-------|-------------------|-----------------------|-----
//            ^Audio Callback Entry  |                   |                       ^Last Sample to DAC
//            |              ^Buffer prepared            ^Waveform sample X
//            |                      ^First sample transferred to DAC
// CPU: ------|-------------------------------------------------------------------------
//            ^Start m_timeInfoTime                      |
//                                                       |
// GPU: ---------|----------------------------------- |--|-------------------------------
//               ^Render Waveform sample X            |  ^VSync (New waveform is displayed
//                by use usFromTimerToNextSync        ^swap Buffer

class VisualPlayPositionData {
  public:
    PerformanceTimer m_referenceTime;
    int m_callbackEntrytoDac; // Time from Audio Callback Entry to first sample of Buffer is transferred to DAC
    double m_enginePlayPos; // Play position of fist Sample in Buffer
    double m_rate;
    double m_positionStep;
    double m_slipPosition;
    double m_tempoTrackSeconds; // total track time, taking the current tempo into account
};


class VisualPlayPosition : public QObject {
    Q_OBJECT
  public:
    VisualPlayPosition(const QString& m_key);
    virtual ~VisualPlayPosition();

    // WARNING: Not thread safe. This function must be called only from the
    // engine thread.
    void set(double playPos, double rate, double positionStep,
            double slipPosition, double tempoTrackSeconds);
    double getAtNextVSync(VSyncThread* vsyncThread);
    void getPlaySlipAtNextVSync(VSyncThread* vSyncThread, double* playPosition, double* slipPosition);
    double getEnginePlayPos();
    void getTrackTime(double* pPlayPosition, double* pTempoTrackSeconds);

    // WARNING: Not thread safe. This function must only be called from the main
    // thread.
    static QSharedPointer<VisualPlayPosition> getVisualPlayPosition(QString group);

    // This is called by SoundDevicePortAudio just after the callback starts.
    static void setCallbackEntryToDacSecs(double secs, const PerformanceTimer& time);

    void setInvalid() { m_valid = false; };

  private slots:
    void slotAudioBufferSizeChanged(double sizeMs);

  private:
    ControlValueAtomic<VisualPlayPositionData> m_data;
    ControlProxy* m_audioBufferSize;
    int m_audioBufferMicros; // Audio buffer size in µs
    bool m_valid;
    QString m_key;

    static QMap<QString, QWeakPointer<VisualPlayPosition> > m_listVisualPlayPosition;
    // Time info from the Sound device, updated just after audio callback is called
    static double m_dCallbackEntryToDacSecs;
    // Time stamp for m_timeInfo in main CPU time
    static PerformanceTimer m_timeInfoTime;
};

#endif // VISUALPLAYPOSITION_H
