#include "analyzer/analyzersilence.h"

#include "analyzer/constants.h"
#include "engine/engine.h"

namespace {

constexpr float kSilenceThreshold = 0.001;
// TODO: Change the above line to:
//constexpr float kSilenceThreshold = db2ratio(-60.0f);

}  // anonymous namespace

AnalyzerSilence::AnalyzerSilence(UserSettingsPointer pConfig)
    : m_pConfig(pConfig),
      m_fThreshold(kSilenceThreshold),
      m_iFramesProcessed(0),
      m_bPrevSilence(true),
      m_iSignalStart(-1),
      m_iSignalEnd(-1) {
}

bool AnalyzerSilence::initialize(TrackPointer tio, int sampleRate, int totalSamples) {
    Q_UNUSED(sampleRate);
    Q_UNUSED(totalSamples);

    m_iFramesProcessed = 0;
    m_bPrevSilence = true;
    m_iSignalStart = -1;
    m_iSignalEnd = -1;

    return !isDisabledOrLoadStoredSuccess(tio);
}

bool AnalyzerSilence::isDisabledOrLoadStoredSuccess(TrackPointer tio) const {
    if (!shouldUpdateCue(tio->getCuePoint())) {
        return false;
    }

    CuePointer pIntroCue = tio->findCueByType(Cue::INTRO);
    if (!pIntroCue || pIntroCue->getSource() != Cue::MANUAL) {
        return false;
    }

    CuePointer pOutroCue = tio->findCueByType(Cue::OUTRO);
    if (!pOutroCue || pOutroCue->getSource() != Cue::MANUAL) {
        return false;
    }

    return true;
}

void AnalyzerSilence::process(const CSAMPLE* pIn, const int iLen) {
    for (int i = 0; i < iLen; i += mixxx::kAnalysisChannels) {
        // Compute max of channels in this sample frame
        CSAMPLE fMax = CSAMPLE_ZERO;
        for (SINT ch = 0; ch < mixxx::kAnalysisChannels; ++ch) {
            CSAMPLE fAbs = fabs(pIn[i + ch]);
            fMax = math_max(fMax, fAbs);
        }

        bool bSilence = fMax < m_fThreshold;

        if (m_bPrevSilence && !bSilence) {
            if (m_iSignalStart < 0) {
                m_iSignalStart = m_iFramesProcessed + i / mixxx::kAnalysisChannels;
            }
        } else if (!m_bPrevSilence && bSilence) {
            m_iSignalEnd = m_iFramesProcessed + i / mixxx::kAnalysisChannels;
        }

        m_bPrevSilence = bSilence;
    }

    m_iFramesProcessed += iLen / mixxx::kAnalysisChannels;
}

void AnalyzerSilence::cleanup(TrackPointer tio) {
    Q_UNUSED(tio);
}

void AnalyzerSilence::finalize(TrackPointer tio) {
    if (m_iSignalStart < 0) {
        m_iSignalStart = 0;
    }
    if (m_iSignalEnd < 0) {
        m_iSignalEnd = m_iFramesProcessed;
    }

    // If track didn't end with silence, place signal end marker
    // on the end of the track.
    if (!m_bPrevSilence) {
        m_iSignalEnd = m_iFramesProcessed;
    }

    if (shouldUpdateCue(tio->getCuePoint())) {
        tio->setCuePoint(CuePosition(mixxx::kAnalysisChannels * m_iSignalStart, Cue::AUTOMATIC));
    }

    CuePointer pIntroCue = tio->findCueByType(Cue::INTRO);
    if (!pIntroCue) {
        pIntroCue = tio->createAndAddCue();
        pIntroCue->setType(Cue::INTRO);
        pIntroCue->setSource(Cue::AUTOMATIC);
        pIntroCue->setPosition(mixxx::kAnalysisChannels * m_iSignalStart);
        pIntroCue->setLength(0.0);
    } else if (pIntroCue->getSource() != Cue::MANUAL) {
        pIntroCue->setPosition(mixxx::kAnalysisChannels * m_iSignalStart);
        pIntroCue->setLength(0.0);
    }

    CuePointer pOutroCue = tio->findCueByType(Cue::OUTRO);
    if (!pOutroCue) {
        pOutroCue = tio->createAndAddCue();
        pOutroCue->setType(Cue::OUTRO);
        pOutroCue->setSource(Cue::AUTOMATIC);
        pOutroCue->setPosition(-1.0);
        pOutroCue->setLength(mixxx::kAnalysisChannels * m_iSignalEnd);
    } else if (pOutroCue->getSource() != Cue::MANUAL) {
        pOutroCue->setPosition(-1.0);
        pOutroCue->setLength(mixxx::kAnalysisChannels * m_iSignalEnd);
    }
}

bool AnalyzerSilence::shouldUpdateCue(CuePosition cue) {
    return cue.getSource() != Cue::MANUAL || cue.getPosition() == -1.0 || cue.getPosition() == 0.0;
}
