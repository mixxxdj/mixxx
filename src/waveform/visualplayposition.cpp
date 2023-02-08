#include "waveform/visualplayposition.h"

#include <QtDebug>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "moc_visualplayposition.cpp"
#include "util/math.h"
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include "waveform/vsyncthread.h"
#endif

//static
QMap<QString, QWeakPointer<VisualPlayPosition>> VisualPlayPosition::m_listVisualPlayPosition;
PerformanceTimer VisualPlayPosition::m_timeInfoTime;
double VisualPlayPosition::m_dCallbackEntryToDacSecs = 0;

VisualPlayPosition::VisualPlayPosition(const QString& key)
        : m_valid(false),
          m_key(key) {
}

VisualPlayPosition::~VisualPlayPosition() {
    m_listVisualPlayPosition.remove(m_key);
}

void VisualPlayPosition::set(
        double playPos,
        double rate,
        double positionStep,
        double slipPosition,
        double tempoTrackSeconds,
        double audioBufferMicroS) {
    VisualPlayPositionData data;
    data.m_referenceTime = m_timeInfoTime;
    data.m_callbackEntrytoDac = static_cast<int>(m_dCallbackEntryToDacSecs * 1000000); // s to µs
    data.m_enginePlayPos = playPos;
    data.m_rate = rate;
    data.m_positionStep = positionStep;
    data.m_slipPosition = slipPosition;
    data.m_tempoTrackSeconds = tempoTrackSeconds;
    data.m_audioBufferMicroS = audioBufferMicroS;

    // Atomic write
    m_data.setValue(data);
    m_valid = true;
}

double VisualPlayPosition::calcPosAtNextVSync(
        VSyncThread* pVSyncThread, const VisualPlayPositionData& data) {
    double playPos = data.m_enginePlayPos; // load playPos for the first sample in Buffer
    if (data.m_audioBufferMicroS != 0.0) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        Q_UNUSED(vSyncThread);
        int refToVSync = 0;
#else
        int refToVSync = pVSyncThread->fromTimerToNextSyncMicros(data.m_referenceTime);
#endif
        int offset = refToVSync - data.m_callbackEntrytoDac;
        // The offset is limited to the audio buffer + waveform sync interval
        // This should be sufficient to compensate jitter, but does not continue
        // in case of underflows.
        int maxOffset = static_cast<int>(data.m_audioBufferMicroS +
                pVSyncThread->getSyncIntervalTimeMicros());
        offset = math_clamp(offset, -maxOffset, maxOffset);
        // add the offset for the position of the sample that will be transferred to the DAC
        // When the next display frame is displayed
        playPos += data.m_positionStep * offset * data.m_rate / data.m_audioBufferMicroS;
    }
    // qDebug() << "playPos" << playPos << offset;
    return playPos;
}

double VisualPlayPosition::getAtNextVSync(VSyncThread* pVSyncThread) {
    if (m_valid) {
        VisualPlayPositionData data = m_data.getValue();
        return calcPosAtNextVSync(pVSyncThread, data);
    }
    return -1;
}

void VisualPlayPosition::getPlaySlipAtNextVSync(VSyncThread* pVSyncThread,
        double* pPlayPosition,
        double* pSlipPosition) {
    if (m_valid) {
        VisualPlayPositionData data = m_data.getValue();
        *pPlayPosition = calcPosAtNextVSync(pVSyncThread, data);
        *pSlipPosition = data.m_slipPosition;
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

void VisualPlayPosition::getTrackTime(double* pPlayPosition, double* pTempoTrackSeconds) {
    if (m_valid) {
        VisualPlayPositionData data = m_data.getValue();
        *pPlayPosition = data.m_enginePlayPos;
        *pTempoTrackSeconds = data.m_tempoTrackSeconds;
    } else {
        *pPlayPosition = 0;
        *pTempoTrackSeconds = 0;
    }
}

//static
QSharedPointer<VisualPlayPosition> VisualPlayPosition::getVisualPlayPosition(const QString& group) {
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
