
#include <qdebug.h>

#include "visualplayposition.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "waveform/waveformwidgetfactory.h"
#include "mathstuff.h"
#include "waveform/vsyncthread.h"


//static
QMap<QString, VisualPlayPosition*> VisualPlayPosition::m_listVisualPlayPosition;
const PaStreamCallbackTimeInfo* VisualPlayPosition::m_timeInfo;
PerformanceTimer VisualPlayPosition::m_timeInfoTime;

VisualPlayPosition::VisualPlayPosition() :
        m_valid(false) {

    m_audioBufferSize = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey("[Master]","audio_buffer_size")));
}

VisualPlayPosition::~VisualPlayPosition() {

}

// This function must be called only form the engine thread (PA callback)
void VisualPlayPosition::set(double playPos, double rate,
                double positionStep, double pSlipPosition) {
    VisualPlayPositionData data;

    data.m_referenceTime = m_timeInfoTime;
    // Time from reference time to Buffer at DAC in µs
    data.m_timeDac = (m_timeInfo->outputBufferDacTime - m_timeInfo->currentTime) * 1000000;
    data.m_playPos = playPos;
    data.m_rate = rate;
    data.m_positionStep = positionStep;
    data.m_pSlipPosition = pSlipPosition;

    // Atomic write
    m_data.setValue(data);
    m_valid = true;
}

double VisualPlayPosition::getAt(VSyncThread* vsyncThread) {
    //static double testPos = 0;
    //testPos += 0.000017759; //0.000016608; //  1.46257e-05;
    //return testPos;

    if (m_valid) {
        VisualPlayPositionData data = m_data.getValue();
        int usRefToVSync = vsyncThread->usFromTimerToNextSync(&data.m_referenceTime);
        int offset = usRefToVSync - data.m_timeDac;
        double playPos = data.m_playPos;  // load playPos for the first sample in Buffer
        playPos += data.m_positionStep * offset * data.m_rate / m_audioBufferSize->get() / 1000;
        //qDebug() << "delta Pos" << playPos - m_playPosOld << offset;
        m_playPosOld = playPos;
        return playPos;
    }
    return -1;
}

void VisualPlayPosition::getPlaySlipAt(int usFromNow, double* playPosition, double* slipPosition) {
    //static double testPos = 0;
    //testPos += 0.000017759; //0.000016608; //  1.46257e-05;
    //return testPos;

    if (m_valid) {
        VisualPlayPositionData data = m_data.getValue();
        int usElapsed = data.m_referenceTime.elapsed() / 1000;
        int dacFromNow = usElapsed - data.m_timeDac;
        int offset = dacFromNow - usFromNow;
        double playPos = data.m_playPos;  // load playPos for the first sample in Buffer
        playPos += data.m_positionStep * offset * data.m_rate / m_audioBufferSize->get() / 1000;
        *playPosition = playPos;
        *slipPosition = data.m_pSlipPosition;
    }
}

double VisualPlayPosition::getEnginePlayPos() {
    if (m_valid) {
        VisualPlayPositionData data = m_data.getValue();
        return data.m_playPos;
    } else {
        return -1;
    }
}

//static
VisualPlayPosition* VisualPlayPosition::getVisualPlayPosition(QString group) {
    VisualPlayPosition* vpp = m_listVisualPlayPosition[group];
    if (!vpp) {
        vpp = new VisualPlayPosition();
        m_listVisualPlayPosition[group] = vpp;
    }
    return vpp;
}

//static
void VisualPlayPosition::setTimeInfo(const PaStreamCallbackTimeInfo *timeInfo) {
    m_timeInfo = timeInfo;
    m_timeInfoTime.start();
    //qDebug() << "TimeInfo" << (timeInfo->currentTime - floor(timeInfo->currentTime)) << (timeInfo->outputBufferDacTime - floor(timeInfo->outputBufferDacTime));
    //m_timeInfo.currentTime = timeInfo->currentTime;
    //m_timeInfo.inputBufferAdcTime = timeInfo->inputBufferAdcTime;
    //m_timeInfo.outputBufferDacTime = timeInfo->outputBufferDacTime;
}

