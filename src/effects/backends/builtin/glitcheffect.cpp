#include "effects/backends/builtin/glitcheffect.h"

#include <QtDebug>

#include "util/math.h"
#include "util/sample.h"

QString GlitchEffect::getId() {
    return QStringLiteral("org.mixxx.effects.glitch");
}

EffectManifestPointer GlitchEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());

    pManifest->setAddDryToWet(false);
    pManifest->setEffectRampsFromDry(false);

    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Glitch"));
    pManifest->setShortName(QObject::tr("Glitch"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(
            QObject::tr("Periodically samples and repeats a small portion of "
                        "audio to create a glitchy metallic sound."));

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
    delay->setRange(0.0, 0.5, kMaxDelay);

    EffectManifestParameterPointer quantize = pManifest->addParameter();
    quantize->setId("quantize");
    quantize->setName(QObject::tr("Quantize"));
    quantize->setShortName(QObject::tr("Quantize"));
    quantize->setDescription(QObject::tr(
            "Round the Time parameter to the nearest 1/8 beat."));
    quantize->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    quantize->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    quantize->setRange(0, 1, 1);

    EffectManifestParameterPointer triplet = pManifest->addParameter();
    triplet->setId("triplet");
    triplet->setName(QObject::tr("Triplets"));
    triplet->setShortName(QObject::tr("Triplets"));
    triplet->setDescription(
            QObject::tr("When the Quantize parameter is enabled, divide "
                        "rounded 1/8 beats of Time parameter by 3."));
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
    Q_UNUSED(enableState);

    // The minimum of the parameter is zero so the exact center of the knob is 1 beat.
    double period = m_pDelayParameter->value();

    double delay_seconds;
    double min_delay;
    if (groupFeatures.beat_length.has_value()) {
        if (m_pQuantizeParameter->toBool()) {
            period = roundToFraction(period, 8);
            if (m_pTripletParameter->toBool()) {
                period /= 3.0;
            }
        }
        period = std::max(period, 1 / 8.0);
        delay_seconds = static_cast<int>(period * groupFeatures.beat_length->seconds);
        min_delay = 1 / 8.0 * groupFeatures.beat_length->seconds;
    } else {
        delay_seconds = period;
        min_delay = 1 / 8.0;
    }

    if (delay_seconds < min_delay) {
        SampleUtil::copy(
                pOutput,
                pInput,
                engineParameters.samplesPerBuffer());
        return;
    }

    int delay_samples = static_cast<int>(delay_seconds *
            engineParameters.channelCount() * engineParameters.sampleRate());
    pGroupState->sample_count += engineParameters.samplesPerBuffer();
    if (pGroupState->sample_count >= delay_samples) {
        // If the delay parameter is at its maximum value, we don't update the `repeat_buf`
        // in order to achieve an "audio freeze" effect.
        if (m_pDelayParameter->value() < kMaxDelay) {
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
