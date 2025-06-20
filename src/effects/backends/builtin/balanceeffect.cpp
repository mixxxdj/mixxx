#include "balanceeffect.h"

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/defs.h"

namespace {
constexpr double kMaxCornerHz = 500;
constexpr double kMinCornerHz = 16;
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
    pManifest->setMetaknobDefault(0.5);

    EffectManifestParameterPointer balance = pManifest->addParameter();
    balance->setId("balance");
    balance->setName(QObject::tr("Balance"));
    balance->setShortName(QObject::tr("Balance"));
    balance->setDescription(QObject::tr(
            "Adjust balance between left and right channels"));
    balance->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    balance->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    balance->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    balance->setRange(-1.0, 0.0, +1.0);

    EffectManifestParameterPointer midSide = pManifest->addParameter();
    midSide->setId("midSide");
    midSide->setName(QObject::tr("Mid/Side"));
    midSide->setShortName(QObject::tr("Mid/Side"));
    midSide->setDescription(QObject::tr(
            "Adjust stereo width by changing balance between middle and side of the signal.\n"
            "Fully left: mono\n"
            "Fully right: only side ambiance\n"
            "Center: does not change the original signal."));
    midSide->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    midSide->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    midSide->setDefaultLinkType(EffectManifestParameter::LinkType::None);
    midSide->setRange(-1.0, 0.0, +1.0);

    EffectManifestParameterPointer midLowPass = pManifest->addParameter();
    midLowPass->setId("bypassFreq");
    midLowPass->setName(QObject::tr("Bypass Frequency"));
    midLowPass->setShortName(QObject::tr("Bypass Fr."));
    midLowPass->setDescription(QObject::tr(
            "Frequencies below this cutoff are not adjusted in the stereo field"));
    midLowPass->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    midLowPass->setUnitsHint(EffectManifestParameter::UnitsHint::Hertz);
    midLowPass->setDefaultLinkType(EffectManifestParameter::LinkType::None);
    midLowPass->setNeutralPointOnScale(1);
    midLowPass->setRange(kMinCornerHz, kMinCornerHz, kMaxCornerHz);

    return pManifest;
}

BalanceGroupState::BalanceGroupState(const mixxx::EngineParameters& engineParameters)
        : EffectState(engineParameters),
          m_pHighBuf(engineParameters.samplesPerBuffer()),
          m_oldSampleRate(engineParameters.sampleRate()),
          m_freq(kMinCornerHz),
          m_oldBalance(0),
          m_oldMidSide(0) {
    m_low = std::make_unique<EngineFilterLinkwitzRiley4Low>(engineParameters.sampleRate(),
            kMinCornerHz);
    m_high = std::make_unique<EngineFilterLinkwitzRiley4High>(engineParameters.sampleRate(),
            kMinCornerHz);
    m_high->setStartFromDry(true);
}

void BalanceGroupState::setFilters(mixxx::audio::SampleRate sampleRate, double freq) {
    m_low->setFrequencyCorners(sampleRate, freq);
    m_high->setFrequencyCorners(sampleRate, freq);
}

void BalanceEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pBalanceParameter = parameters.value("balance");
    m_pMidSideParameter = parameters.value("midSide");
    m_pBypassFreqParameter = parameters.value("bypassFreq");
}

void BalanceEffect::processChannel(
        BalanceGroupState* pGroupState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(groupFeatures);

    CSAMPLE_GAIN balance = 0;
    CSAMPLE_GAIN midSide = 0;
    if (enableState != EffectEnableState::Disabling) {
        balance = static_cast<CSAMPLE_GAIN>(m_pBalanceParameter->value());
        midSide = static_cast<CSAMPLE_GAIN>(m_pMidSideParameter->value());
    }

    CSAMPLE_GAIN balanceDelta = (balance - pGroupState->m_oldBalance) /
            CSAMPLE_GAIN(engineParameters.framesPerBuffer());
    CSAMPLE_GAIN midSideDelta = (midSide - pGroupState->m_oldMidSide) /
            CSAMPLE_GAIN(engineParameters.framesPerBuffer());

    CSAMPLE_GAIN balanceStart = pGroupState->m_oldBalance + balanceDelta;
    CSAMPLE_GAIN midSideStart = pGroupState->m_oldMidSide + midSideDelta;

    double freq = pGroupState->m_freq;
    if (pGroupState->m_oldSampleRate != engineParameters.sampleRate() ||
            (freq != m_pBypassFreqParameter->value())) {
        freq = m_pBypassFreqParameter->value();
        pGroupState->m_oldSampleRate = engineParameters.sampleRate();
        pGroupState->setFilters(engineParameters.sampleRate(), pGroupState->m_freq);
    }

    if (pGroupState->m_freq > kMinCornerHz) {
        if (freq > kMinCornerHz && enableState != EffectEnableState::Disabling) {
            pGroupState->m_high->process(pInput,
                    pGroupState->m_pHighBuf.data(),
                    engineParameters.samplesPerBuffer());
            pGroupState->m_low->process(pInput, pOutput, engineParameters.samplesPerBuffer());
        } else {
            pGroupState->m_high->processAndPauseFilter(pInput,
                    pGroupState->m_pHighBuf.data(),
                    engineParameters.samplesPerBuffer());
            pGroupState->m_low->processAndPauseFilter(
                    pInput, pOutput, engineParameters.samplesPerBuffer());
        }

        for (SINT i = 0; i < engineParameters.samplesPerBuffer() / 2; ++i) {
            CSAMPLE mid = (pGroupState->m_pHighBuf[i * 2] +
                                  pGroupState->m_pHighBuf[i * 2 + 1]) /
                    2.0f;
            CSAMPLE side = (pGroupState->m_pHighBuf[i * 2 + 1] -
                                   pGroupState->m_pHighBuf[i * 2]) /
                    2.0f;
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

        for (SINT i = 0; i < engineParameters.samplesPerBuffer() / 2; ++i) {
            CSAMPLE mid = (pInput[i * 2] + pInput[i * 2 + 1]) / 2.0f;
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
