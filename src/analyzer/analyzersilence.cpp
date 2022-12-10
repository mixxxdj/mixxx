#include "analyzer/analyzersilence.h"

#include "analyzer/analyzertrack.h"
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
    CuePointer pN60dBSound = pTrack->findCueByType(mixxx::CueType::N60dBSound);

    if (!pIntroCue || !pOutroCue || !pN60dBSound || pN60dBSound->getLengthFrames() <= 0) {
        return true;
    }
    return false;
}

template<typename Iterator>
Iterator first_sound(Iterator begin, Iterator end) {
    return std::find_if(begin, end, [](const auto elem) {
        return fabs(elem) >= kSilenceThreshold;
    });
}

} // anonymous namespace

AnalyzerSilence::AnalyzerSilence(UserSettingsPointer pConfig)
        : m_pConfig(pConfig),
          m_iFramesProcessed(0),
          m_iSignalStart(-1),
          m_iSignalEnd(-1) {
}

bool AnalyzerSilence::initialize(const AnalyzerTrack& track,
        mixxx::audio::SampleRate sampleRate,
        SINT totalSamples) {
    Q_UNUSED(sampleRate);
    Q_UNUSED(totalSamples);

    if (!shouldAnalyze(track.getTrack())) {
        return false;
    }

    m_iFramesProcessed = 0;
    m_iSignalStart = -1;
    m_iSignalEnd = -1;

    return true;
}

// static
SINT AnalyzerSilence::findFirstSoundInChunk(std::span<const CSAMPLE> samples) {
    return std::distance(samples.begin(), first_sound(samples.begin(), samples.end()));
}

// static
/// returns a std::reverse_iterator
SINT AnalyzerSilence::findLastSoundInChunk(std::span<const CSAMPLE> samples) {
    // -1 is required, because the distance from the fist sample index (0) to crend() is 1,
    return std::distance(first_sound(samples.rbegin(), samples.rend()), samples.rend()) - 1;
}

// static
bool AnalyzerSilence::verifyFirstSound(
        std::span<const CSAMPLE> samples,
        mixxx::audio::FramePos firstSoundFrame) {
    const SINT firstSoundSample = findFirstSoundInChunk(samples);
    if (firstSoundSample < static_cast<SINT>(samples.size())) {
        return mixxx::audio::FramePos::fromEngineSamplePos(firstSoundSample) == firstSoundFrame;
    }
    return false;
}

bool AnalyzerSilence::processSamples(const CSAMPLE* pIn, SINT iLen) {
    std::span<const CSAMPLE> samples = mixxx::spanutil::spanFromPtrLen(pIn, iLen);
    if (m_iSignalStart < 0) {
        const SINT firstSoundSample = findFirstSoundInChunk(samples);
        if (firstSoundSample < static_cast<SINT>(samples.size())) {
            m_iSignalStart = m_iFramesProcessed + firstSoundSample / mixxx::kAnalysisChannels;
        }
    }
    if (m_iSignalStart >= 0) {
        const SINT lastSoundSample = findLastSoundInChunk(samples);
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

    CuePointer pN60dBSound = pTrack->findCueByType(mixxx::CueType::N60dBSound);
    if (pN60dBSound == nullptr) {
        pN60dBSound = pTrack->createAndAddCue(
                mixxx::CueType::N60dBSound,
                Cue::kNoHotCue,
                firstSoundPosition,
                lastSoundPosition);
    } else {
        // The user has no way to directly edit the N60dBSound cue. If the user
        // has deleted the Intro or Outro Cue, this analysis will be rerun when
        // the track is loaded again. In this case, adjust the N60dBSound Cue's
        // positions. This could be helpful, for example, when the track length
        // is changed in a different program.
        pN60dBSound->setStartAndEndPosition(firstSoundPosition, lastSoundPosition);
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
