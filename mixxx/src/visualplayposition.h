#ifndef VISUALPLAYPOSITION_H
#define VISUALPLAYPOSITION_H

#include <portaudio.h>
#include "util/performancetimer.h"
#include "controlobjectbase.h"

#include <QMutex>
#include <QTime>
#include <QMap>
#include <QAtomicPointer>

class ControlObjectThreadMain;
class VSyncThread;

class VisualPlayPositionData {
  public:
    PerformanceTimer m_referenceTime;
    int m_timeDac;
    double m_playPos;
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
    double getAt(VSyncThread* vsyncThread);
    void getPlaySlipAt(int usFromNow, double* playPosition, double* slipPosition);
    double getEnginePlayPos();
    static VisualPlayPosition* getVisualPlayPosition(QString group);
    static void setTimeInfo(const PaStreamCallbackTimeInfo *timeInfo);
    void setInvalid() { m_valid = false; };


  private:
    ControlObjectBase<VisualPlayPositionData> m_data;
    double m_playPosOld;
    int m_deltatime;
    ControlObjectThreadMain* m_audioBufferSize;
    PaTime m_outputBufferDacTime;
    bool m_valid;

    static QMap<QString, VisualPlayPosition*> m_listVisualPlayPosition;
    static const PaStreamCallbackTimeInfo* m_timeInfo;
    static PerformanceTimer m_timeInfoTime;

};

#endif // VISUALPLAYPOSITION_H

