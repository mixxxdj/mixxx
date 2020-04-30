#include "effects/backends/builtin/choppereffect.h"

#include "util/math.h"
#include "util/rampingvalue.h"
#include "util/sample.h"

// static
QString ChopperEffect::getId() {
    return QStringLiteral("org.mixxx.effects.chopper");
}

// static
EffectManifestPointer ChopperEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());

    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Chopper"));
    pManifest->setShortName(QObject::tr("Chopper"));
    pManifest->setAuthor(QStringLiteral("The Mixxx Team"));
    pManifest->setVersion(QStringLiteral("1.0"));
    pManifest->setDescription(QObject::tr(
            "Stores the input signal in a temporary buffer and loops it"));
    pManifest->setMetaknobDefault(db2ratio(-3.0));

    EffectManifestParameterPointer length = pManifest->addParameter();
    length->setId(QStringLiteral("loop_length"));
    length->setName(QObject::tr("Length"));
    length->setShortName(QObject::tr("Length"));
    length->setDescription(QObject::tr(
            "Length of the loop\n"
            "1/8 - 2 beats if tempo is detected\n"
            "1/8 - 2 seconds if no tempo is detected"));
    length->setUnitsHint(EffectManifestParameter::UnitsHint::Beats);
    length->setRange(0.0, 0.5, 2.0);

    EffectManifestParameterPointer quantize = pManifest->addParameter();
    quantize->setId(QStringLiteral("quantize"));
    quantize->setName(QObject::tr("Quantize"));
    quantize->setShortName(QObject::tr("Quantize"));
    quantize->setDescription(QObject::tr(
            "Round the Time parameter to the nearest 1/4 beat."));
    quantize->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    quantize->setRange(0, 1, 1);

    EffectManifestParameterPointer reverse = pManifest->addParameter();
    reverse->setId(QStringLiteral("reverse"));
    reverse->setName(QObject::tr("Reverse"));
    reverse->setShortName(QObject::tr("Reverse"));
    reverse->setDescription(QObject::tr(
            "Play the recorded loop reversed"));
    reverse->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    reverse->setRange(0, 0, 1);

    EffectManifestParameterPointer mix = pManifest->addParameter();
    mix->setId(QStringLiteral("mix"));
    mix->setName(QObject::tr("Mix"));
    mix->setShortName(QObject::tr("Mix"));
    mix->setDescription(QObject::tr(
            "Crossfade between the input signal and the loop"));
    mix->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    mix->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    mix->setRange(0.0, 0.5, 1.0);

    EffectManifestParameterPointer triplet = pManifest->addParameter();
    triplet->setId(QStringLiteral("triplet"));
    triplet->setName(QObject::tr("Triplets"));
    triplet->setShortName(QObject::tr("Triplets"));
    triplet->setDescription(
            QObject::tr("When the Quantize parameter is enabled, divide "
                        "rounded 1/4 beats of Time parameter by 3."));
    triplet->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    triplet->setRange(0, 0, 1);

    return pManifest;
}

void ChopperEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pLengthParameter = parameters.value(QStringLiteral("loop_length"));
    m_pQuantizeParameter = parameters.value(QStringLiteral("quantize"));
    m_pReverseParameter = parameters.value(QStringLiteral("reverse"));
    m_pMixParameter = parameters.value(QStringLiteral("mix"));
    m_pTripletParameter = parameters.value(QStringLiteral("triplet"));
}

void ChopperEffect::processChannel(
        ChopperState* pGroupState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    ChopperState& gs = *pGroupState;
    // The minimum of the parameter is zero so the exact center of the knob is 1 beat.

    double mixValue = m_pMixParameter->value();
    double period = m_pLengthParameter->value();

    if (enableState == EffectEnableState::Enabling) {
        gs.clear();
    }

    int loopFrames = 0;
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
        loopFrames = static_cast<int>(
                period * groupFeatures.beat_length_sec * engineParameters.sampleRate());
    } else {
        // period is a number of seconds
        period = std::max(period, 1 / 8.0);
        loopFrames = static_cast<int>(period * engineParameters.sampleRate());
    }
    VERIFY_OR_DEBUG_ASSERT(loopFrames > 0) {
        loopFrames = 1;
    }

    int loopSamples = loopFrames * engineParameters.channelCount();
    VERIFY_OR_DEBUG_ASSERT(loopSamples <= gs.loop.size()) {
        loopSamples = gs.loop.size();
    }
    if (m_pReverseParameter->toBool()) {
        gs.selectedLoop = gs.reverseLoop.data();
        gs.startPos = gs.loop.size() - loopSamples;
        gs.endPos = gs.loop.size();
    } else {
        gs.startPos = 0;
        gs.endPos = loopSamples;
        gs.selectedLoop = gs.loop.data();
    }

    RampingValue<CSAMPLE_GAIN> mixRamp(
            static_cast<CSAMPLE_GAIN>(mixValue),
            static_cast<CSAMPLE_GAIN>(gs.mixPrev),
            engineParameters.framesPerBuffer());

    for (unsigned int frame = 0;
            frame < engineParameters.samplesPerBuffer();
            frame += engineParameters.channelCount()) {
        for (int channel = 0; channel < engineParameters.channelCount(); channel++) {
            CSAMPLE_GAIN mixRamped = mixRamp.getNext();
            // record loop
            if (gs.isRecording) {
                if (gs.writeSamplePos >= gs.loop.size()) {
                    gs.isRecording = false;
                } else {
                    gs.loop[gs.writeSamplePos] = pInput[frame + channel];
                    gs.reverseLoop[gs.loop.size() - gs.writeSamplePos - 1] =
                            pInput[frame + channel];
                    gs.writeSamplePos += 1;
                }
            }
            // if there is nothing to output yet passthrough dry signal??
            CSAMPLE wetValue = pInput[frame + channel];
            // play loop
            if (gs.writeSamplePos >= loopSamples) {
                if (gs.readSamplePos >= gs.endPos) {
                    gs.readSamplePos = gs.startPos;
                }
                wetValue = gs.selectedLoop[gs.readSamplePos];
                gs.readSamplePos += 1;
            }
            pOutput[frame + channel] =
                    pInput[frame + channel] * (1 - mixRamped) +
                    wetValue * mixRamped;
        }
    }
    gs.mixPrev = mixValue;
}
