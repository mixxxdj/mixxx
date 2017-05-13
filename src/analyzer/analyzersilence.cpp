#include "analyzer/analyzersilence.h"
#include "util/audiosignal.h"

namespace {
    const SINT kChannelCount = mixxx::AudioSignal::kChannelCountStereo;
    const float kSilenceThresholdDb = -60.0f;
}  // anonymous namespace

AnalyzerSilence::AnalyzerSilence(UserSettingsPointer pConfig)
    : m_pConfig(pConfig),
      m_fThreshold(db2ratio(kSilenceThresholdDb)),
      m_iFramesProcessed(0),
      m_bPrevSilence(true),
      m_iSignalBegin(-1),
      m_iSignalEnd(-1) {
}

AnalyzerSilence::~AnalyzerSilence() {
}

bool AnalyzerSilence::initialize(TrackPointer tio, int sampleRate, int totalSamples) {
    Q_UNUSED(tio);
    Q_UNUSED(sampleRate);
    Q_UNUSED(totalSamples);

    m_iFramesProcessed = 0;
    m_bPrevSilence = true;
    m_iSignalBegin = -1;
    m_iSignalEnd = -1;

    return true;
}

bool AnalyzerSilence::isDisabledOrLoadStoredSuccess(TrackPointer tio) const {
    Q_UNUSED(tio);

    return false;
}

void AnalyzerSilence::process(const CSAMPLE *pIn, const int iLen) {
    for (int i = 0; i < iLen; i += kChannelCount) {
        // Compute max of channels in this sample frame
        CSAMPLE fMax = CSAMPLE_ZERO;
        for (SINT ch = 0; ch < kChannelCount; ++ch) {
            CSAMPLE fAbs = fabs(pIn[i + ch]);
            fMax = math_max(fMax, fAbs);
        }

        bool bSilence = fMax < m_fThreshold;

        if (m_bPrevSilence && !bSilence) {
            if (m_iSignalBegin < 0) {
                m_iSignalBegin = m_iFramesProcessed + i / kChannelCount;
            }
        } else if (!m_bPrevSilence && bSilence) {
            m_iSignalEnd = m_iFramesProcessed + i / kChannelCount;
        }

        m_bPrevSilence = bSilence;
    }

    m_iFramesProcessed += iLen / kChannelCount;
}

void AnalyzerSilence::cleanup(TrackPointer tio) {
    Q_UNUSED(tio);
}

void AnalyzerSilence::finalize(TrackPointer tio) {
    if (m_iSignalBegin < 0) {
        m_iSignalBegin = 0;
    }
    if (m_iSignalEnd < 0) {
        m_iSignalEnd = m_iFramesProcessed;
    }

    // If track didn't end with silence, place signal end marker
    // on the end of the track.
    if (!m_bPrevSilence) {
        m_iSignalEnd = m_iFramesProcessed;
    }

    bool bBeginPointFoundAndSet = false;
    bool bEndPointFoundAndSet = false;
    QList<CuePointer> cues = tio->getCuePoints();
    foreach (CuePointer pCue, cues) {
        if (pCue->getType() == Cue::BEGIN) {
            pCue->setSource(Cue::AUTOMATIC);
            pCue->setHotCue(-1);
            pCue->setLength(0);
            pCue->setPosition(kChannelCount * m_iSignalBegin);
            bBeginPointFoundAndSet = true;
        } else if (pCue->getType() == Cue::END) {
            pCue->setSource(Cue::AUTOMATIC);
            pCue->setHotCue(-1);
            pCue->setLength(0);
            pCue->setPosition(kChannelCount * m_iSignalEnd);
            bEndPointFoundAndSet = true;
        }
    }

    if (!bBeginPointFoundAndSet) {
        CuePointer pCue = tio->createAndAddCue();
        pCue->setSource(Cue::AUTOMATIC);
        pCue->setType(Cue::BEGIN);
        pCue->setHotCue(-1);
        pCue->setLength(0);
        pCue->setPosition(kChannelCount * m_iSignalBegin);
    }

    if (!bEndPointFoundAndSet) {
        CuePointer pCue = tio->createAndAddCue();
        pCue->setSource(Cue::AUTOMATIC);
        pCue->setType(Cue::END);
        pCue->setHotCue(-1);
        pCue->setLength(0);
        pCue->setPosition(kChannelCount * m_iSignalEnd);
    }
}
