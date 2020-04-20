#include "analyzer/plugins/analyzerkeyfinder.h"

#include "analyzer/constants.h"
#include "util/assert.h"
#include "util/math.h"

using mixxx::track::io::key::ChromaticKey;
using mixxx::track::io::key::ChromaticKey_IsValid;

namespace mixxx {

AnalyzerKeyFinder::AnalyzerKeyFinder()
        : m_pKeyFinder(nullptr),
          m_pWorkspace(nullptr),
          m_pAudioData(nullptr),
          m_currentFrame(0) {
}

AnalyzerKeyFinder::~AnalyzerKeyFinder() {
}

bool AnalyzerKeyFinder::initialize(int samplerate) {
    m_resultKeys.clear();
    m_currentFrame = 0;

    m_pKeyFinder = std::make_unique<KeyFinder::KeyFinder>();
    m_pWorkspace = std::make_unique<KeyFinder::Workspace>();
    m_pAudioData = std::make_unique<KeyFinder::AudioData>();
    m_pAudioData->setFrameRate(samplerate);
    m_pAudioData->setChannels(kAnalysisChannels);

    return true;
}

bool AnalyzerKeyFinder::processSamples(const CSAMPLE* pIn, const int iLen) {
    DEBUG_ASSERT(iLen % kAnalysisChannels == 0);

    const size_t numInputFrames = iLen / kAnalysisChannels;
    m_currentFrame += numInputFrames;

    m_pAudioData->addToSampleCount(iLen);
    for (unsigned int frame = 0; frame < numInputFrames; frame++) {
        for (unsigned int channel = 0; channel < kAnalysisChannels; channel++) {
            m_pAudioData->setSampleByFrame(
                    frame, channel, pIn[frame * kAnalysisChannels + channel]);
        }
    }
    // Do not build a progressive list of key changes here because it is very slow.
    // Instead, just calculate the global key in finalize.
    return true;
}

bool AnalyzerKeyFinder::finalize() {
    m_pKeyFinder->progressiveChromagram(*m_pAudioData, *m_pWorkspace);
    ChromaticKey globalKey = chromaticKeyFromKeyFinderKeyT(
            m_pKeyFinder->keyOfChromagram(*m_pWorkspace));
    m_resultKeys.push_back(qMakePair(globalKey, 0));
    return true;
}

ChromaticKey AnalyzerKeyFinder::chromaticKeyFromKeyFinderKeyT(KeyFinder::key_t key) {
    switch (key) {
    case (KeyFinder::A_MAJOR):
        return ChromaticKey::A_MAJOR;
    case (KeyFinder::A_MINOR):
        return ChromaticKey::A_MINOR;
    case (KeyFinder::B_FLAT_MAJOR):
        return ChromaticKey::B_FLAT_MAJOR;
    case (KeyFinder::B_FLAT_MINOR):
        return ChromaticKey::B_FLAT_MINOR;
    case (KeyFinder::B_MAJOR):
        return ChromaticKey::B_MAJOR;
    case (KeyFinder::B_MINOR):
        return ChromaticKey::B_MINOR;
    case (KeyFinder::C_MAJOR):
        return ChromaticKey::C_MAJOR;
    case (KeyFinder::C_MINOR):
        return ChromaticKey::C_MINOR;
    case (KeyFinder::D_FLAT_MAJOR):
        return ChromaticKey::D_FLAT_MAJOR;
    case (KeyFinder::D_FLAT_MINOR):
        return ChromaticKey::C_SHARP_MINOR;
    case (KeyFinder::D_MAJOR):
        return ChromaticKey::D_MAJOR;
    case (KeyFinder::D_MINOR):
        return ChromaticKey::D_MINOR;
    case (KeyFinder::E_FLAT_MAJOR):
        return ChromaticKey::E_FLAT_MAJOR;
    case (KeyFinder::E_FLAT_MINOR):
        return ChromaticKey::E_FLAT_MINOR;
    case (KeyFinder::E_MINOR):
        return ChromaticKey::E_MINOR;
    case (KeyFinder::F_MAJOR):
        return ChromaticKey::F_MAJOR;
    case (KeyFinder::F_MINOR):
        return ChromaticKey::F_MINOR;
    case (KeyFinder::G_FLAT_MAJOR):
        return ChromaticKey::F_SHARP_MAJOR;
    case (KeyFinder::G_FLAT_MINOR):
        return ChromaticKey::F_SHARP_MINOR;
    case (KeyFinder::G_MAJOR):
        return ChromaticKey::G_MAJOR;
    case (KeyFinder::G_MINOR):
        return ChromaticKey::G_MINOR;
    case (KeyFinder::A_FLAT_MAJOR):
        return ChromaticKey::A_FLAT_MAJOR;
    case (KeyFinder::A_FLAT_MINOR):
        return ChromaticKey::G_SHARP_MINOR;
    case (KeyFinder::SILENCE):
    default:
        return ChromaticKey::INVALID;
    }
}

} // namespace mixxx
