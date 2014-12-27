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
    depth->setDefault(1.0);
    
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
    strength->setDefault(0.0);
    
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
    period->setDefault(150.0);
    
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

// todo : ramping signal carré
// todo : 


void PanEffect::processGroup(const QString& group, PanGroupState* pGroupState,
                              const CSAMPLE* pInput,
                              CSAMPLE* pOutput, const unsigned int numSamples,
                              const unsigned int sampleRate,
                              const EffectProcessor::EnableState enableState,
                              const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    // Q_UNUSED(enableState);
    Q_UNUSED(groupFeatures);
    Q_UNUSED(sampleRate);
    
    PanGroupState& gs = *pGroupState;
    
    // if (EnableState::DISABLED == enableState) {
    if (2 == enableState) {
    // if (true) {
        gs.time = 0; // DISABLING = 2
        return;
    }
    
    int read_position = gs.write_position;
    
    CSAMPLE lfoPeriod = roundf(m_pPeriodParameter->value());
    CSAMPLE stepFrac = m_pStrengthParameter->value();
    CSAMPLE depth = m_pDepthParameter->value();
    
    gs.time++;
    if (gs.time > lfoPeriod) {
        gs.time = 0;
    }
    
    CSAMPLE periodFraction = CSAMPLE(gs.time) / lfoPeriod;
    
    // size of a step (while stuck on 0.25 or 0.75)
    // todo rename as stepFrac
    // float stepSize = stepFrac * lfoPeriod / 2.0;
    
    // coef of the slope
    // a = (y2 - y1) / (x2 - x1)
    //       1  / ( 1 - 2 * stepfrac)
    float a = 1.0f / (1.0f - stepFrac * 2.0f);
    
    // merging of tests for 
    // float inInterval = fmod( CSAMPLE(gs.time), (lfoPeriod / 2.0) );
    float inInterval = fmod( periodFraction, 0.5f );
    
    // size of a segment of slope
    float u = ( 0.5f - stepFrac ) / 2.0f;
    
    float quarter = floorf(periodFraction * 4.0f);
    
    CSAMPLE position;
    if (inInterval > u && inInterval < ( u + stepFrac)) {
        // position should be stuck on 0.25 or 0.75
        // Is the position in the first or second half of the period?
        position = quarter < 2.0f ? 0.25f : 0.75f;
    } else {
        // qDebug() << "calcul | a : " << a << "| step : " << stepFrac;
        // position should be in the slope
        position = (periodFraction - (floorf((quarter+1.0f)/2.0f) * stepFrac )) * a;
    }
    
    // set the curve between 0 and 1
    CSAMPLE frac = (sin(M_PI * 2.0f * position) + 1.0f) / 2.0f;
    // CSAMPLE frac = sin(M_PI * 2.0f * position) + 1.0f;
    // frac = CSAMPLE_clamp(frac);
    float lawCoef =  1.0f / sqrtf(2.0f) * 2.0f;
    // float lawCoef =  1.0f / sqrtf(2.0f);
    
    qDebug() << "stepFrac" << stepFrac
        << "| position :" << (roundf(position * 100.0f) / 100.0f)
        << "| time :" << gs.time
        << "| period :" << lfoPeriod
        // << "| a :" << a          // coef of slope between 1 and -1
        // << "| q :" << quarter    // current quarter in the trigo circle
        // << "| q+ :" << floorf((quarter+1.0f)/2.0f)
        << "| vol_coef :" << frac
        << "| pos_coef :" << frac
        // << "| lawCoef :" << lawCoef
        << "| enableState :" << enableState
        << "| numSamples :" << numSamples;
    
    // qDebug() << "frac" << frac << "| 1 :" << frac1 << "| 2: " << frac2 << "| 3 :" << frac3;
    
    SampleUtil::mixStereoToMono(pOutput, pInput, numSamples);
    
    // Pingpong the output.  If the pingpong value is zero, all of the
    // math below should result in a simple copy of delay buf to pOutput.
    for (unsigned int i = 0; i + 1 < numSamples; i += 2) {
        
        pOutput[i] = pOutput[i] * (1 - depth)
            + pOutput[i] * frac * lawCoef * depth;
        pOutput[i+1] = pOutput[i+1] * (1 - depth)
            + pOutput[i+1]  * (1.0f - frac) * lawCoef * depth;
        
    }
    
    // qDebug() << "pOutput[1]" << pOutput[1] << "| pOutput[2]" << pOutput[2];
}


// float fmod(float a, float b) {
    // return (a/b) - ( (int)(a/b) * b);
// }

