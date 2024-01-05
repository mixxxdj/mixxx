#include "waveform/visualplayposition.h"

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
          m_key(key),
          m_noTransport(false) {
}

VisualPlayPosition::~VisualPlayPosition() {
    m_listVisualPlayPosition.remove(m_key);
}

void VisualPlayPosition::set(
        double playPosition,
        double playRate,
        double positionStep,
        double slipPosition,
        double slipRate,
        SlipModeState m_slipModeState,
        bool loopEnabled,
        double loopStartPosition,
        double loopEndPosition,
        double tempoTrackSeconds,
        double audioBufferMicroS) {
    VisualPlayPositionData data;
    data.m_referenceTime = m_timeInfoTime;
    data.m_callbackEntrytoDac = static_cast<int>(m_dCallbackEntryToDacSecs * 1000000); // s to µs
    data.m_playPos = playPosition;
    data.m_playRate = playRate;
    data.m_slipRate = slipRate;
    data.m_positionStep = positionStep;
    data.m_slipPos = slipPosition;
    data.m_slipModeState = m_slipModeState;
    data.m_loopEnabled = loopEnabled;
    data.m_loopStartPos = loopStartPosition;
    data.m_loopEndPos = loopEndPosition;
    data.m_tempoTrackSeconds = tempoTrackSeconds;
    data.m_audioBufferMicroS = audioBufferMicroS;

    // Atomic write
    m_data.setValue(data);
    m_valid = true;
}

double VisualPlayPosition::calcOffsetAtNextVSync(
        VSyncThread* pVSyncThread, const VisualPlayPositionData& data) {
    if (data.m_audioBufferMicroS != 0.0) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        Q_UNUSED(pVSyncThread);
        const int refToVSync = 0;
        const int syncIntervalTimeMicros = 0;
#else
        const int refToVSync = pVSyncThread->fromTimerToNextSyncMicros(data.m_referenceTime);
        const int syncIntervalTimeMicros = pVSyncThread->getSyncIntervalTimeMicros();
#endif
        // The positive offset is limited to the audio buffer + 2 x waveform sync interval
        // This should be sufficient to compensate jitter, but does not continue
        // in case of underflows.
        const int maxOffset = static_cast<int>(
                data.m_audioBufferMicroS + 2 * syncIntervalTimeMicros);

        // The negative offset is limited to -data.m_callbackEntrytoDac to not
        // More negative values indicated an outdated request, that is anyway no
        // longer valid. Probably cause bay a vsync issue.
        const int minOffset = -data.m_callbackEntrytoDac;

        // Calculate the offset in micros for the position of the sample that will be transferred
        // to the DAC when the next display frame is displayed
        int offset = refToVSync - data.m_callbackEntrytoDac;
        if (offset < minOffset) {
            offset = minOffset;
            if (!m_noTransport) {
                qWarning() << "VisualPlayPosition::calcOffsetAtNextVSync"
                           << m_key << "outdated position request (offset < minOffset)";
                qDebug() << m_key << "refToVSync:" << refToVSync
                         << "data.m_callbackEntrytoDac:"
                         << data.m_callbackEntrytoDac;
                m_noTransport = true;
            }
        } else if (offset > maxOffset) {
            offset = maxOffset;
            if (!m_noTransport) {
                qWarning() << "VisualPlayPosition::calcOffsetAtNextVSync"
                           << m_key << "no transport (offset > maxOffset)";
                qDebug() << m_key << "refToVSync:" << refToVSync
                         << "data.m_callbackEntrytoDac:"
                         << data.m_callbackEntrytoDac;
                m_noTransport = true;
            }
        } else {
            if (m_noTransport) {
                qDebug() << m_key << "refToVSync:" << refToVSync
                         << "data.m_callbackEntrytoDac:"
                         << data.m_callbackEntrytoDac;
            }
            m_noTransport = false;
        }
        // Apply the offset proportional to m_positionStep
        return data.m_positionStep * static_cast<double>(offset) / data.m_audioBufferMicroS;
    }
    return 0.0;
}

double VisualPlayPosition::determinePlayPosInLoopBoundries(
        const VisualPlayPositionData& data, const double& offset) {
    double interpolatedPlayPos = data.m_playPos + offset * data.m_playRate;

    if (data.m_loopEnabled) {
        double loopSize = data.m_loopEndPos - data.m_loopStartPos;
        if (loopSize > 0) {
            if ((data.m_playRate < 0.0) &&
                    (interpolatedPlayPos < data.m_loopStartPos) &&
                    (data.m_playPos >= data.m_loopStartPos)) {
                // 1. Deck playing reverse
                // 2. Interpolated playposition at the time of next VSync would
                // be outsite of the active loop
                // 3. Playposition is currently inside the active loop
                //    (not scratching left of an activated loop)
                interpolatedPlayPos = data.m_loopEndPos -
                        std::remainder(
                                data.m_loopStartPos - interpolatedPlayPos,
                                loopSize);
            }
            if ((data.m_playRate > 0.0) &&
                    (interpolatedPlayPos > data.m_loopEndPos) &&
                    (data.m_playPos <= data.m_loopEndPos)) {
                // 1. Deck playing forward
                // 2. Interpolated playposition at the time of next VSync would
                // be outsite of the active loop
                // 3. Playposition is currently inside the active loop
                //    (not scratching right of an activated loop)
                interpolatedPlayPos = data.m_loopStartPos +
                        std::remainder(
                                interpolatedPlayPos - data.m_loopEndPos,
                                loopSize);
            }
        }
    }
    return interpolatedPlayPos;
}

double VisualPlayPosition::getAtNextVSync(VSyncThread* pVSyncThread) {
    if (m_valid) {
        const VisualPlayPositionData data = m_data.getValue();
        const double offset = calcOffsetAtNextVSync(pVSyncThread, data);

        return determinePlayPosInLoopBoundries(data, offset);
    }
    return -1;
}

void VisualPlayPosition::getPlaySlipAtNextVSync(VSyncThread* pVSyncThread,
        double* pPlayPosition,
        double* pSlipPosition) {
    if (m_valid) {
        const VisualPlayPositionData data = m_data.getValue();
        const double offset = calcOffsetAtNextVSync(pVSyncThread, data);

        double interpolatedPlayPos = determinePlayPosInLoopBoundries(data, offset);
        *pPlayPosition = interpolatedPlayPos;

        if (data.m_slipModeState == SlipModeState::Running) {
            *pSlipPosition = data.m_slipPos + offset * data.m_slipRate;
        } else {
            *pSlipPosition = interpolatedPlayPos;
        }
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

void VisualPlayPosition::getTrackTime(double* pPlayPosition, double* pTempoTrackSeconds) {
    if (m_valid) {
        VisualPlayPositionData data = m_data.getValue();
        *pPlayPosition = data.m_playPos;
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
