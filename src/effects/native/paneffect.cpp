#include <QtDebug>

#include "effects/native/paneffect.h"

#include "sampleutil.h"

#define INCREMENT_RING(index, increment, length) index = (index + increment) % length
#define RAMP_LENGTH 500

// static
QString PanEffect::getId() {
    return "org.mixxx.effects.pan";
}

// static
EffectManifest PanEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Pan"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr("Simple Pan"));
    
    EffectManifestParameter* strength = manifest.addParameter();
    strength->setId("strength");
    strength->setName(QObject::tr("Strength"));
    strength->setDescription(
            QObject::tr("How fast the signal goes from a channel to an other"));
    strength->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    strength->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    strength->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    strength->setMinimum(0.0);
    strength->setMaximum(0.5);
    // strength->setDefault(0.25);
    strength->setDefault(0.5);
    
    EffectManifestParameter* period = manifest.addParameter();
    period->setId("period");
    period->setName(QObject::tr("Period"));
    period->setDescription("Controls the speed of the effect.");
    period->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    period->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    period->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    period->setMinimum(1.0);
    period->setMaximum(500.0);
    // period->setDefault(250.0);
    period->setDefault(50.0);
    
    EffectManifestParameter* ramping = manifest.addParameter();
    ramping->setId("ramping_treshold");
    ramping->setName(QObject::tr("Ramping"));
    ramping->setDescription("");
    ramping->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    ramping->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    ramping->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    ramping->setMinimum(0.000000000001f);
    ramping->setMaximum(0.015978f);
    // ramping->setDefault(250.0);
    ramping->setDefault(0.003f);
    // 0.00387852 => other good value
    
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

PanEffect::PanEffect(EngineEffect* pEffect, const EffectManifest& manifest)
        : 
          m_pDepthParameter(pEffect->getParameterById("depth")),
          m_pStrengthParameter(pEffect->getParameterById("strength")),
          m_pPeriodParameter(pEffect->getParameterById("period")),
          m_pRampingParameter(pEffect->getParameterById("ramping_treshold"))
           {
    oldFrac = -1.0f;
    Q_UNUSED(manifest);
}

PanEffect::~PanEffect() {
}

void PanEffect::processGroup(const QString& group, PanGroupState* pGroupState,
                              const CSAMPLE* pInput,
                              CSAMPLE* pOutput, const unsigned int numSamples,
                              const unsigned int sampleRate,
                              const EffectProcessor::EnableState enableState,
                              const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    Q_UNUSED(groupFeatures);
    Q_UNUSED(sampleRate);
    
    PanGroupState& gs = *pGroupState;
    
    // if (EnableState::DISABLED == enableState) {
    if (0x00 == enableState) {
        return; // DISABLED = 0x00
    }
    
    CSAMPLE period = roundf(m_pPeriodParameter->value()) * (float)numSamples / 2.0f;
    CSAMPLE stepFrac = m_pStrengthParameter->value();
    CSAMPLE depth = m_pDepthParameter->value();
    float rampingTreshold = m_pRampingParameter->value();
    
    if (gs.time > period || 0x03 == enableState) { // ENABLING = 0x03
        gs.time = 0;
    }
    
    // coef of the slope
    // a = (y2 - y1) / (x2 - x1)
    //       1  / ( 1 - 2 * stepfrac)
    float a;
    if( stepFrac != 0.5f ){
        a = 1.0f / (1.0f - stepFrac * 2.0f);
    } else {
        a = 0.0001f;
    }
    
    // define the increasing of gain when only one output channel is used
    float lawCoef =  1.0f / sqrtf(2.0f) * 2.0f;
    
    // size of a segment of slope
    float u = ( 0.5f - stepFrac ) / 2.0f;
    
    // todo (jclaveau) : stereo
    SampleUtil::mixStereoToMono(pOutput, pInput, numSamples);
    
    /** stereo : 
     * + position 0 <=> 0.5 <=> 1 => gauche et droite au milieu
     * + position 0.25 => droite à droite et gauche à droite
     * + position 0.75 => droite à gauche et gauche à gauche
     * law coef         : sqrt(2)   1           sqrt(2)
     * position         : 1         0           0.5
     * 
     * position left    : 
     * channel left     : 0     ->  pi/4    ->  pi/2    ->  pi/4
     * 
     * position right   : 
     * channel right    : pi/2  ->  3pi/4   ->  pi      ->  3pi/4
     * 
     */
    
    // RampedSample* frac = new RampedSample();
    // frac->setRamping(rampingTreshold);
    RampedSample frac(rampingTreshold);
    // frac.setRamping(rampingTreshold);
    for (unsigned int i = 0; i + 1 < numSamples; i += 2) {
        
        CSAMPLE periodFraction = CSAMPLE(gs.time) / period;
        
        // current quarter in the trigonometric circle
        float quarter = floorf(periodFraction * 4.0f);
        
        // part of the period fraction being steps (not in the slope)
        CSAMPLE stepsFractionPart = floorf((quarter+1.0f)/2.0f) * stepFrac;
        
        // float inInterval = fmod( periodFraction, (period / 2.0) );
        float inInterval = fmod( periodFraction, 0.5f );
        
        
        CSAMPLE position;
        if (inInterval > u && inInterval < ( u + stepFrac)) {
            // at full left or full right
            position = quarter < 2.0f ? 0.25f : 0.75f;
        } else {
            // in the slope (linear function)
            position = (periodFraction - stepsFractionPart) * a;
        }
        
        // transform the position into a sinusoid (between 0 and 1)
        frac = (sinf(M_PI * 2.0f * position) + 1.0f) / 2.0f;
        
        // todo (jclaveau) for stereo :
        // frac left in left
        // frac left in right
        // frac right in left
        // frac right in right
        
        
        // 
        /*
        if (oldFrac != -1.0f) {
            float diff = frac - oldFrac;
            
            if( fabs(diff) >= rampingTreshold ){
                frac = oldFrac + (diff / fabs(diff) * rampingTreshold);
            }
            
            if( fabs(diff) > maxDiff ){
                maxDiff = fabs(diff);
            }
        } 
        
        oldFrac = frac;
        */
        
        
        pOutput[i] = pOutput[i] * (1 - depth)
            + pOutput[i] * frac * lawCoef * depth;
        
        pOutput[i+1] = pOutput[i+1] * (1 - depth)
            + pOutput[i+1]  * (1.0f - frac) * lawCoef * depth;
        
        
        gs.time++;
    }
    
    
    /** /
    qDebug() << "stepFrac" << stepFrac
        << "| time :" << gs.time
        // << "| period :" << period
        << "| a :" << a          // coef of slope between 1 and -1
        // << "| lawCoef :" << lawCoef
        // << "| enableState :" << enableState
        // << "| numSamples :" << numSamples
        ;
    
    /**/
    
    
    qDebug()
        // <    < "| position :" << (roundf(position * 100.0f) / 100.0f)
        << "| frac :" << frac
        // << "| maxDiff :" << maxDiff
        // << "| maxDiff2 :" << maxDiff2
        << "| time :" << gs.time
        << "| rampingTreshold :" << rampingTreshold
        ;
        // << "| a :" << a          // coef of slope between 1 and -1
        // << "| numSamples :" << numSamples;
    
    /**/
    // qDebug() << "pOutput[1]" << pOutput[1] << "| pOutput[2]" << pOutput[2];
}

