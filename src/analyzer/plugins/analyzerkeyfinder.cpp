#include "analyzer/plugins/analyzerkeyfinder.h"

#include "analyzer/constants.h"
#include "util/assert.h"
#include "util/math.h"

using mixxx::track::io::key::ChromaticKey;
using mixxx::track::io::key::ChromaticKey_IsValid;

namespace {
// 2 is for the library version
const QString pluginId = QStringLiteral("keyfinder:2");
const QString pluginAuthor = QStringLiteral("Ibrahim Sha'ath");
const QString pluginName = QStringLiteral("KeyFinder");

ChromaticKey chromaticKeyFromKeyFinderKeyT(KeyFinder::key_t key) {
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
} // anonymous namespace

namespace mixxx {

AnalyzerKeyFinder::AnalyzerKeyFinder()
        : m_currentFrame(0) {
}

AnalyzerPluginInfo AnalyzerKeyFinder::pluginInfo() {
    return AnalyzerPluginInfo(pluginId, pluginAuthor, pluginName, false);
}

bool AnalyzerKeyFinder::initialize(int samplerate) {
    m_audioData.setFrameRate(samplerate);
    m_audioData.setChannels(kAnalysisChannels);
    return true;
}

bool AnalyzerKeyFinder::processSamples(const CSAMPLE* pIn, const int iLen) {
    DEBUG_ASSERT(iLen % kAnalysisChannels == 0);
    if (m_audioData.getSampleCount() == 0) {
        m_audioData.addToSampleCount(iLen);
    }

    const SINT numInputFrames = iLen / kAnalysisChannels;
    m_currentFrame += numInputFrames;

    for (SINT frame = 0; frame < numInputFrames; frame++) {
        for (SINT channel = 0; channel < kAnalysisChannels; channel++) {
            m_audioData.setSampleByFrame(
                    frame, channel, pIn[frame * kAnalysisChannels + channel]);
        }
    }
    m_keyFinder.progressiveChromagram(m_audioData, m_workspace);
    return true;
}

bool AnalyzerKeyFinder::finalize() {
    m_keyFinder.finalChromagram(m_workspace);
    ChromaticKey finalKey = chromaticKeyFromKeyFinderKeyT(
            m_keyFinder.keyOfChromagram(m_workspace));
    m_resultKeys.push_back(qMakePair(finalKey, m_currentFrame));
    return true;
}

} // namespace mixxx
