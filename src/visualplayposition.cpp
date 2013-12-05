#include <QtDebug>

#include "visualplayposition.h"
#include "controlobjectslave.h"
#include "controlobject.h"
#include "mathstuff.h"
#include "waveform/vsyncthread.h"

//static
QMap<QString, QWeakPointer<VisualPlayPosition> > VisualPlayPosition::m_listVisualPlayPosition;
PaStreamCallbackTimeInfo VisualPlayPosition::m_timeInfo = { 0.0, 0.0, 0.0 };
PerformanceTimer VisualPlayPosition::m_timeInfoTime;

VisualPlayPosition::VisualPlayPosition(const QString& key)
        : m_valid(false),
          m_key(key) {
    m_audioBufferSize = new ControlObjectSlave("[Master]", "audio_buffer_size");
}

VisualPlayPosition::~VisualPlayPosition() {
    m_listVisualPlayPosition.remove(m_key);
    delete m_audioBufferSize;
}

void VisualPlayPosition::set(double playPos, double rate,
                             double positionStep, double pSlipPosition) {
    VisualPlayPositionData data;
    data.m_referenceTime = m_timeInfoTime;
    // Time from reference time to Buffer at DAC in µs
    data.m_callbackEntrytoDac = (m_timeInfo.outputBufferDacTime - m_timeInfo.currentTime) * 1000000;
    data.m_enginePlayPos = playPos;
    data.m_rate = rate;
    data.m_positionStep = positionStep;
    data.m_pSlipPosition = pSlipPosition;

    // Atomic write
    m_data.setValue(data);
    m_valid = true;
}

double VisualPlayPosition::getAtNextVSync(VSyncThread* vsyncThread) {
    //static double testPos = 0;
    //testPos += 0.000017759; //0.000016608; //  1.46257e-05;
    //return testPos;

    if (m_valid) {
        VisualPlayPositionData data = m_data.getValue();
        int usRefToVSync = vsyncThread->usFromTimerToNextSync(&data.m_referenceTime);
        int offset = usRefToVSync - data.m_callbackEntrytoDac;
        double playPos = data.m_enginePlayPos;  // load playPos for the first sample in Buffer
        // add the offset for the position of the sample that will be transfered to the DAC
        // When the next display frame is displayed
        playPos += data.m_positionStep * offset * data.m_rate / m_audioBufferSize->get() / 1000;
        //qDebug() << "delta Pos" << playPos - m_playPosOld << offset;
        //m_playPosOld = playPos;
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
        int dacFromNow = usElapsed - data.m_callbackEntrytoDac;
        int offset = dacFromNow - usFromNow;
        double playPos = data.m_enginePlayPos;  // load playPos for the first sample in Buffer
        playPos += data.m_positionStep * offset * data.m_rate / m_audioBufferSize->get() / 1000;
        *playPosition = playPos;
        *slipPosition = data.m_pSlipPosition;
    }
}

double VisualPlayPosition::getEnginePlayPos() {
    if (m_valid) {
        VisualPlayPositionData data = m_data.getValue();
        return data.m_enginePlayPos;
    } else {
        return -1;
    }
}

//static
QSharedPointer<VisualPlayPosition> VisualPlayPosition::getVisualPlayPosition(QString group) {
    QSharedPointer<VisualPlayPosition> vpp = m_listVisualPlayPosition.value(group);
    if (vpp.isNull()) {
        vpp = QSharedPointer<VisualPlayPosition>(new VisualPlayPosition(group));
        m_listVisualPlayPosition.insert(group, vpp);
    }
    return vpp;
}

//static
void VisualPlayPosition::setTimeInfo(const PaStreamCallbackTimeInfo* timeInfo) {
    // the timeInfo is valid only just NOW, so measure the time from NOW for
    // later correction
    m_timeInfoTime.start();
    m_timeInfo = *timeInfo;
    //qDebug() << "TimeInfo" << (timeInfo->currentTime - floor(timeInfo->currentTime)) << (timeInfo->outputBufferDacTime - floor(timeInfo->outputBufferDacTime));
}
