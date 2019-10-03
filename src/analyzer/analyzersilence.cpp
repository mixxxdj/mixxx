#include "analyzer/analyzersilence.h"

#include "analyzer/constants.h"
#include "engine/engine.h"

namespace {

constexpr float kSilenceThreshold = 0.001;
// TODO: Change the above line to:
//constexpr float kSilenceThreshold = db2ratio(-60.0f);

bool shouldUpdateMainCue(CuePosition mainCue) {
    return mainCue.getSource() != Cue::MANUAL ||
            mainCue.getPosition() == -1.0 ||
            mainCue.getPosition() == 0.0;
}

bool hasIntroCueStart(const Cue& introCue) {
    return introCue.getPosition() != -1.0;
}

bool hasOutroCueEnd(const Cue& outroCue) {
    return outroCue.getEndPosition() > 0.0;
}

bool needsIntroCueStart(const Cue& introCue) {
    return introCue.getSource() != Cue::MANUAL &&
            !hasIntroCueStart(introCue);
}

bool needsOutroCueEnd(const Cue& outroCue) {
    return outroCue.getSource() != Cue::MANUAL &&
            !hasOutroCueEnd(outroCue);
}

bool shouldAnalyze(TrackPointer tio) {
    CuePointer pIntroCue = tio->findCueByType(Cue::INTRO);
    if (!pIntroCue) {
        return true;
    }
    CuePointer pOutroCue = tio->findCueByType(Cue::OUTRO);
    if (!pOutroCue) {
        return true;
    }
    return needsIntroCueStart(*pIntroCue) || needsOutroCueEnd(*pOutroCue);
}

} // anonymous namespace

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

    if (!shouldAnalyze(tio)) {
        return false;
    }

    m_iFramesProcessed = 0;
    m_bPrevSilence = true;
    m_iSignalStart = -1;
    m_iSignalEnd = -1;

    return true;
}

bool AnalyzerSilence::processSamples(const CSAMPLE* pIn, const int iLen) {
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
    return true;
}

void AnalyzerSilence::cleanup() {
}

void AnalyzerSilence::storeResults(TrackPointer tio) {
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

    double introStart = mixxx::kAnalysisChannels * m_iSignalStart;
    double outroEnd = mixxx::kAnalysisChannels * m_iSignalEnd;

    if (shouldUpdateMainCue(tio->getCuePoint())) {
        tio->setCuePoint(CuePosition(introStart, Cue::AUTOMATIC));
    }

    CuePointer pIntroCue = tio->findCueByType(Cue::INTRO);
    if (!pIntroCue) {
        pIntroCue = tio->createAndAddCue();
        pIntroCue->setType(Cue::INTRO);
        pIntroCue->setSource(Cue::AUTOMATIC);
    }
    if (pIntroCue->getSource() != Cue::MANUAL) {
        pIntroCue->setPosition(introStart);
        pIntroCue->setLength(0.0);
    }

    CuePointer pOutroCue = tio->findCueByType(Cue::OUTRO);
    if (!pOutroCue) {
        pOutroCue = tio->createAndAddCue();
        pOutroCue->setType(Cue::OUTRO);
        pOutroCue->setSource(Cue::AUTOMATIC);
    }
    if (pOutroCue->getSource() != Cue::MANUAL) {
        pOutroCue->setPosition(-1.0);
        pOutroCue->setLength(outroEnd);
    }
}
