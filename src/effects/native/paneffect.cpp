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
    
    EffectManifestParameter* depth = manifest.addParameter();
    depth->setId("depth");
    depth->setName(QObject::tr("Depth"));
    depth->setDescription("Controls the intensity of the effect.");
    depth->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    depth->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    depth->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    depth->setMinimum(0.0);
    depth->setMaximum(1.0);
    depth->setDefault(0.5);
    
    EffectManifestParameter* strength = manifest.addParameter();
    strength->setId("strength");
    strength->setName(QObject::tr("Strength"));
    strength->setDescription(
            QObject::tr("How fast the signal goes from a channel to an other"));
    strength->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    strength->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    strength->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    strength->setMinimum(0.1);
    strength->setMaximum(0.9);
    strength->setDefault(0.5);
    
    EffectManifestParameter* period = manifest.addParameter();
    period->setId("period");
    period->setName(QObject::tr("Period"));
    period->setDescription("Controls the speed of the effect.");
    period->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    period->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    period->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    period->setDefault(250.0);
    period->setMinimum(1.0);
    period->setMaximum(500.0);
    
    return manifest;
}

PanEffect::PanEffect(EngineEffect* pEffect, const EffectManifest& manifest)
        : 
          m_pDepthParameter(pEffect->getParameterById("depth")),
          m_pStrengthParameter(pEffect->getParameterById("strength")),
          m_pPeriodParameter(pEffect->getParameterById("period"))
           {
    Q_UNUSED(manifest);
}

PanEffect::~PanEffect() {
    //qDebug() << debugString() << "destroyed";
}

void PanEffect::processGroup(const QString& group, PanGroupState* pGroupState,
                              const CSAMPLE* pInput,
                              CSAMPLE* pOutput, const unsigned int numSamples,
                              const unsigned int sampleRate,
                              const EffectProcessor::EnableState enableState,
                              const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    Q_UNUSED(enableState);
    Q_UNUSED(groupFeatures);
    Q_UNUSED(sampleRate);
    
    PanGroupState& gs = *pGroupState;
    
    int read_position = gs.write_position;
    
    CSAMPLE lfoPeriod = roundf(m_pPeriodParameter->value());
    CSAMPLE strength = m_pStrengthParameter->value();
    
    gs.time++;
    // gs.time += strength * ;
    // gs.time += abs(lfoPeriod / 2.0f - gs.time) / lfoPeriod;
    if (gs.time > lfoPeriod) {
        gs.time = 0;
    }
    
    CSAMPLE periodFraction = CSAMPLE(gs.time) / lfoPeriod;
    
    // size of a step (stuck on 0.25 or 0.75)
    float stepSize = strength * lfoPeriod / 2.0;
    
    // coef of the slope
    float a = 1 / 4 / (lfoPeriod - stepSize * 2);
    
    // merging of tests for 
    float inInterval = CSAMPLE(gs.time);
    // float inInterval = CSAMPLE(gs.time) % (lfoPeriod / 2.0);
    
    // size of a segment of slope
    float u = ( lfoPeriod / 2.0 - stepSize ) / 2.0;
    
    // bool firstHalf = (periodFraction > 0.5);
    CSAMPLE position;
    if (inInterval > u && inInterval < ( u + stepSize)) {
        position = 0.25;
    } else if( inInterval > (3.0 * u + stepSize)
                && inInterval < (3.0 * u + 2.0 * stepSize) ){
        // position should be stuck on 0.25 or 0.75
        // Is the position in the first or second half of the period?
        // position = periodFraction > 0.5 ? 0.25 : 0.75;
        position = 0.75;
    } else  {
        // position should be in the slope
        position = periodFraction * a;
    }
    
    
    // 0.8 > strength > 0.2
    
    /*
    CSAMPLE clampedPeriod = roundf(strength * lfoPeriod);
    CSAMPLE position = (periodFraction * clampedPeriod + (lfoPeriod - clampedPeriod) / 2) / lfoPeriod;
    */
    
    
    // get a sinusoid
    // CSAMPLE frac1 = sin(M_PI * 2.0f * periodFraction);
    // CSAMPLE frac1 = sin(M_PI * 2.0f * position);
    // add strength to the signal (keep an impair as power to avoid loss of sign)
    // CSAMPLE frac2 = frac1 * strength;
    // CSAMPLE frac2 = frac1;
    // CSAMPLE frac3 = CSAMPLE_clamp(frac2);
    // set the curve between 0 and 1
    CSAMPLE frac = (sin(M_PI * 2.0f * position) + 1.0f) / 2.0f;
    // CSAMPLE frac = (frac1 + 1.0f) / 2.0f;
    qDebug() << "strength" << strength << "| position :" << (roundf(position * 100.0) / 100.0)
        << "| time :" << gs.time << "| period :" << lfoPeriod
        // << "| clamped period :" << clampedPeriod
        << "| frac :" << frac;
    
    // qDebug() << "frac" << frac << "| 1 :" << frac1 << "| 2: " << frac2 << "| 3 :" << frac3;
    
    // Pingpong the output.  If the pingpong value is zero, all of the
    // math below should result in a simple copy of delay buf to pOutput.
    for (unsigned int i = 0; i + 1 < numSamples; i += 2) {
        
        pOutput[i] = 
              (pInput[i] + pInput[i + 1]) * frac;
        
        pOutput[i + 1] =
              (pInput[i] + pInput[i + 1]) * (1 - frac);
        
    }
}
