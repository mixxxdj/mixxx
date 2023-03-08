#include "effects/backends/builtin/glitcheffect.h"

#include <QtDebug>

#include "util/math.h"
#include "util/sample.h"

QString GlitchEffect::getId() {
    return "com.kopanko.mixxxeffects.glitch";
}

EffectManifestPointer GlitchEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());

    pManifest->setAddDryToWet(false);
    pManifest->setEffectRampsFromDry(false);

    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Glitch"));
    pManifest->setShortName(QObject::tr("Glitch"));
    pManifest->setAuthor("pcktm");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
            "Stores the input signal in a temporary buffer and repeats it"));

    EffectManifestParameterPointer delay = pManifest->addParameter();
    delay->setId("delay_time");
    delay->setName(QObject::tr("Time"));
    delay->setShortName(QObject::tr("Time"));
    delay->setDescription(QObject::tr(
            "Delay time\n"
            "1/8 - 2 beats if tempo is detected\n"
            "1/8 - 2 seconds if no tempo is detected"));
    delay->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    delay->setUnitsHint(EffectManifestParameter::UnitsHint::Beats);
    delay->setRange(0.0, 0.5, 2.0);

    EffectManifestParameterPointer quantize = pManifest->addParameter();
    quantize->setId("quantize");
    quantize->setName(QObject::tr("Quantize"));
    quantize->setShortName(QObject::tr("Quantize"));
    quantize->setDescription(QObject::tr(
            "Round the Time parameter to the nearest 1/4 beat."));
    quantize->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    quantize->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    quantize->setRange(0, 1, 1);

    EffectManifestParameterPointer triplet = pManifest->addParameter();
    triplet->setId("triplet");
    triplet->setName(QObject::tr("Triplets"));
    triplet->setShortName(QObject::tr("Triplets"));
    triplet->setDescription(
            QObject::tr("When the Quantize parameter is enabled, divide "
                        "rounded 1/4 beats of Time parameter by 3."));
    triplet->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    triplet->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    triplet->setRange(0, 0, 1);

    return pManifest;
}

void GlitchEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pDelayParameter = parameters.value("delay_time");
    m_pQuantizeParameter = parameters.value("quantize");
    m_pTripletParameter = parameters.value("triplet");
}

void GlitchEffect::processChannel(
        GlitchGroupState* pGroupState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {

    // The minimum of the parameter is zero so the exact center of the knob is 1 beat.
    double period = m_pDelayParameter->value();

    int delay_frames;
    double max_delay;
    double min_delay;
    if (groupFeatures.has_beat_length_sec) {
        // period is a number of beats
        if (m_pQuantizeParameter->toBool()) {
            period = std::max(roundToFraction(period, 4), 1 / 8.0);
            if (m_pTripletParameter->toBool()) {
                period /= 3.0;
            }
        } else if (period < 1 / 8.0) {
            period = 1 / 8.0;
        }
        delay_frames = static_cast<int>(period * groupFeatures.beat_length_sec *
                engineParameters.sampleRate());
        max_delay = 2 * groupFeatures.beat_length_sec * engineParameters.sampleRate();
        min_delay = 1 / 8.0 * groupFeatures.beat_length_sec * engineParameters.sampleRate();
    } else {
        // period is a number of seconds
        period = std::max(period, 1 / 8.0);
        delay_frames = static_cast<int>(period * engineParameters.sampleRate());
        max_delay = 2 * engineParameters.sampleRate();
        min_delay = 1 / 8.0 * engineParameters.sampleRate();
    }

    // Scale the delay so that the knob starts at no glitch and ends at the maximum delay.
    delay_frames = static_cast<int>((delay_frames - min_delay) *
            (max_delay - min_delay) / (max_delay - min_delay));
    int delay_samples = delay_frames * engineParameters.channelCount();

    pGroupState->sample_count += engineParameters.samplesPerBuffer();
    if (pGroupState->sample_count >= delay_samples) {
        if (m_pDelayParameter->value() < 2.0) {
            SampleUtil::copy(pGroupState->repeat_buf.data(),
                    pInput,
                    engineParameters.samplesPerBuffer());
        }
        pGroupState->sample_count = 0;
    }

    SampleUtil::copy(
            pOutput,
            pGroupState->repeat_buf.data(),
            engineParameters.samplesPerBuffer());
}
