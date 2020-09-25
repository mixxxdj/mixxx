#include "balanceeffect.h"

#include "util/defs.h"

namespace {
    const double kMaxCornerHz = 500;
    const double kMinCornerHz = 16;
} // anonymous namespace

// static
QString BalanceEffect::getId() {
    return "org.mixxx.effects.balance";
}

// static
EffectManifestPointer BalanceEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Stereo Balance"));
    pManifest->setShortName(QObject::tr("Balance"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
        "Adjust the left/right balance and stereo width"));
    pManifest->setEffectRampsFromDry(true);

    EffectManifestParameterPointer balance = pManifest->addParameter();
    balance->setId("balance");
    balance->setName(QObject::tr("Balance"));
    balance->setShortName(QObject::tr("Balance"));
    balance->setDescription(QObject::tr(
        "Adjust balance between left and right channels"));
    balance->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    balance->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    balance->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    balance->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED);
    balance->setMinimum(-1.0);
    balance->setMaximum(+1.0);
    balance->setDefault(0.0);

    EffectManifestParameterPointer midSide = pManifest->addParameter();
    midSide->setId("midSide");
    midSide->setName(QObject::tr("Mid/Side"));
    midSide->setShortName(QObject::tr("Mid/Side"));
    midSide->setDescription(QObject::tr(
        "Adjust stereo width by changing balance between middle and side of the signal.\n"
        "Fully left: mono\n"
        "Fully right: only side ambiance\n"
        "Center: does not change the original signal."));
    midSide->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    midSide->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    midSide->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    midSide->setDefaultLinkType(EffectManifestParameter::LinkType::NONE);
    midSide->setMinimum(-1.0);
    midSide->setMaximum(+1.0);
    midSide->setDefault(0.0);

    EffectManifestParameterPointer midLowPass = pManifest->addParameter();
    midLowPass->setId("bypassFreq");
    midLowPass->setName(QObject::tr("Bypass Frequency"));
    midLowPass->setShortName(QObject::tr("Bypass Fr."));
    midLowPass->setDescription(QObject::tr(
        "Frequencies below this cutoff are not adjusted in the stereo field"));
    midLowPass->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    midLowPass->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    midLowPass->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    midLowPass->setDefaultLinkType(EffectManifestParameter::LinkType::NONE);
    midLowPass->setNeutralPointOnScale(1);
    midLowPass->setDefault(kMinCornerHz);
    midLowPass->setMinimum(kMinCornerHz);
    midLowPass->setMaximum(kMaxCornerHz);

    return pManifest;
}

BalanceGroupState::BalanceGroupState(const mixxx::EngineParameters& bufferParameters)
        : EffectState(bufferParameters),
          m_pHighBuf(MAX_BUFFER_LEN),
          m_oldSampleRate(bufferParameters.sampleRate()),
          m_freq(kMinCornerHz),
          m_oldBalance(0),
          m_oldMidSide(0) {
    m_low = std::make_unique<EngineFilterLinkwitzRiley4Low>(bufferParameters.sampleRate(),
                                                            kMinCornerHz);
    m_high = std::make_unique<EngineFilterLinkwitzRiley4High>(bufferParameters.sampleRate(),
                                                              kMinCornerHz);
    m_high->setStartFromDry(true);
}

BalanceGroupState::~BalanceGroupState() {
}

void BalanceGroupState::setFilters(int sampleRate, int freq) {
    m_low->setFrequencyCorners(sampleRate, freq);
    m_high->setFrequencyCorners(sampleRate, freq);
}

BalanceEffect::BalanceEffect(EngineEffect* pEffect)
          : m_pBalanceParameter(pEffect->getParameterById("balance")),
            m_pMidSideParameter(pEffect->getParameterById("midSide")),
            m_pBypassFreqParameter(pEffect->getParameterById("bypassFreq")) {
}

BalanceEffect::~BalanceEffect() {
}

void BalanceEffect::processChannel(const ChannelHandle& handle,
                               BalanceGroupState* pGroupState,
                               const CSAMPLE* pInput, CSAMPLE* pOutput,
                               const mixxx::EngineParameters& bufferParameters,
                               const EffectEnableState enableState,
                               const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(groupFeatures);

    CSAMPLE_GAIN balance = 0;
    CSAMPLE_GAIN midSide = 0;
    if (enableState != EffectEnableState::Disabling) {
        balance = static_cast<CSAMPLE_GAIN>(m_pBalanceParameter->value());
        midSide = static_cast<CSAMPLE_GAIN>(m_pMidSideParameter->value());
    }

    CSAMPLE_GAIN balanceDelta = (balance - pGroupState->m_oldBalance)
                    / CSAMPLE_GAIN(bufferParameters.framesPerBuffer());
    CSAMPLE_GAIN midSideDelta = (midSide - pGroupState->m_oldMidSide)
                    / CSAMPLE_GAIN(bufferParameters.framesPerBuffer());

    CSAMPLE_GAIN balanceStart = pGroupState->m_oldBalance + balanceDelta;
    CSAMPLE_GAIN midSideStart = pGroupState->m_oldMidSide + midSideDelta;

    int freq = pGroupState->m_freq;
    if (pGroupState->m_oldSampleRate != bufferParameters.sampleRate() ||
            (freq != static_cast<int>(m_pBypassFreqParameter->value()))) {
        freq = static_cast<int>(m_pBypassFreqParameter->value());
        pGroupState->m_oldSampleRate = bufferParameters.sampleRate();
        pGroupState->setFilters(bufferParameters.sampleRate(), pGroupState->m_freq);
    }

    if (pGroupState->m_freq > kMinCornerHz) {
        if (freq > kMinCornerHz && enableState != EffectEnableState::Disabling) {
            pGroupState->m_high->process(pInput, pGroupState->m_pHighBuf.data(), bufferParameters.samplesPerBuffer());
            pGroupState->m_low->process(pInput, pOutput, bufferParameters.samplesPerBuffer());
        } else {
            pGroupState->m_high->processAndPauseFilter(pInput, pGroupState->m_pHighBuf.data(), bufferParameters.samplesPerBuffer());
            pGroupState->m_low->processAndPauseFilter(pInput, pOutput, bufferParameters.samplesPerBuffer());
        }

        for (SINT i = 0; i < bufferParameters.samplesPerBuffer() / 2; ++i) {
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

        for (SINT i = 0; i < bufferParameters.samplesPerBuffer() / 2; ++i) {
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

    pGroupState->m_oldBalance = balance;
    pGroupState->m_oldMidSide = midSide;
    pGroupState->m_freq = freq;
}
