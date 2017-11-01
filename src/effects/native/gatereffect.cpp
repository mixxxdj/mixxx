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
    rate->setDefault(0);
    rate->setMaximum(1);

    EffectManifestParameter* attack = manifest.addParameter();
    attack->setId("attack");
    attack->setName(QObject::tr("Attack"));
    attack->setDescription(QObject::tr(""));
    attack->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    attack->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    attack->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    attack->setMinimum(0.1);
    attack->setDefault(0.2);
    attack->setMaximum(0.4);

    EffectManifestParameter* decay = manifest.addParameter();
    decay->setId("decay");
    decay->setName(QObject::tr("Decay"));
    decay->setDescription(QObject::tr(""));
    decay->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    decay->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    decay->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    decay->setMinimum(0.1);
    decay->setDefault(0.5);
    decay->setMaximum(0.9);

    EffectManifestParameter* sustain = manifest.addParameter();
    sustain->setId("sustain");
    sustain->setName(QObject::tr("Sustain"));
    sustain->setDescription(QObject::tr(""));
    sustain->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    sustain->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    sustain->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    sustain->setMinimum(0.1);
    sustain->setDefault(0.5);
    sustain->setMaximum(0.9);

    EffectManifestParameter* release = manifest.addParameter();
    release->setId("release");
    release->setName(QObject::tr("Release"));
    release->setDescription(QObject::tr(""));
    release->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    release->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    release->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    release->setMinimum(0.1);
    release->setDefault(0.5);
    release->setMaximum(0.9);



    EffectManifestParameter* attackCurve = manifest.addParameter();
    attackCurve->setId("attackCurve");
    attackCurve->setName(QObject::tr("Attack Curve"));
    attackCurve->setDescription(QObject::tr(""));
    attackCurve->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    attackCurve->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    attackCurve->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    attackCurve->setMinimum(0.1);
    attackCurve->setDefault(0.5);
    attackCurve->setMaximum(0.9);



    EffectManifestParameter* sustainLevel = manifest.addParameter();
    sustainLevel->setId("sustainLevel");
    sustainLevel->setName(QObject::tr("Sustain Level"));
    sustainLevel->setDescription(QObject::tr(""));
    sustainLevel->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    sustainLevel->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    sustainLevel->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    sustainLevel->setMinimum(0.1);
    sustainLevel->setDefault(0.5);
    sustainLevel->setMaximum(0.9);



    return manifest;
}

GaterEffect::GaterEffect(EngineEffect* pEffect,
                             const EffectManifest& manifest)
        : m_pRateParameter(pEffect->getParameterById("rate")),
          m_pAttackParameter(pEffect->getParameterById("attack")),
          m_pAttackCurveParameter(pEffect->getParameterById("attackCurve")),
          m_pDecayParameter(pEffect->getParameterById("decay")),
          m_pSustainParameter(pEffect->getParameterById("sustain")),
          m_pSustainLevelParameter(pEffect->getParameterById("sustainLevel")),
          m_pReleaseParameter(pEffect->getParameterById("release")) {

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

    int divider = pow(2, ((int)(4*rate)));


    //const auto shape = m_pShapeParameter->value();

    /*
    // Number of samples to reach gain 1.0
    const unsigned int attack = 1000;
    const unsigned int attack_curve = 500;
    // Number of samples to reach gain sustainLevel
    const unsigned int decay = 500;
    // Sustain level
    const double sustainLevel = 0.5;
    // Number of samples at sustainLevem
    const unsigned int sustain = 1000;
    // Number of samples to reach gain 0
    const unsigned int release = 1000;

    */
    
    // Initialization
    if(enableState == EffectProcessor::ENABLING)
    {
        /*
        std::cout << groupFeatures.has_beat_length_sec << std::endl;
        std::cout << groupFeatures.beat_length_sec << std::endl;
        std::cout << groupFeatures.has_beat_fraction << std::endl;
        std::cout << groupFeatures.beat_fraction << std::endl;
        
        std::cout << numSamples << std::endl;
        std::cout << sampleRate << std::endl;
        std::cout << fmod(groupFeatures.beat_fraction, 1.0/4) << std::endl << std::endl;

        */

        if(groupFeatures.has_beat_fraction)
        {
            double beat_length = groupFeatures.beat_length_sec;
            double beat_frac   = groupFeatures.beat_fraction;
            double period = beat_length*sampleRate/divider;
            
            //const double period = std::max(roundToFraction(1-rate, 4), 1/8.0)*beat_length*sampleRate;
            
            
            const auto attack_curve = m_pAttackParameter->value()*period;
            int decay = m_pAttackParameter->value()*period;
            int release = m_pAttackParameter->value()*period;
    
            const auto sustain_level = m_pSustainParameter->value();
    
            int sustain = (period-decay-release)/(1+m_pDecayParameter->value());
            int attack = m_pDecayParameter->value()*sustain;            
            
            std::cout << rate << std::endl;
            //std::cout << period << " (" << std::max(roundToFraction(1-rate, 4), 1/8.0) << ")" << std::endl;
            std::cout << period << " (" << divider << ")" << std::endl;
            std::cout << attack << " + " << decay << " + " << sustain << " + " << release << " = " << attack+decay+sustain+release << std::endl;
            std::cout << "Shape : " << 1.0*attack/sustain << std::endl;
            

            std::cout << "Attack : " << m_pAttackParameter->value() << std::endl;
            std::cout << "Decay : " << m_pDecayParameter->value() << std::endl;
            std::cout << "Sustain : " << m_pSustainParameter->value() << std::endl;
            
            /*
            std::cout << "Attack : " << m_pAttackParameter->value()*period << std::endl;
            std::cout << "Attack curve : " << m_pAttackCurveParameter->value()*period << std::endl;
            std::cout << "Decay : " << m_pDecayParameter->value()*period << std::endl;
            std::cout << "Sustain : " << m_pSustainParameter->value()*period << std::endl;
            std::cout << "Sustain level : " << m_pSustainLevelParameter->value() << std::endl;
            std::cout << "Release : " << m_pReleaseParameter->value()*period << std::endl;
            */
            std::cout << std::endl;
        }        
    }

    if(groupFeatures.has_beat_fraction)
    {


        double beat_length = groupFeatures.beat_length_sec;
        double beat_frac   = groupFeatures.beat_fraction;
        double period = beat_length*sampleRate/divider;
        
        //const double period = std::max(roundToFraction(1-rate, 4), 1/8.0)*beat_length*sampleRate;


        int attack_curve = m_pAttackParameter->value()*period;
        int decay = m_pAttackParameter->value()*period;
        int release = m_pAttackParameter->value()*period;

        const auto sustain_level = m_pSustainParameter->value();

        int sustain = (period-decay-release)/(1+m_pDecayParameter->value());
        int attack = m_pDecayParameter->value()*sustain;
        

        /*
        const auto attack = m_pAttackParameter->value()*period;
        const auto decay = m_pDecayParameter->value()*period;
        const auto sustain = m_pSustainParameter->value()*period;
        const auto sustain_level = m_pSustainLevelParameter->value();
        const auto release = m_pReleaseParameter->value()*period;
*/

        double attackCoef  = 1-exp(-1.0*attack/attack_curve);

        for(unsigned int i = 0; i < numSamples; i+=2)
        {
            unsigned int numSamples_readjusted = i/2 + beat_frac * groupFeatures.beat_length_sec * sampleRate;

            numSamples_readjusted = numSamples_readjusted %((int)(groupFeatures.beat_length_sec * sampleRate/divider));

            double gain = 0;
            if(numSamples_readjusted < attack)
            {
                gain = (1-exp(-1.0*numSamples_readjusted/attack_curve))/attackCoef;
            }
            else if(numSamples_readjusted < attack + decay)
            {
                gain = exp((1.0*numSamples_readjusted-attack)/decay*log(sustain_level));
            }
            else if(numSamples_readjusted < attack + decay + sustain)
            {
                gain = sustain_level;
            }
            else if(numSamples_readjusted < attack + decay + sustain + release)
            {
                gain = exp(1.0*(numSamples_readjusted-attack-decay-sustain)/release*log(1-sustain_level))+sustain_level-1;
            }

            pOutput[i] = gain*pInput[i];
            pOutput[i+1] = gain*pInput[i+1];
            file << numSamples_readjusted  << "\t" << gain << "\t" <<  pInput[i] << "\t" << pOutput[i] << std::endl;
        }
    }
}
