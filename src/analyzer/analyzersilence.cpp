#include "analyzer/analyzersilence.h"

#include "analyzer/constants.h"
#include "engine/engine.h"
#include "track/track.h"

namespace {

// This threshold must not be changed, because this value is also used to
// verify that the track samples have not changed since the last analysis
constexpr CSAMPLE kSilenceThreshold = 0.001f; // -60 dB
// TODO: Change the above line to:
//constexpr CSAMPLE kSilenceThreshold = db2ratio(-60.0f);

bool shouldAnalyze(TrackPointer pTrack) {
    CuePointer pIntroCue = pTrack->findCueByType(mixxx::CueType::Intro);
    CuePointer pOutroCue = pTrack->findCueByType(mixxx::CueType::Outro);
    CuePointer pAudibleSound = pTrack->findCueByType(mixxx::CueType::AudibleSound);

    if (!pIntroCue || !pOutroCue || !pAudibleSound || pAudibleSound->getLengthFrames() <= 0) {
        return true;
    }
    return false;
}

} // anonymous namespace

AnalyzerSilence::AnalyzerSilence(UserSettingsPointer pConfig)
        : m_pConfig(pConfig),
          m_iFramesProcessed(0),
          m_iSignalStart(-1),
          m_iSignalEnd(-1) {
}

bool AnalyzerSilence::initialize(TrackPointer pTrack,
        mixxx::audio::SampleRate sampleRate,
        int totalSamples) {
    Q_UNUSED(sampleRate);
    Q_UNUSED(totalSamples);

    if (!shouldAnalyze(pTrack)) {
        return false;
    }

    m_iFramesProcessed = 0;
    m_iSignalStart = -1;
    m_iSignalEnd = -1;

    return true;
}

// static
SINT AnalyzerSilence::findFirstSoundInChunk(const CSAMPLE* pIn, SINT iLen) {
    SINT i = 0;
    for (; i < iLen; ++i) {
        if (fabs(pIn[i]) >= kSilenceThreshold) {
            break;
        }
    }
    return i;
}

// static
SINT AnalyzerSilence::findLastSoundInChunk(const CSAMPLE* pIn, SINT iLen) {
    SINT i = iLen - 1;
    for (; i >= 0; --i) {
        if (fabs(pIn[i]) >= kSilenceThreshold) {
            break;
        }
    }
    return i;
}

// static
bool AnalyzerSilence::verifyFirstSound(
        const CSAMPLE* pIn, SINT iLen, mixxx::audio::FramePos firstSoundFrame) {
    const SINT firstSoundSample = findFirstSoundInChunk(pIn, iLen);
    if (firstSoundSample < iLen) {
        return (mixxx::audio::FramePos(findFirstSoundInChunk(pIn, iLen) /
                        mixxx::kAnalysisChannels) == firstSoundFrame);
    }
    return false;
}

bool AnalyzerSilence::processSamples(const CSAMPLE* pIn, SINT iLen) {
    if (m_iSignalStart < 0) {
        const SINT firstSoundSample = findFirstSoundInChunk(pIn, iLen);
        if (firstSoundSample < iLen) {
            m_iSignalStart = m_iFramesProcessed + firstSoundSample / mixxx::kAnalysisChannels;
        }
    }
    if (m_iSignalStart >= 0) {
        const SINT lastSoundSample = findLastSoundInChunk(pIn, iLen);
        if (lastSoundSample > -1 &&           // only silence
                lastSoundSample < iLen - 1) { // only sound
            m_iSignalEnd = m_iFramesProcessed + lastSoundSample / mixxx::kAnalysisChannels + 1;
        }
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

    const auto firstSoundPosition = mixxx::audio::FramePos(m_iSignalStart);
    const auto lastSoundPosition = mixxx::audio::FramePos(m_iSignalEnd);

    CuePointer pAudibleSound = pTrack->findCueByType(mixxx::CueType::AudibleSound);
    if (pAudibleSound == nullptr) {
        pAudibleSound = pTrack->createAndAddCue(
                mixxx::CueType::AudibleSound,
                Cue::kNoHotCue,
                firstSoundPosition,
                lastSoundPosition);
    } else {
        // The user has no way to directly edit the AudibleSound cue. If the user
        // has deleted the Intro or Outro Cue, this analysis will be rerun when
        // the track is loaded again. In this case, adjust the AudibleSound Cue's
        // positions. This could be helpful, for example, when the track length
        // is changed in a different program, or the silence detection threshold
        // is changed.
        pAudibleSound->setStartAndEndPosition(firstSoundPosition, lastSoundPosition);
    }

    CuePointer pIntroCue = pTrack->findCueByType(mixxx::CueType::Intro);

    mixxx::audio::FramePos mainCuePosition = pTrack->getMainCuePosition();
    mixxx::audio::FramePos introStartPosition = firstSoundPosition;
    // Before Mixxx 2.3, the default position for the main cue was 0.0. In this
    // case, move the main cue point to the first sound. This case can be
    // distinguished from a user intentionally setting the main cue position
    // to 0.0 at a later time after analysis because in that case the intro cue
    // would have already been created by this analyzer.
    bool upgradingWithMainCueAtDefault =
            (mainCuePosition == mixxx::audio::kStartFramePos &&
                    pIntroCue == nullptr);
    if (!mainCuePosition.isValid() || upgradingWithMainCueAtDefault) {
        pTrack->setMainCuePosition(firstSoundPosition);
        // NOTE: the actual default for this ConfigValue is set in DlgPrefDeck.
    } else if (m_pConfig->getValue(ConfigKey("[Controls]", "SetIntroStartAtMainCue"), false) &&
            pIntroCue == nullptr) {
        introStartPosition = mainCuePosition;
    }

    if (pIntroCue == nullptr) {
        pIntroCue = pTrack->createAndAddCue(
                mixxx::CueType::Intro,
                Cue::kNoHotCue,
                introStartPosition,
                mixxx::audio::kInvalidFramePos);
    }

    CuePointer pOutroCue = pTrack->findCueByType(mixxx::CueType::Outro);
    if (pOutroCue == nullptr) {
        pOutroCue = pTrack->createAndAddCue(
                mixxx::CueType::Outro,
                Cue::kNoHotCue,
                mixxx::audio::kInvalidFramePos,
                lastSoundPosition);
    }
}
