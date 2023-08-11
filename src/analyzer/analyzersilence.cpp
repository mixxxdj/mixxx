#include "analyzer/analyzersilence.h"

#include "analyzer/constants.h"
#include "engine/engine.h"
#include "track/track.h"

namespace {

constexpr CSAMPLE kSilenceThreshold = 0.001f;
// TODO: Change the above line to:
//constexpr CSAMPLE kSilenceThreshold = db2ratio(-60.0f);

bool shouldAnalyze(TrackPointer pTrack) {
    CuePointer pIntroCue = pTrack->findCueByType(mixxx::CueType::Intro);
    CuePointer pOutroCue = pTrack->findCueByType(mixxx::CueType::Outro);
    CuePointer pAudibleSound = pTrack->findCueByType(mixxx::CueType::AudibleSound);

    if (!pIntroCue || !pOutroCue || !pAudibleSound || pAudibleSound->getLength() <= 0) {
        return true;
    }
    return false;
}

} // anonymous namespace

AnalyzerSilence::AnalyzerSilence(UserSettingsPointer pConfig)
        : m_pConfig(pConfig),
          m_fThreshold(kSilenceThreshold),
          m_framesProcessed(0),
          m_bPrevSilence(true),
          m_signalStart(-1),
          m_signalEnd(-1) {
}

bool AnalyzerSilence::initialize(TrackPointer pTrack,
        mixxx::audio::SampleRate sampleRate,
        SINT frameLength) {
    Q_UNUSED(sampleRate);
    Q_UNUSED(frameLength);

    if (!shouldAnalyze(pTrack)) {
        return false;
    }

    m_framesProcessed = 0;
    m_bPrevSilence = true;
    m_signalStart = -1;
    m_signalEnd = -1;

    return true;
}

bool AnalyzerSilence::processSamples(const CSAMPLE* pIn, SINT count) {
    for (int i = 0; i < count; i += mixxx::kAnalysisChannels) {
        // Compute max of channels in this sample frame
        CSAMPLE fMax = CSAMPLE_ZERO;
        for (SINT ch = 0; ch < mixxx::kAnalysisChannels; ++ch) {
            CSAMPLE fAbs = fabs(pIn[i + ch]);
            fMax = math_max(fMax, fAbs);
        }

        bool bSilence = fMax < m_fThreshold;

        if (m_bPrevSilence && !bSilence) {
            if (m_signalStart < 0) {
                m_signalStart = m_framesProcessed + i / mixxx::kAnalysisChannels;
            }
        } else if (!m_bPrevSilence && bSilence) {
            m_signalEnd = m_framesProcessed + i / mixxx::kAnalysisChannels;
        }

        m_bPrevSilence = bSilence;
    }
    m_framesProcessed += count / mixxx::kAnalysisChannels;
    return true;
}

void AnalyzerSilence::cleanup() {
}

void AnalyzerSilence::storeResults(TrackPointer pTrack) {
    if (m_signalStart < 0) {
        m_signalStart = 0;
    }
    if (m_signalEnd < 0) {
        m_signalEnd = m_framesProcessed;
    }

    // If track didn't end with silence, place signal end marker
    // on the end of the track.
    if (!m_bPrevSilence) {
        m_signalEnd = m_framesProcessed;
    }

    double firstSound = mixxx::kAnalysisChannels * m_signalStart;
    double lastSound = mixxx::kAnalysisChannels * m_signalEnd;

    CuePointer pAudibleSound = pTrack->findCueByType(mixxx::CueType::AudibleSound);
    if (pAudibleSound == nullptr) {
        pAudibleSound = pTrack->createAndAddCue();
        pAudibleSound->setType(mixxx::CueType::AudibleSound);
    }
    // The user has no way to directly edit the AudibleSound cue. If the user
    // has deleted the Intro or Outro Cue, this analysis will be rerun when
    // the track is loaded again. In this case, adjust the AudibleSound Cue's
    // positions. This could be helpful, for example, when the track length
    // is changed in a different program, or the silence detection threshold
    // is changed.
    pAudibleSound->setStartPosition(firstSound);
    pAudibleSound->setEndPosition(lastSound);

    setupMainAndIntroCue(pTrack.get(), firstSound, m_pConfig.data());
    setupOutroCue(pTrack.get(), lastSound);
}

// static
void AnalyzerSilence::setupMainAndIntroCue(
        Track* pTrack, double firstSound, UserSettings* pConfig) {
    CuePointer pIntroCue = pTrack->findCueByType(mixxx::CueType::Intro);

    double mainCue = pTrack->getCuePoint().getPosition();
    double introStart = firstSound;
    // Before Mixxx 2.3, the default position for the main cue was 0.0. In this
    // case, move the main cue point to the first sound. This case can be
    // distinguished from a user intentionally setting the main cue position
    // to 0.0 at a later time after analysis because in that case the intro cue
    // would have already been created by this analyzer.
    bool upgradingWithMainCueAtDefault = (mainCue == 0.0 && pIntroCue == nullptr);
    if (mainCue == Cue::kNoPosition || upgradingWithMainCueAtDefault) {
        pTrack->setCuePoint(CuePosition(firstSound));
        // NOTE: the actual default for this ConfigValue is set in DlgPrefDeck.
    } else if (pConfig->getValue(ConfigKey("[Controls]", "SetIntroStartAtMainCue"), false) &&
            pIntroCue == nullptr) {
        introStart = mainCue;
    }

    if (pIntroCue == nullptr) {
        pIntroCue = pTrack->createAndAddCue();
        pIntroCue->setType(mixxx::CueType::Intro);
        pIntroCue->setStartPosition(introStart);
        pIntroCue->setEndPosition(Cue::kNoPosition);
    }
}

// static
void AnalyzerSilence::setupOutroCue(Track* pTrack, double lastSound) {
    CuePointer pOutroCue = pTrack->findCueByType(mixxx::CueType::Outro);
    if (pOutroCue == nullptr) {
        pOutroCue = pTrack->createAndAddCue();
        pOutroCue->setType(mixxx::CueType::Outro);
        pOutroCue->setStartPosition(Cue::kNoPosition);
        pOutroCue->setEndPosition(lastSound);
    }
}
