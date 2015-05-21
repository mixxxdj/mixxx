#include "util/math.h"
#include <QtDebug>

#include "effects/native/autopaneffect.h"

#include "sampleutil.h"
#include "util/experiment.h"

const float kPositionRampingThreshold = 0.002f;


// static
QString AutoPanEffect::getId() {
    return "org.mixxx.effects.autopan";
}

// static
EffectManifest AutoPanEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("AutoPan"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr("Bounce the sound from a channel "
            "to another, roughly or softly, fully or partially, fastly or slowly. "
            "A delay, inversed on each side, is added to increase the "
            "spatial move and the period can be synced with the BPM."));
    
    // Period unit
    EffectManifestParameter* periodUnit = manifest.addParameter();
    periodUnit->setId("periodUnit");
    periodUnit->setName(QObject::tr("Sync"));
    periodUnit->setDescription("Synchronizes the period with the BPM if it can be retrieved");
    periodUnit->setControlHint(EffectManifestParameter::CONTROL_TOGGLE_STEPPING);
    periodUnit->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    periodUnit->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    periodUnit->setDefault(0);
    periodUnit->setMinimum(0);
    periodUnit->setMaximum(1);
    
    // Period
    EffectManifestParameter* period = manifest.addParameter();
    period->setId("period");
    period->setName(QObject::tr("Period"));
    period->setDescription("How fast the sound goes from a side to another,"
            " following a logarithmic scale");
    period->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    period->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    period->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    period->setMinimum(0.0625);     // 1 / 16
    period->setMaximum(129.0);
    period->setDefault(8.0);
    
    // This parameter controls the easing of the sound from a side to another.
    EffectManifestParameter* smoothing = manifest.addParameter();
    smoothing->setId("smoothing");
    smoothing->setName(QObject::tr("Smoothing"));
    smoothing->setDescription(
            QObject::tr("How fast the signal goes from a channel to an other"));
    smoothing->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    smoothing->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    smoothing->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    smoothing->setMinimum(0.0);
    smoothing->setMaximum(0.5);  // there are two steps per period so max is half
    smoothing->setDefault(0.0);
    
    // Width : applied on the channel with gain reducing.
    EffectManifestParameter* width = manifest.addParameter();
    width->setId("width");
    width->setName(QObject::tr("Width"));
    width->setDescription("How far the signal goes on the left or on the right");
    width->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    width->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    width->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    width->setMinimum(0.0);
    width->setMaximum(1.0);    // 0.02 * sampleRate => 20ms
    width->setDefault(0.00);
    
    return manifest;
}

AutoPanEffect::AutoPanEffect(EngineEffect* pEffect, const EffectManifest& manifest)
        : m_pSmoothingParameter(pEffect->getParameterById("smoothing")),
          m_pPeriodUnitParameter(pEffect->getParameterById("periodUnit")),
          m_pPeriodParameter(pEffect->getParameterById("period")),
          m_pWidthParameter(pEffect->getParameterById("width")) {
    Q_UNUSED(manifest);
}

AutoPanEffect::~AutoPanEffect() {
}

void AutoPanEffect::processChannel(const ChannelHandle& handle, PanGroupState* pGroupState,
                              const CSAMPLE* pInput,
                              CSAMPLE* pOutput, const unsigned int numSamples,
                              const unsigned int sampleRate,
                              const EffectProcessor::EnableState enableState,
                              const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    
    PanGroupState& gs = *pGroupState;
    
    if (enableState == EffectProcessor::DISABLED) {
        return;
    }
    
    double periodUnit = m_pPeriodUnitParameter->value();
    
    CSAMPLE width = m_pWidthParameter->value();
    CSAMPLE period = m_pPeriodParameter->value();
    
    // when the period knob is at its max, the time is paused
    bool pausePeriod = period == 129.0;
    
    if (periodUnit == 1 && groupFeatures.has_beat_length) {
        // floor the param on on eof these values :
        // 1/16, 1/8, 1/4, 1/2, 1, 2, 4, 8, 16, 32, 64, 128
        
        int i = 0;
        while (period > m_pPeriodParameter->minimum()) {
            period /= 2;
            i++;
        }
        
        double beats = m_pPeriodParameter->minimum();
        while (i != 0.0) {
            beats *= 2;
            i--;
        }
        
        period = groupFeatures.beat_length * beats;
    } else {
        // max period is 128 seconds
        period *= sampleRate;
    }
    
    CSAMPLE stepFrac = m_pSmoothingParameter->value();
    
    if (gs.time > period || enableState == EffectProcessor::ENABLING) {
        gs.time = 0;
    }
    
    // Normally, the position goes from 0 to 1 linearly. Here we make steps at
    // 0.25 and 0.75 to have the sound fully on the right or fully on the left.
    // At the end, the "position" value can describe a sinusoid or a square
    // curve depending of the size of those steps.
    
    // coef of the slope
    // a = (y2 - y1) / (x2 - x1)
    //       1  / ( 1 - 2 * stepfrac)
    float a = stepFrac != 0.5f ? 1.0f / (1.0f - stepFrac * 2.0f) : 1.0f;
    
    // size of a segment of slope (controled by the "strength" parameter)
    float u = (0.5f - stepFrac) / 2.0f;
    
    gs.frac.setRampingThreshold(kPositionRampingThreshold);
    gs.frac.ramped = false;     // just for debug
    
    double sinusoid = 0;
    
    for (unsigned int i = 0; i + 1 < numSamples; i += 2) {
        
        CSAMPLE periodFraction = CSAMPLE(gs.time) / period;
        
        // current quarter in the trigonometric circle
        float quarter = floorf(periodFraction * 4.0f);
        
        // part of the period fraction being a step (not in the slope)
        CSAMPLE stepsFractionPart = floorf((quarter+1.0f)/2.0f) * stepFrac;
        
        // float inInterval = fmod( periodFraction, (period / 2.0) );
        float inStepInterval = fmod(periodFraction, 0.5f);
        
        CSAMPLE angleFraction;
        if (inStepInterval > u && inStepInterval < (u + stepFrac)) {
            // at full left or full right
            angleFraction = quarter < 2.0f ? 0.25f : 0.75f;
        } else {
            // in the slope (linear function)
            angleFraction = (periodFraction - stepsFractionPart) * a;
        }
        
        // transforms the angleFraction into a sinusoid.
        // The width parameter modulates the two limits. if width values 0.5,
        // the limits will be 0.25 and 0.75. If it's 0, it will be 0.5 and 0.5
        // so the sound will be stuck at the center. If it values 1, the limits 
        // will be 0 and 1 (full left and full right).
        sinusoid = sin(M_PI * 2.0f * angleFraction) * width;
        gs.frac.setWithRampingApplied((sinusoid + 1.0f) / 2.0f);

        // apply the delay
        gs.delay->process(&pInput[i], &pOutput[i],
                -0.005 * math_clamp(((gs.frac * 2.0) - 1.0f), -1.0, 1.0) * sampleRate);

        pOutput[i] *= gs.frac * 2;
        pOutput[i+1] *= (1.0f - gs.frac) * 2;
        
        if (!pausePeriod){
            gs.time++;
        }
    }
}

