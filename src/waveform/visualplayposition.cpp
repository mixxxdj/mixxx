#include "waveform/visualplayposition.h"

#include <QtDebug>

#include "control/controlproxy.h"
#include "control/controlobject.h"
#include "util/math.h"
#include "waveform/vsyncthread.h"

//static
QMap<QString, QWeakPointer<VisualPlayPosition> > VisualPlayPosition::m_listVisualPlayPosition;
PerformanceTimer VisualPlayPosition::m_timeInfoTime;
double VisualPlayPosition::m_dCallbackEntryToDacSecs = 0;

VisualPlayPosition::VisualPlayPosition(const QString& key)
        : m_valid(false),
          m_key(key) {
    m_audioBufferSize = new ControlProxy(
            "[Master]", "audio_buffer_size", this);
    m_audioBufferSize->connectValueChanged(
            SLOT(slotAudioBufferSizeChanged(double)));
    m_dAudioBufferSize = m_audioBufferSize->get();
}

VisualPlayPosition::~VisualPlayPosition() {
    m_listVisualPlayPosition.remove(m_key);
}

void VisualPlayPosition::set(double playPos, double rate,
                             double positionStep, double pSlipPosition) {
    VisualPlayPositionData data;
    data.m_referenceTime = m_timeInfoTime;
    data.m_callbackEntrytoDac = m_dCallbackEntryToDacSecs * 1000000; // s to µs
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
        int usRefToVSync = vsyncThread->usFromTimerToNextSync(data.m_referenceTime);
        int offset = usRefToVSync - data.m_callbackEntrytoDac;
        double playPos = data.m_enginePlayPos;  // load playPos for the first sample in Buffer
        // add the offset for the position of the sample that will be transfered to the DAC
        // When the next display frame is displayed
        playPos += data.m_positionStep * offset * data.m_rate / m_dAudioBufferSize / 1000;
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
        int usElapsed = data.m_referenceTime.elapsed().toIntegerMicros();
        int dacFromNow = usElapsed - data.m_callbackEntrytoDac;
        int offset = dacFromNow - usFromNow;
        double playPos = data.m_enginePlayPos;  // load playPos for the first sample in Buffer
        playPos += data.m_positionStep * offset * data.m_rate / m_dAudioBufferSize / 1000;
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

void VisualPlayPosition::slotAudioBufferSizeChanged(double size) {
    m_dAudioBufferSize = size;
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
void VisualPlayPosition::setCallbackEntryToDacSecs(double secs, const PerformanceTimer& time) {
    // the time is valid only just NOW, so measure the time from NOW for
    // later correction
    m_timeInfoTime = time;
    m_dCallbackEntryToDacSecs = secs;
}
