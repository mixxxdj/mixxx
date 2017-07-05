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

    EffectManifestParameter* balance = manifest.addParameter();
    balance->setId("balance");
    balance->setName(QObject::tr("Balance"));
    balance->setDescription(QObject::tr("Alternate gain for left and right channel"));
    balance->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    balance->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    balance->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    balance->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED);
    balance->setMinimum(-1.0);
    balance->setMaximum(+1.0);
    balance->setDefault(0.0);

    EffectManifestParameter* midSide = manifest.addParameter();
    midSide->setId("midSide");
    midSide->setName(QObject::tr("Mid/Side"));
    midSide->setDescription(QObject::tr("Balance of Mid and Side"));
    midSide->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    midSide->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    midSide->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    midSide->setDefaultLinkType(EffectManifestParameter::LinkType::NONE);
    midSide->setMinimum(-1.0);
    midSide->setMaximum(+1.0);
    midSide->setDefault(0.0);

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
          m_oldBalance(0),
          m_oldMidSide(0) {
    m_low = std::make_unique<EngineFilterLinkwitzRiley4Low>(kStartupSamplerate, kMinCorner);
    m_high = std::make_unique<EngineFilterLinkwitzRiley4High>(kStartupSamplerate, kMinCorner);
    m_high->setStartFromDry(true);
}

BalanceGroupState::~BalanceGroupState() {
}

void BalanceGroupState::setFilters(int sampleRate, int freq) {
    m_low->setFrequencyCorners(sampleRate, freq);
    m_high->setFrequencyCorners(sampleRate, freq);
}

BalanceEffect::BalanceEffect(EngineEffect* pEffect, const EffectManifest& manifest)
          : m_pBalanceParameter(pEffect->getParameterById("balance")),
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

    CSAMPLE_GAIN balance = m_pBalanceParameter->value();
    CSAMPLE_GAIN midSide = m_pMidSideParameter->value();

    CSAMPLE_GAIN balanceDelta = (balance - pGroupState->m_oldBalance)
                    / CSAMPLE_GAIN(numSamples / 2);
    CSAMPLE_GAIN midSideDelta = (midSide - pGroupState->m_oldMidSide)
                    / CSAMPLE_GAIN(numSamples / 2);

    CSAMPLE_GAIN balanceStart = balance - balanceDelta;
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
            CSAMPLE_GAIN currentMidSide = midSideStart + midSideDelta * i;
            if (currentMidSide > 0) {
                mid *= (1 - currentMidSide);
            } else {
                side *= (1 + currentMidSide);
            }
            CSAMPLE_GAIN currentBalance = (balanceStart + balanceDelta * i);
            if (currentBalance > 0) {
                pOutput[i * 2] += (mid - side) * (1 - currentBalance);
                pOutput[i * 2 + 1] += (mid + side);
            } else {
                pOutput[i * 2] += (mid - side);
                pOutput[i * 2 + 1] += (mid + side) * (1 + currentBalance);
            }
        }
    } else {
        pGroupState->m_high->pauseFilter();
        pGroupState->m_low->pauseFilter();

        for (SINT i = 0; i < numSamples / 2; ++i) {
            CSAMPLE mid = (pInput[i * 2]  + pInput[i * 2 + 1]) / 2.0f;
            CSAMPLE side = (pInput[i * 2 + 1] - pInput[i * 2]) / 2.0f;
            CSAMPLE_GAIN currentMidSide = midSideStart + midSideDelta * i;
            if (currentMidSide > 0) {
               mid *= (1 - currentMidSide);
            } else {
               side *= (1 + currentMidSide);
            }
            CSAMPLE_GAIN currentBalance = (balanceStart + balanceDelta * i);
            if (currentBalance > 0) {
               pOutput[i * 2] = (mid - side) * (1 - currentBalance);
               pOutput[i * 2 + 1] = (mid + side);
            } else {
               pOutput[i * 2] = (mid - side);
               pOutput[i * 2 + 1] = (mid + side) * (1 + currentBalance);
            }
        }
    }

    if (enableState == EffectProcessor::DISABLING) {
        // we rely on the ramping to dry in EngineEffect
        // since this EQ is not fully dry at unity
        pGroupState->m_low->pauseFilter();
        pGroupState->m_high->pauseFilter();
    }
    pGroupState->m_oldBalance = balance;
    pGroupState->m_oldMidSide = midSide;
}
