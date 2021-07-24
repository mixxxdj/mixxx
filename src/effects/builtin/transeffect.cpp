#include "effects/builtin/transeffect.h"

#include "util/rampingvalue.h"
#include "util/sample.h"

// static
QString TransEffect::getId() {
    return "org.mixxx.effects.trans";
}

// static
EffectManifestPointer TransEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());

    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Trans"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr("Trans effect"));

    EffectManifestParameterPointer period = pManifest->addParameter();
    period->setId("period");
    period->setName(QObject::tr("Period"));
    period->setDescription(QObject::tr("Trans period (1/period = repetitions per beat when quantized / otherwise period time in seconds)."));
    period->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    period->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    period->setUnitsHint(EffectManifestParameter::UnitsHint::TIME);
    period->setMinimum(0.05);
    period->setDefault(0.1);
    period->setMaximum(2.0);

    EffectManifestParameterPointer fade_time_frac = pManifest->addParameter();
    fade_time_frac->setId("fadetime");
    fade_time_frac->setName(QObject::tr("Fade Time Fraction"));
    fade_time_frac->setDescription(QObject::tr("Fraction of time for fade in/out: 100% = 50%% fade-in + 50%% fade-out."));
    fade_time_frac->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    fade_time_frac->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    fade_time_frac->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    fade_time_frac->setMinimum(0);
    fade_time_frac->setDefault(0.75);
    fade_time_frac->setMaximum(1.0);

    EffectManifestParameterPointer cut_off_frac = pManifest->addParameter();
    cut_off_frac->setId("cutoff");
    cut_off_frac->setName(QObject::tr("Cutoff Time Fraction"));
    cut_off_frac->setDescription(QObject::tr("Fraction of time of cutoff."));
    cut_off_frac->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    cut_off_frac->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    cut_off_frac->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    cut_off_frac->setMinimum(0);
    cut_off_frac->setDefault(0.4);
    cut_off_frac->setMaximum(0.9);

    // This effect can take away a lot of power and we cannot compensate for that (1.0 is max), so send everything by default.
    EffectManifestParameterPointer send = pManifest->addParameter();
    send->setId("send_amount");
    send->setName(QObject::tr("Send"));
    send->setShortName(QObject::tr("Send"));
    send->setDescription(QObject::tr(
            "How much of the signal to send to the output."));
    send->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    send->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    send->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    send->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED);
    send->setMinimum(0.0);
    send->setDefault(db2ratio(0.0));
    send->setMaximum(1.0);

    EffectManifestParameterPointer quantize = pManifest->addParameter();
    quantize->setId("quantize");
    quantize->setName(QObject::tr("Quantize"));
    quantize->setShortName(QObject::tr("Quantize"));
    quantize->setDescription(QObject::tr(
            "Round inverse period to nearest integer per second."));
    quantize->setControlHint(EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
    quantize->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    quantize->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    quantize->setDefault(1);
    quantize->setMinimum(0);
    quantize->setMaximum(1);

    return pManifest;
}

TransEffect::TransEffect(EngineEffect* pEffect)
        : m_pPeriodParameter(pEffect->getParameterById("period")),
          m_pFadeTimeParameter(pEffect->getParameterById("fadetime")),
          m_pCutoffTimeParameter(pEffect->getParameterById("cutoff")),
          m_pSendParameter(pEffect->getParameterById("send_amount")),
          m_pQuantizeParameter(pEffect->getParameterById("quantize")) {
}

void TransEffect::processChannel(const ChannelHandle& handle,
        TransGroupState* pGroupState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& bufferParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);

    using Sample_rate = ::mixxx::audio::SampleRate::value_t;

    TransGroupState& gs = *pGroupState;
    auto period = m_pPeriodParameter->value();
    auto fade_frac = m_pFadeTimeParameter->value();
    auto cutoff_frac = m_pCutoffTimeParameter->value();
    auto original_send_target = static_cast<CSAMPLE_GAIN>(m_pSendParameter->value());
    Sample_rate sampleRate = bufferParameters.sampleRate();

    if (m_pQuantizeParameter->toBool() && groupFeatures.has_beat_length_sec) {
        auto four_beat_length_sec = groupFeatures.beat_length_sec * 4.0;
        auto next_multiple_in_four_beats = std::round(four_beat_length_sec / period);
        if (next_multiple_in_four_beats < 1.0) {
            next_multiple_in_four_beats = 1.0;
        }
        period = four_beat_length_sec / next_multiple_in_four_beats;
    }

    std::uint32_t period_samples = static_cast<std::uint32_t>(double(sampleRate) * period);

    auto on_frac = 1.0 - cutoff_frac;
    auto on_samples = double(period_samples) * on_frac;
    std::uint32_t ramp_samples = static_cast<std::uint32_t>(on_samples * fade_frac * 0.5);
    std::uint32_t kill_point = static_cast<std::uint32_t>(on_samples) - ramp_samples;

    auto send_target = original_send_target;
    if (enableState == EffectEnableState::Disabling) {
        send_target = 0.0;
    }

    auto factor_delta_per_sample = CSAMPLE_GAIN(1.0) / CSAMPLE_GAIN(ramp_samples);

    RampingValue<CSAMPLE_GAIN> send(send_target,
            pGroupState->lastSend,
            bufferParameters.framesPerBuffer());

    /*
     *   ramp-up         ramp-down
     *   |<---->|        |<---->|
     *
     *  |x|              +- killpoint
     *   ^               v
     *   |      /--------\        <-send->                        /--------\
     *   |     /          \                                      /          \
     *   |    /            \                                    /            \
     *   |   /              \                                  /              \
     *   |  /                \                                /                \
     *   | /                  \                              /                  \
     *   |/                    \                            /                    \
     * 0 +---------------------------------------------------------------------------> N
     *   |<--------------------- period ------------------>|
     *                          |<---- cutoff fraction --->|
     */

    for (unsigned int i = 0; i < bufferParameters.samplesPerBuffer(); i += bufferParameters.channelCount()) {
        if (gs.playedFrames < kill_point) {
            if (gs.transFactor < 1.0) {
                gs.transFactor = SampleUtil::clampGain(gs.transFactor + factor_delta_per_sample);
            }
        } else {
            if (gs.transFactor > 0.0) {
                gs.transFactor = SampleUtil::clampGain(gs.transFactor - factor_delta_per_sample);
            } else if (gs.playedFrames >= period_samples) {
                gs.playedFrames = 0;
            }
        }

        auto send_factor = send.getNext();
        auto full_factor = SampleUtil::clampGain(gs.transFactor * send_factor);
        auto j = i;
        for (auto channel = 0U; channel < bufferParameters.channelCount(); channel++) {
            pOutput[j] = pInput[j] * full_factor;
            ++j;
        }

        gs.playedFrames++;
    }

    gs.lastSend = send_target;
}
