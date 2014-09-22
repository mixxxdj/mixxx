#include "effects/native/filtereffect.h"
#include "util/math.h"

// static
QString FilterEffect::getId() {
    return "org.mixxx.effects.filter";
}

// static
EffectManifest FilterEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Filter"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription("TODO");

    EffectManifestParameter* depth = manifest.addParameter();
    depth->setId("depth");
    depth->setName(QObject::tr("Depth"));
    depth->setDescription("TODO");
    depth->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    depth->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    depth->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    depth->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    depth->setDefaultLinkType(EffectManifestParameter::LINK_LINKED);
    depth->setNeutralPointOnScale(0.5);
    depth->setDefault(0.0);
    depth->setMinimum(-1.0);
    depth->setMaximum(1.0);

    EffectManifestParameter* bandpass_width = manifest.addParameter();
    bandpass_width->setId("bandpass_width");
    bandpass_width->setName(QObject::tr("Bandpass Width"));
    bandpass_width->setDescription("TODO");
    bandpass_width->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    bandpass_width->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    bandpass_width->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    bandpass_width->setUnitsHint(EffectManifestParameter::UNITS_SAMPLERATE);
    bandpass_width->setDefault(0.01);
    bandpass_width->setMinimum(0.001);
    bandpass_width->setMaximum(0.01);

    EffectManifestParameter* bandpass_gain = manifest.addParameter();
    bandpass_gain->setId("bandpass_gain");
    bandpass_gain->setName(QObject::tr("Bandpass Gain"));
    bandpass_gain->setDescription("TODO");
    bandpass_gain->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    bandpass_gain->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    bandpass_gain->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    bandpass_gain->setUnitsHint(EffectManifestParameter::UNITS_SAMPLERATE);
    bandpass_gain->setDefault(0.3);
    bandpass_gain->setMinimum(0.0);
    bandpass_gain->setMaximum(1.0);

    return manifest;
}

FilterEffect::FilterEffect(EngineEffect* pEffect,
                           const EffectManifest& manifest)
        : m_pDepthParameter(pEffect->getParameterById("depth")),
          m_pBandpassWidthParameter(
              pEffect->getParameterById("bandpass_width")),
          m_pBandpassGainParameter(
              pEffect->getParameterById("bandpass_gain")) {
    Q_UNUSED(manifest);
}

FilterEffect::~FilterEffect() {
    //qDebug() << debugString() << "destroyed";
}

double getLowFrequencyCorner(double depth) {
    return pow(2.0, 5.0 + depth * 9.0);
}

double getHighFrequencyCorner(double depth, double bandpassSize) {
    return pow(2.0, 5.0 + (depth + bandpassSize) * 9.0);
}

void FilterEffect::processGroup(const QString& group,
                                FilterGroupState* pState,
                                const CSAMPLE* pInput, CSAMPLE* pOutput,
                                const unsigned int numSamples,
                                const unsigned int sampleRate,
                                const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    Q_UNUSED(groupFeatures);
    double depth = m_pDepthParameter ?
            m_pDepthParameter->value().toDouble() : 0.0;
    double bandpass_width = m_pBandpassWidthParameter ?
            m_pBandpassWidthParameter->value().toDouble() : 0.0;
    CSAMPLE bandpass_gain = m_pBandpassGainParameter ?
            m_pBandpassGainParameter->value().toFloat() : 0.0;

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
                         numSamples, sampleRate, pState->oldDepth,
                         pState->oldBandpassGain);
        }
        if (depth < 0.0) {
            // Lowpass + bandpass
            // Freq from 2^5=32Hz to 2^(5+9)=16384
            double freq = getLowFrequencyCorner(depth + 1.0);
            double freq2 = getHighFrequencyCorner(depth + 1.0, bandpass_width);
            pState->lowFilter.setFrequencyCorners(44100, freq2);
            pState->bandpassFilter.setFrequencyCorners(44100, freq, freq2);
        } else if (depth > 0.0) {
            // Highpass + bandpass
            double freq = getLowFrequencyCorner(depth);
            double freq2 = getHighFrequencyCorner(depth, bandpass_width);
            pState->highFilter.setFrequencyCorners(44100, freq);
            pState->bandpassFilter.setFrequencyCorners(44100, freq, freq2);
        }
    }

    if (depth == 0.0) {
        SampleUtil::copyWithGain(pOutput, pInput, 1.0, numSamples);
    } else if (depth == -1.0 || depth == 1.0) {
        SampleUtil::copyWithGain(pOutput, pInput, 0.0, numSamples);
    } else {
        applyFilters(pState, pInput, pOutput, pState->bandpassBuffer,
                     numSamples, sampleRate, depth, bandpass_gain);
    }

    if (parametersChanged) {
        SampleUtil::linearCrossfadeBuffers(pOutput, pState->crossfadeBuffer,
                                           pOutput, numSamples);
        pState->oldDepth = depth;
        pState->oldBandpassWidth = bandpass_width;
        pState->oldBandpassGain = bandpass_gain;
    }
}

void FilterEffect::applyFilters(FilterGroupState* pState,
                                const CSAMPLE* pInput, CSAMPLE* pOutput,
                                CSAMPLE* pTempBuffer,
                                const unsigned int numSamples,
                                const unsigned int sampleRate,
                                double depth, CSAMPLE bandpassGain) {
    Q_UNUSED(sampleRate)
    if (depth < 0.0) {
        pState->lowFilter.process(pInput, pOutput, numSamples);
        pState->bandpassFilter.process(pInput, pTempBuffer, numSamples);
    } else {
        pState->highFilter.process(pInput, pOutput, numSamples);
        pState->bandpassFilter.process(pInput, pTempBuffer, numSamples);

    }
    SampleUtil::addWithGain(pOutput, pTempBuffer, bandpassGain, numSamples);
}
