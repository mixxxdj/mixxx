#include "balanceeffect.h"

#include "util/defs.h"

namespace {
    double kMaxCorner = 22000; // Hz
    double kMinCorner = 16; // Hz
    static const unsigned int kStartupSamplerate = 44100;
} // anonymous namespace

// static
QString BalanceEffect::getId() {
    return "org.mixxx.effects.balance";
}

// static
EffectManifest BalanceEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Balance"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr("Adjust the left/right balance and Stereo width"));

    EffectManifestParameter* left = manifest.addParameter();
    left->setId("left");
    left->setName(QObject::tr("Left"));
    left->setDescription(QObject::tr("Level of the left channel"));
    left->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    left->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    left->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    left->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED_RIGHT);
    left->setDefaultLinkInversion(EffectManifestParameter::LinkInversion::INVERTED);
    left->setMinimum(0.0);
    left->setMaximum(1.0);
    left->setDefault(1.0);

    EffectManifestParameter* right = manifest.addParameter();
    right->setId("right");
    right->setName(QObject::tr("Right"));
    right->setDescription(QObject::tr("Level of the right channel"));
    right->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    right->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    right->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    right->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED_LEFT);
    right->setMinimum(0.0);
    right->setMaximum(1.0);
    right->setDefault(1.0);

    EffectManifestParameter* midSide = manifest.addParameter();
    midSide->setId("midSide");
    midSide->setName(QObject::tr("Mid/Side"));
    midSide->setDescription(QObject::tr("Balance of Mid and Side"));
    midSide->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    midSide->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    midSide->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    midSide->setDefaultLinkType(EffectManifestParameter::LinkType::NONE);
    midSide->setMinimum(0);
    midSide->setMaximum(1.0);
    midSide->setDefault(0.5);

    EffectManifestParameter* midLowPass = manifest.addParameter();
    midLowPass->setId("bypassFreq");
    midLowPass->setName(QObject::tr("Bypass Freq."));
    midLowPass->setDescription(QObject::tr("Frequencies below this cutoff are not adjusted in the stereo field"));
    midLowPass->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    midLowPass->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    midLowPass->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    midLowPass->setDefaultLinkType(EffectManifestParameter::LinkType::NONE);
    midLowPass->setNeutralPointOnScale(1);
    midLowPass->setDefault(kMinCorner);
    midLowPass->setMinimum(kMinCorner);
    midLowPass->setMaximum(kMaxCorner);

    return manifest;
}

BalanceGroupState::BalanceGroupState()
        : m_pHighBuf(MAX_BUFFER_LEN),
          m_oldSampleRate(kStartupSamplerate),
          m_freq(kMinCorner),
          m_oldLeft(1),
          m_oldRight(1),
          m_oldMidSide(0) {
    m_low = std::make_unique<EngineFilterLinkwtzRiley4Low>(kStartupSamplerate, kMinCorner);
    m_high = std::make_unique<EngineFilterLinkwtzRiley4High>(kStartupSamplerate, kMinCorner);
    m_high->setStartFromDry(true);
}

BalanceGroupState::~BalanceGroupState() {
}

void BalanceGroupState::setFilters(int sampleRate, int freq) {
    m_low->setFrequencyCorners(sampleRate, freq);
    m_high->setFrequencyCorners(sampleRate, freq);
}

BalanceEffect::BalanceEffect(EngineEffect* pEffect, const EffectManifest& manifest)
          : m_pLeftParameter(pEffect->getParameterById("left")),
            m_pRightParameter(pEffect->getParameterById("right")),
            m_pMidSideParameter(pEffect->getParameterById("midSide")),
            m_pBypassFreqParameter(pEffect->getParameterById("bypassFreq")) {
    Q_UNUSED(manifest);
}

BalanceEffect::~BalanceEffect() {
}

void BalanceEffect::processChannel(const ChannelHandle& handle,
                               BalanceGroupState* pGroupState,
                               const CSAMPLE* pInput,
                               CSAMPLE* pOutput, const unsigned int numSamples,
                               const unsigned int sampleRate,
                               const EffectProcessor::EnableState enableState,
                               const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(groupFeatures);


    CSAMPLE_GAIN left = m_pLeftParameter->value();
    CSAMPLE_GAIN right = m_pRightParameter->value();
    CSAMPLE_GAIN midSide = m_pMidSideParameter->value();

    CSAMPLE_GAIN leftDelta = (left - pGroupState->m_oldLeft)
                    / CSAMPLE_GAIN(numSamples / 2);
    CSAMPLE_GAIN rightDelta = (right - pGroupState->m_oldRight)
                    / CSAMPLE_GAIN(numSamples / 2);
    CSAMPLE_GAIN midSideDelta = (midSide - pGroupState->m_oldMidSide)
                    / CSAMPLE_GAIN(numSamples / 2);

    CSAMPLE_GAIN leftStart = left - leftDelta;
    CSAMPLE_GAIN rightStart = right - rightDelta;
    CSAMPLE_GAIN midSideStart = midSide - midSideDelta;


    if (pGroupState->m_oldSampleRate != sampleRate ||
            (pGroupState->m_freq != static_cast<int>(m_pBypassFreqParameter->value()))) {
        pGroupState->m_freq = static_cast<int>(m_pBypassFreqParameter->value());
        pGroupState->m_oldSampleRate = sampleRate;
        pGroupState->setFilters(sampleRate, pGroupState->m_freq);
    }

    if (pGroupState->m_freq > kMinCorner) {
        pGroupState->m_high->process(pInput, pGroupState->m_pHighBuf.data(), numSamples); // HighPass first run
        pGroupState->m_low->process(pInput, pOutput, numSamples); // LowPass first run for low and bandpass

        for (SINT i = 0; i < numSamples / 2; ++i) {
            CSAMPLE mid = (pGroupState->m_pHighBuf[i * 2]  + pGroupState->m_pHighBuf[i * 2 + 1]) / 2.0f;
            CSAMPLE side = (pGroupState->m_pHighBuf[i * 2 + 1] - pGroupState->m_pHighBuf[i * 2]) / 2.0f;
            if (midSide > 0.5) {
                mid *= 2 * (1 - (midSideStart + midSideDelta * i));
            } else {
                side *= 2 * (midSideStart + midSideDelta * i);
            }
            pOutput[i * 2] += (mid - side) * (leftStart + leftDelta * i);
            pOutput[i * 2 + 1] += (mid + side) * (rightStart + rightDelta * i);
        }

    } else {
        pGroupState->m_high->pauseFilter();
        pGroupState->m_low->pauseFilter();

        for (SINT i = 0; i < numSamples / 2; ++i) {
            CSAMPLE mid = (pInput[i * 2]  + pInput[i * 2 + 1]) / 2.0f;
            CSAMPLE side = (pInput[i * 2 + 1] - pInput[i * 2]) / 2.0f;
            if (midSide > 0.5) {
                mid *= 2 * (1 - (midSideStart + midSideDelta * i));
            } else {
                side *= 2 * (midSideStart + midSideDelta * i);
            }
            pOutput[i * 2] = (mid - side) * (leftStart + leftDelta * i);
            pOutput[i * 2 + 1] = (mid + side) * (rightStart + rightDelta * i);
        }
    }


    if (enableState == EffectProcessor::DISABLING) {
        // we rely on the ramping to dry in EngineEffect
        // since this EQ is not fully dry at unity
        pGroupState->m_low->pauseFilter();
        pGroupState->m_high->pauseFilter();

        pGroupState->m_oldLeft = left;
        pGroupState->m_oldRight = right;
        pGroupState->m_oldMidSide = midSide;
    } else {
        pGroupState->m_oldLeft = left;
        pGroupState->m_oldRight = right;
        pGroupState->m_oldMidSide = midSide;
    }

}
