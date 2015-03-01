#include "util/math.h"
#include <QtDebug>

#include "effects/native/autopaneffect.h"

#include "sampleutil.h"
#include "util/experiment.h"

const float positionRampingThreshold = 0.005f;


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
    manifest.setDescription(QObject::tr(
        "Bounce the sound from a channel to another, fastly or softly"));
    
    // TODO(jclaveau) : this doesn't look like a good name but doesn't exist on
    //  my mixer. Any suggestion?
    // This parameter controls the easing of the sound from a side to another.
    EffectManifestParameter* strength = manifest.addParameter();
    strength->setId("curve");
    strength->setName(QObject::tr("Curve"));
    strength->setDescription(
            QObject::tr("How fast the signal goes from a channel to an other"));
    strength->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    strength->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    strength->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    strength->setMinimum(0.0);
    strength->setMaximum(0.5);  // there are two steps per period so max is half
    strength->setDefault(0.25);
    
    // On my mixer, the period is defined as a multiple of the BPM
    // I wonder if I should implement it as the bouncing is really nice
    // when it is synced.
    EffectManifestParameter* period = manifest.addParameter();
    period->setId("period");
    period->setName(QObject::tr("Period"));
    period->setDescription("Controls the speed of the effect.");
    period->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    period->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    period->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    period->setMinimum(1.0);
    period->setMaximum(500.0);
    period->setDefault(220.0);
    
    // On my mixer, the period is defined as a multiple of the BPM
    // I wonder if I should implement it as the bouncing is really nice
    // when it is synced.
    EffectManifestParameter* delay = manifest.addParameter();
    delay->setId("delay");
    delay->setName(QObject::tr("delay"));
    delay->setDescription("Controls length of the delay");
    delay->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    delay->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    delay->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    delay->setMinimum(0.0);
    delay->setMaximum(500.0);
    delay->setDefault(300.0);
    
    return manifest;
}

AutoPanEffect::AutoPanEffect(EngineEffect* pEffect, const EffectManifest& manifest)
        : 
          m_pCurveParameter(pEffect->getParameterById("curve")),
          m_pPeriodParameter(pEffect->getParameterById("period")),
          m_pDelayParameter(pEffect->getParameterById("delay"))
           {
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
    Q_UNUSED(groupFeatures);
    Q_UNUSED(sampleRate);
    
    PanGroupState& gs = *pGroupState;
    
    if (enableState == EffectProcessor::DISABLED) {
        return;
    }
    
    CSAMPLE period = roundf(m_pPeriodParameter->value());
    if (groupFeatures.has_beat_length) {
        // 1/8, 1/4, 1/2, 1, 2, 4, 8, 16, 32, 64
        double beats = pow(2, floor(period * 9 / 500) - 3);
        period = groupFeatures.beat_length * beats;
    } else {
        // 500 * 2048 as max period as number of samples
        period *= 2048.0f;
    }
    
    CSAMPLE stepFrac = m_pCurveParameter->value();
    
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
    
    // define the increasing of gain (the music sounds lower if only one 
    // channel is enabled).
    // TODO(jclaveau) : Challenge this value
    float lawCoef = 1.0f / sqrtf(2.0f) * 2.0f;
    
    // size of a segment of slope (controled by the "strength" parameter)
    float u = (0.5f - stepFrac) / 2.0f;
    
    gs.frac.setRampingThreshold(positionRampingThreshold);
    gs.frac.ramped = false;     // just for debug
    
    // float delay = round(m_pDelayParameter->value());
    // gs.delay->setDelay(delay);
    // gs.delay->process(pInput, gs.m_pDelayBuf, 2048);
    
    float quarter;
    
    for (unsigned int i = 0; i + 1 < numSamples; i += 2) {
        
        CSAMPLE periodFraction = (CSAMPLE(gs.time) / period);
        
        // current quarter in the trigonometric circle
        // float quarter = floorf(periodFraction * 4.0f);
        quarter = floorf(periodFraction * 4.0f);
        
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
        
        // transform the angleFraction into a sinusoid (but between 0 and 1)
        gs.frac.setWithRampingApplied(
            (sin(M_PI * 2.0f * angleFraction) + 1.0f) / 2.0f);
        
        if (Experiment::isExperiment()) {
            // delay on the reducing channel
            
            if ( quarter == 0.0 || quarter == 3.0 ){
                // channel 1 increasing
                pOutput[i] = pInput[i] * gs.frac * lawCoef;
                pOutput[i+1] = (gs.m_pDelayBuf[i+1] + pInput[i+1]) / 2
                    * (1.0f - gs.frac) * lawCoef;
                // pOutput[i+1] = gs.m_pDelayBuf[i+1] * (1.0f - gs.frac) * lawCoef;
                
            } else {
                // channel 2 increasing
                // pOutput[i] = pInput[i] * gs.frac * lawCoef;
                pOutput[i] = (gs.m_pDelayBuf[i] + pInput[i]) / 2
                    * gs.frac * lawCoef;
                pOutput[i+1] = pInput[i+1] * (1.0f - gs.frac) * lawCoef;
            }
            
        } else if (Experiment::isBase()) {
            // delay on both sides
            pOutput[i] = (gs.m_pDelayBuf[i] + pInput[i]) / 2 * gs.frac * lawCoef;
            pOutput[i+1] = (gs.m_pDelayBuf[i+1] + pInput[i+1]) / 2 * (1.0f - gs.frac) * lawCoef;
        
        } else {
            // no delay
            pOutput[i] = pInput[i] * gs.frac * lawCoef;
            pOutput[i+1] = pInput[i+1] * (1.0f - gs.frac) * lawCoef;
        }
        
        gs.time++;
    }
    
    /**
     * todo
     * apply delay with ramping 
     * 
     */
    float delay = round(m_pDelayParameter->value());
    gs.delay->setDelay(delay);
    gs.delay->process(pInput, gs.m_pDelayBuf, 2048);
    
    /**/
    qDebug()
        // << "| ramped :" << gs.frac.ramped
        << "| quarter :" << quarter
        << "| delay :" << delay
        // << "| beat_length :" << groupFeatures.beat_length
        // << "| period :" << period
        << "| frac :" << gs.frac
        << "| time :" << gs.time
        << "| numSamples :" << numSamples
        ;
    /**/
}

