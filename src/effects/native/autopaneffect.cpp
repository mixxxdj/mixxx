#include "util/math.h"
#include <QtDebug>

#include "effects/native/autopaneffect.h"

#include "sampleutil.h"

// static
QString AutoPanEffect::getId() {
    return "org.mixxx.effects.pan";
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
    strength->setId("strength");
    strength->setName(QObject::tr("Strength"));
    strength->setDescription(
            QObject::tr("How fast the signal goes from a channel to an other"));
    strength->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    strength->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    strength->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    strength->setMinimum(0.0);
    strength->setMaximum(0.5);  // there are two steps per period so max is half
    strength->setDefault(0.5);
    
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
    period->setDefault(50.0);
    
    // This only extists for testing and finding the best value
    // TODO(jclaveau) : remove when a satisfying value is chosen
    // The current value is taken with a simple laptop soundcard with a sample
    // rate of 44100
    EffectManifestParameter* ramping = manifest.addParameter();
    ramping->setId("ramping_threshold");
    ramping->setName(QObject::tr("Ramping"));
    ramping->setDescription("");
    ramping->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    ramping->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    ramping->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    ramping->setMinimum(0.000000000001f);
    ramping->setMaximum(0.015978f); // max good value with my soundcard 
    ramping->setDefault(0.003f);    // best value
    
    // This control is hidden for the moment in Deere (replace by the previous
    // one wich would be useless soon)
    EffectManifestParameter* depth = manifest.addParameter();
    depth->setId("depth");
    depth->setName(QObject::tr("Depth"));
    depth->setDescription("Controls the intensity of the effect.");
    depth->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    depth->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    depth->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    depth->setMinimum(0.0);
    depth->setMaximum(1.0);
    depth->setDefault(1.0);
    
    return manifest;
}

AutoPanEffect::AutoPanEffect(EngineEffect* pEffect, const EffectManifest& manifest)
        : 
          m_pDepthParameter(pEffect->getParameterById("depth")),
          m_pStrengthParameter(pEffect->getParameterById("strength")),
          m_pPeriodParameter(pEffect->getParameterById("period")),
          m_pRampingParameter(pEffect->getParameterById("ramping_threshold"))
           {
    Q_UNUSED(manifest);
}

AutoPanEffect::~AutoPanEffect() {
}

void AutoPanEffect::processGroup(const QString& group, PanGroupState* pGroupState,
                              const CSAMPLE* pInput,
                              CSAMPLE* pOutput, const unsigned int numSamples,
                              const unsigned int sampleRate,
                              const EffectProcessor::EnableState enableState,
                              const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    Q_UNUSED(groupFeatures);
    Q_UNUSED(sampleRate);
    
    PanGroupState& gs = *pGroupState;
    
    if (enableState == EffectProcessor::DISABLED) {
        return;
    }
    
    CSAMPLE period = roundf(m_pPeriodParameter->value()) * (float)numSamples / 2.0f;
    CSAMPLE stepFrac = m_pStrengthParameter->value();
    CSAMPLE depth = m_pDepthParameter->value();
    float rampingTreshold = m_pRampingParameter->value();
    
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
    
    gs.frac.setRamping(rampingTreshold);
    gs.frac.ramped = false;     // just for debug
    
    for (unsigned int i = 0; i + 1 < numSamples; i += 2) {
        
        CSAMPLE periodFraction = CSAMPLE(gs.time) / period;
        
        // current quarter in the trigonometric circle
        float quarter = floorf(periodFraction * 4.0f);
        
        // part of the period fraction being a step (not in the slope)
        CSAMPLE stepsFractionPart = floorf((quarter+1.0f)/2.0f) * stepFrac;
        
        // float inInterval = fmod( periodFraction, (period / 2.0) );
        float inStepInterval = fmod(periodFraction, 0.5f);
        
        CSAMPLE position;
        if (inStepInterval > u && inStepInterval < (u + stepFrac)) {
            // at full left or full right
            position = quarter < 2.0f ? 0.25f : 0.75f;
        } else {
            // in the slope (linear function)
            position = (periodFraction - stepsFractionPart) * a;
        }
        
        // transform the position into a sinusoid (but between 0 and 1)
        gs.frac = (sin(M_PI * 2.0f * position) + 1.0f) / 2.0f;
        
        pOutput[i] = pInput[i] * (1 - depth)
            + pInput[i] * gs.frac * lawCoef * depth;
        
        pOutput[i+1] = pInput[i+1] * (1 - depth)
            + pInput[i+1]  * (1.0f - gs.frac) * lawCoef * depth;
        
        
        gs.time++;
    }
    
    
    qDebug()
        << "| ramped :" << gs.frac.ramped
        << "| frac :" << gs.frac
        << "| time :" << gs.time
        << "| rampingTreshold :" << rampingTreshold
        ;
}

