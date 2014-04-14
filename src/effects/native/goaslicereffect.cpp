#include "effects/native/goaslicereffect.h"

// static
QString GoaSlicerEffect::getId() {
    return "org.mixxx.effects.goaslicer";
}

// static
EffectManifest GoaSlicerEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("GoaSlicer"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription("TODO");

    EffectManifestParameter* length = manifest.addParameter();
    length->setId("length");
    length->setName(QObject::tr("Length"));
    length->setDescription("TODO");
    length->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    length->setValueHint(EffectManifestParameter::VALUE_INTEGRAL);
    length->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    length->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    length->setLinkHint(EffectManifestParameter::LINK_LINKED);
    length->setDefault(2048);
    length->setMinimum(1);
    length->setMaximum(8192);

    EffectManifestParameter* slope = manifest.addParameter();
    slope->setId("slope");
    slope->setName(QObject::tr("Slope"));
    slope->setDescription("TODO");
    slope->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    slope->setValueHint(EffectManifestParameter::VALUE_INTEGRAL);
    slope->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    slope->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    slope->setDefault(512);
    slope->setMinimum(1);
    slope->setMaximum(2048);

    return manifest;
}

GoaSlicerEffect::GoaSlicerEffect(EngineEffect* pEffect,
                           const EffectManifest& manifest)
        : m_pLengthParameter(pEffect->getParameterById("length")),
          m_pSlopeParameter(
              pEffect->getParameterById("slope")) {
    Q_UNUSED(manifest);
}

GoaSlicerEffect::~GoaSlicerEffect() {
    qDebug() << debugString() << "destroyed";
}

void GoaSlicerEffect::processGroup(const QString& group,
                                GoaSlicerGroupState* pState,
                                const CSAMPLE* pInput, CSAMPLE* pOutput,
                                const unsigned int numSamples,
                                const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    unsigned int length = m_pLengthParameter ?
            m_pLengthParameter->value().toInt() : 0;
    unsigned int slope = m_pSlopeParameter ?
            m_pSlopeParameter->value().toInt() : 0;

#if 0
    // TODO(rryan) what if bandpass_gain changes?
    bool parametersChanged = depth != pState->oldDepth ||
            bandpass_width != pState->oldBandpassWidth;
    if (parametersChanged) {
        if (pState->oldDepth == 0.0) {
            SampleUtil::copyWithGain(
                pState->crossfadeBuffer, pInput, 1.0, numSamples);
        } else if (pState->oldDepth == -1.0 || pState->oldDepth == 1.0) {
            SampleUtil::copyWithGain(
                pState->crossfadeBuffer, pInput, 0.0, numSamples);
        } else {
            applyFilters(pState,
                         pInput, pState->crossfadeBuffer,
                         pState->bandpassBuffer,
                         numSamples, pState->oldDepth,
                         pState->oldBandpassGain);
        }
        if (depth < 0.0) {
            // Lowpass + bandpass
            // Freq from 2^5=32Hz to 2^(5+9)=16384
            double freq = getLowFrequencyCorner(depth + 1.0);
            double freq2 = getHighFrequencyCorner(depth + 1.0, bandpass_width);
            pState->lowFilter.setFrequencyCorners(freq2);
            pState->bandpassFilter.setFrequencyCorners(freq, freq2);
        } else if (depth > 0.0) {
            // Highpass + bandpass
            double freq = getLowFrequencyCorner(depth);
            double freq2 = getHighFrequencyCorner(depth, bandpass_width);
            pState->highFilter.setFrequencyCorners(freq);
            pState->bandpassFilter.setFrequencyCorners(freq, freq2);
        }
    }

    if (depth == 0.0) {
        SampleUtil::copyWithGain(pOutput, pInput, 1.0, numSamples);
    } else if (depth == -1.0 || depth == 1.0) {
        SampleUtil::copyWithGain(pOutput, pInput, 0.0, numSamples);
    } else {
        applyFilters(pState, pInput, pOutput, pState->bandpassBuffer,
                     numSamples, depth, bandpass_gain);
    }

    if (parametersChanged) {
        SampleUtil::linearCrossfadeBuffers(pOutput, pState->crossfadeBuffer,
                                           pOutput, numSamples);
        pState->oldDepth = depth;
        pState->oldBandpassWidth = bandpass_width;
        pState->oldBandpassGain = bandpass_gain;
    }
#endif
}
