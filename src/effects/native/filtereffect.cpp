#include "effects/native/filtereffect.h"

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
    depth->setValueHint(EffectManifestParameter::EffectManifestParameter::VALUE_FLOAT);
    depth->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    depth->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
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
}

FilterEffect::~FilterEffect() {
    qDebug() << debugString() << "destroyed";
    for (QMap<QString, GroupState*>::iterator it = m_groupState.begin();
         it != m_groupState.end();) {
        delete it.value();
        it = m_groupState.erase(it);
    }
}

double getLowFrequencyCorner(double depth) {
    return pow(2.0, 5.0 + depth * 9.0);
}

double getHighFrequencyCorner(double depth, double bandpassSize) {
    return pow(2.0, 5.0 + (depth + bandpassSize) * 9.0);
}

void FilterEffect::process(const QString& group,
                           const CSAMPLE* pInput, CSAMPLE* pOutput,
                           const unsigned int numSamples) {
    GroupState* pGroupState = m_groupState.value(group, NULL);
    if (pGroupState == NULL) {
        pGroupState = new GroupState();
        m_groupState[group] = pGroupState;
    }
    GroupState& group_state = *pGroupState;

    double depth = m_pDepthParameter ?
            m_pDepthParameter->value().toDouble() : 0.0;
    double bandpass_width = m_pBandpassWidthParameter ?
            m_pBandpassWidthParameter->value().toDouble() : 0.0;
    CSAMPLE bandpass_gain = m_pBandpassGainParameter ?
            m_pBandpassGainParameter->value().toFloat() : 0.0;

    // TODO(rryan) what if bandpass_gain changes?
    bool parametersChanged = depth != group_state.oldDepth ||
            bandpass_width != group_state.oldBandpassWidth;
    if (parametersChanged) {
        if (group_state.oldDepth == 0.0) {
            SampleUtil::copyWithGain(
                group_state.crossfadeBuffer, pInput, 1.0, numSamples);
        } else if (group_state.oldDepth == -1.0 || group_state.oldDepth == 1.0) {
            SampleUtil::copyWithGain(
                group_state.crossfadeBuffer, pInput, 0.0, numSamples);
        } else {
            applyFilters(group_state,
                         pInput, group_state.crossfadeBuffer,
                         group_state.bandpassBuffer,
                         numSamples, group_state.oldDepth,
                         group_state.oldBandpassGain);
        }
        if (depth < 0.0) {
            // Lowpass + bandpass
            // Freq from 2^5=32Hz to 2^(5+9)=16384
            double freq = getLowFrequencyCorner(depth + 1.0);
            double freq2 = getHighFrequencyCorner(depth + 1.0, bandpass_width);
            group_state.lowFilter.setFrequencyCorners(freq2);
            group_state.bandpassFilter.setFrequencyCorners(freq, freq2);
        } else if (depth > 0.0) {
            // Highpass + bandpass
            double freq = getLowFrequencyCorner(depth);
            double freq2 = getHighFrequencyCorner(depth, bandpass_width);
            group_state.highFilter.setFrequencyCorners(freq);
            group_state.bandpassFilter.setFrequencyCorners(freq, freq2);
        }
    }

    if (depth == 0.0) {
        SampleUtil::copyWithGain(pOutput, pInput, 1.0, numSamples);
    } else if (depth == -1.0 || depth == 1.0) {
        SampleUtil::copyWithGain(pOutput, pInput, 0.0, numSamples);
    } else {
        applyFilters(group_state, pInput, pOutput, group_state.bandpassBuffer,
                     numSamples, depth, bandpass_gain);
    }

    if (parametersChanged) {
        SampleUtil::linearCrossfadeBuffers(pOutput, group_state.crossfadeBuffer,
                                           pOutput, numSamples);
        group_state.oldDepth = depth;
        group_state.oldBandpassWidth = bandpass_width;
        group_state.oldBandpassGain = bandpass_gain;
    }
}

void FilterEffect::applyFilters(GroupState& group_state,
                                const CSAMPLE* pInput, CSAMPLE* pOutput,
                                CSAMPLE* pTempBuffer,
                                const int numSamples,
                                double depth, CSAMPLE bandpassGain) {
    if (depth < 0.0) {
        group_state.lowFilter.process(pInput, pOutput, numSamples);
        group_state.bandpassFilter.process(pInput, pTempBuffer, numSamples);
    } else {
        group_state.highFilter.process(pInput, pOutput, numSamples);
        group_state.bandpassFilter.process(pInput, pTempBuffer, numSamples);

    }
    SampleUtil::addWithGain(pOutput, pTempBuffer, bandpassGain, numSamples);
}
