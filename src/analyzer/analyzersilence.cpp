#include "analyzer/analyzersilence.h"

#include "analyzer/constants.h"
#include "engine/engine.h"

namespace {

constexpr float kSilenceThreshold = 0.001;
// TODO: Change the above line to:
//constexpr float kSilenceThreshold = db2ratio(-60.0f);

const double kCueNotSet = -1.0;

bool shouldAnalyze(TrackPointer pTrack) {
    CuePointer pIntroCue = pTrack->findCueByType(Cue::Type::Intro);
    CuePointer pOutroCue = pTrack->findCueByType(Cue::Type::Outro);
    CuePointer pAudibleSound = pTrack->findCueByType(Cue::Type::AudibleSound);

    if (!pIntroCue || !pOutroCue || !pAudibleSound || pAudibleSound->getLength() <= 0) {
        return true;
    }
    return false;
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

    double firstSound = mixxx::kAnalysisChannels * m_iSignalStart;
    double lastSound = mixxx::kAnalysisChannels * m_iSignalEnd;

    CuePointer pAudibleSound = pTrack->findCueByType(Cue::Type::AudibleSound);
    if (pAudibleSound == nullptr) {
        pAudibleSound = pTrack->createAndAddCue();
        pAudibleSound->setType(Cue::Type::AudibleSound);
        pAudibleSound->setPosition(firstSound);
        pAudibleSound->setLength(lastSound - firstSound);
    }

    CuePointer pIntroCue = pTrack->findCueByType(Cue::Type::Intro);

    double mainCue = pTrack->getCuePoint().getPosition();
    double introStart = firstSound;
    if (mainCue == kCueNotSet) {
        pTrack->setCuePoint(CuePosition(firstSound));
    // NOTE: the actual default for this ConfigValue is set in DlgPrefDeck.
    } else if (m_pConfig->getValue(ConfigKey("[Controls]", "SetIntroStartAtMainCue"), false)
            && pIntroCue == nullptr && mainCue != 0.0) {
        introStart = mainCue;
    }

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
        pOutroCue->setLength(lastSound);
    }
}
