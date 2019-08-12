#include "analyzer/analyzersilence.h"

#include "analyzer/constants.h"
#include "engine/engine.h"

namespace {

constexpr float kSilenceThreshold = 0.001;
// TODO: Change the above line to:
//constexpr float kSilenceThreshold = db2ratio(-60.0f);

const double kCueNotSet = -1.0;

bool shouldUpdateMainCue(CuePosition mainCue) {
    return mainCue.getPosition() == kCueNotSet ||
            mainCue.getPosition() == 0.0;
}

bool hasIntroCueStart(const Cue& introCue) {
    return introCue.getPosition() != kCueNotSet;
}

bool hasOutroCueEnd(const Cue& outroCue) {
    return outroCue.getEndPosition() > 0.0;
}

bool shouldAnalyze(TrackPointer pTrack) {
    CuePointer pIntroCue = pTrack->findCueByType(Cue::Type::Intro);
    CuePointer pOutroCue = pTrack->findCueByType(Cue::Type::Outro);
    CuePointer pAudibleSound = pTrack->findCueByType(Cue::Type::AudibleSound);

    if (!pIntroCue || !pOutroCue || !pAudibleSound || pAudibleSound->getLength() <= 0) {
        return true;
    }
    return !hasIntroCueStart(*pIntroCue) || !hasOutroCueEnd(*pOutroCue);
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

bool AnalyzerSilence::initialize(TrackPointer pTrack, int sampleRate, int totalSamples) {
    Q_UNUSED(sampleRate);
    Q_UNUSED(totalSamples);

    if (!shouldAnalyze(pTrack)) {
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

void AnalyzerSilence::storeResults(TrackPointer pTrack) {
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

    if (shouldUpdateMainCue(pTrack->getCuePoint())) {
        pTrack->setCuePoint(CuePosition(introStart));
    }

    CuePointer pIntroCue = pTrack->findCueByType(Cue::Type::Intro);
    if (!pIntroCue) {
        pIntroCue = pTrack->createAndAddCue();
        pIntroCue->setType(Cue::Type::Intro);
        pIntroCue->setPosition(introStart);
        pIntroCue->setLength(0.0);
    }

    CuePointer pOutroCue = pTrack->findCueByType(Cue::Type::Outro);
    if (!pOutroCue) {
        pOutroCue = pTrack->createAndAddCue();
        pOutroCue->setType(Cue::Type::Outro);
        pOutroCue->setPosition(kCueNotSet);
        pOutroCue->setLength(outroEnd);
    }

    CuePointer pAudibleSound = pTrack->findCueByType(Cue::Type::AudibleSound);
    if (pAudibleSound == nullptr || pAudibleSound->getLength() <= 0) {
        pAudibleSound = pTrack->createAndAddCue();
        pAudibleSound->setType(Cue::Type::AudibleSound);
        pAudibleSound->setPosition(introStart);
        pAudibleSound->setLength(outroEnd - introStart);
    }
}
