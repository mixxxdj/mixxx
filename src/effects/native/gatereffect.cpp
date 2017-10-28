#include "effects/native/gatereffect.h"



#include "util/sample.h"

// static
QString GaterEffect::getId() {
    return "org.mixxx.effects.gater";
}

// static
EffectManifest GaterEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Gater"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription("");

    EffectManifestParameter* rate = manifest.addParameter();
    rate->setId("rate");
    rate->setName(QObject::tr("Rate"));
    rate->setDescription(QObject::tr(""));
    rate->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    rate->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    rate->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    rate->setMinimum(0);
    rate->setDefault(0.5);
    rate->setMaximum(1);

    EffectManifestParameter* shape = manifest.addParameter();
    shape->setId("shape");
    shape->setName(QObject::tr("Shape"));
    shape->setDescription(QObject::tr(""));
    shape->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    shape->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    shape->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    shape->setMinimum(0.1);
    shape->setDefault(0.5);
    shape->setMaximum(0.9);
    return manifest;
}

GaterEffect::GaterEffect(EngineEffect* pEffect,
                             const EffectManifest& manifest)
        : m_pRateParameter(pEffect->getParameterById("rate")),
          m_pShapeParameter(pEffect->getParameterById("shape")) {

            file.open("test.dat");
    Q_UNUSED(manifest);
}

GaterEffect::~GaterEffect() {
}

void GaterEffect::processChannel(const ChannelHandle& handle,
                                GaterGroupState* pState,
                                const CSAMPLE* pInput, CSAMPLE* pOutput,
                                const unsigned int numSamples,
                                const unsigned int sampleRate,
                                const EffectProcessor::EnableState enableState,
                                const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(groupFeatures);

    const auto rate  = m_pRateParameter->value();
    const auto shape = m_pShapeParameter->value();
    
    // Initialization
    if(enableState == EffectProcessor::ENABLING)
    {
        std::cout << groupFeatures.has_beat_length_sec << std::endl;
        std::cout << groupFeatures.beat_length_sec << std::endl;
        std::cout << groupFeatures.has_beat_fraction << std::endl;
        std::cout << groupFeatures.beat_fraction << std::endl;
        
        std::cout << numSamples << std::endl;
        std::cout << sampleRate << std::endl;
        std::cout << fmod(groupFeatures.beat_fraction, 1.0/4) << std::endl << std::endl;
    }



    int divider = pow(2, ((int)4*rate));
    double slope = 0.001;
    double cycle = 1.0/divider;
    double upcycle = shape*cycle;

    if(groupFeatures.has_beat_fraction)
    {
        double beat_frac = groupFeatures.beat_fraction;
        
        for(int i = 0; i < numSamples; i+=2)
        {
            double beat_frac_readjusted = beat_frac + i/(2.0*sampleRate)/groupFeatures.beat_length_sec;
/*
            double gain = 0.5+atan(sin(2*M_PI*beat_frac_readjusted*divider/2)/(divider*shape))/M_PI;

*/  
            double position = fmod(beat_frac_readjusted, cycle);
            double gain;
            if(position < upcycle)
            {
                gain = 1-exp((position-upcycle)/slope);
            }
            else
            {
                gain = exp((position-cycle)/slope);
            }

            pOutput[i] = gain*pInput[i];
            pOutput[i+1] = gain*pInput[i+1];
          //  file << gain  << "\t" << position << "\t" << upcycle << "\t" << cycle << "\t" << pInput[i] << "\t" << pInput[i+1] << "\t" << pOutput[i] << "\t" << pOutput[i+1] << std::endl;
        }
    }
}
