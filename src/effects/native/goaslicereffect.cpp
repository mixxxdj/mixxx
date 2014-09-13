#include "effects/native/goaslicereffect.h"

// static
QString GoaSlicerEffect::getId() {
    return "org.mixxx.effects.goaslicer";
}

// static
EffectManifest GoaSlicerEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("GoaSlicer"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription("TODO");

    EffectManifestParameter* length = manifest.addParameter();
    length->setId("length");
    length->setName(QObject::tr("Length"));
    length->setDescription("TODO");
    length->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    length->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    length->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    length->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    length->setLinkHint(EffectManifestParameter::LINK_LINKED);
    length->setDefault(0.25);
    length->setMinimum(0.0);
    length->setMaximum(1.0);

    EffectManifestParameter* slope = manifest.addParameter();
    slope->setId("slope");
    slope->setName(QObject::tr("Slope"));
    slope->setDescription("TODO");
    slope->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    slope->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    slope->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    slope->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    slope->setDefault(0.25);
    slope->setMinimum(0.0);
    slope->setMaximum(1.0);

    // 2,4,8,16 ...
    EffectManifestParameter* period = manifest.addParameter();
    period->setId("period");
    period->setName(QObject::tr("Period"));
    period->setDescription("TODO");
    period->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    period->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    period->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    period->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    period->setDefault(0.25);
    period->setMinimum(0.0);
    period->setMaximum(1.0);
    
    return manifest;
}

GoaSlicerEffect::GoaSlicerEffect(EngineEffect* pEffect,
                           const EffectManifest& manifest)
        : m_pLengthParameter(pEffect->getParameterById("length")),
          m_pSlopeParameter(
              pEffect->getParameterById("slope")),
          m_pPeriodParameter(pEffect->getParameterById("period")) {
    Q_UNUSED(manifest);
}

GoaSlicerEffect::~GoaSlicerEffect() {
    qDebug() << debugString() << "destroyed";
}

void GoaSlicerEffect::processGroup(const QString& group,
                                GoaSlicerGroupState* pState,
                                const CSAMPLE* pInput, CSAMPLE* pOutput,
                                const unsigned int numSamples,
                                const unsigned int sampleRate,
                                const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    unsigned int length = (int)(m_pLengthParameter ?
        m_pLengthParameter->value().toFloat() * 8192.0 : 0.0);
    unsigned int slope = (int)(m_pSlopeParameter ?
        m_pSlopeParameter->value().toFloat() * 1024.0 : 256.0);
    unsigned int period = m_pPeriodParameter ?
        m_pPeriodParameter->value().toFloat: 0.25;
    
    unsigned int tick_periods[] = {2,4,8,16};
    int ticks_per_beat = tick_periods[(period * (sizeof(tick_periods)/sizeof(unsigned int)))-1];
    double slope_amount = slope * ((double)sampleRate)/(8192.0*((double)sampleRate));
    
    if (groupFeatures.has_beat_length && groupFeatures.has_beat_fraction) {
        for (unsigned int i=0;i < numSamples;i+=2) {
            double beat_pos = (i +
                (groupFeatures.beat_length * groupFeatures.beat_fraction));
            double tick_length = groupFeatures.beat_length / (double)ticks_per_beat;
            double tick_pos = fmod(beat_pos, tick_length);
            double slope_pos = (length*2.0) - tick_pos;
            bool muted = slope_pos <= 0.0 ? true : false;
            double current_volume = 1.0 - (((slope_pos/tick_length) * slope_amount) / 2.0);
            
            if (muted) {
                pOutput[i] = 0.0;
                pOutput[i+1] = 0.0;
            }
            else {
                pOutput[i] = pInput[i] * current_volume;
                pOutput[i+1] = pInput[i] * current_volume;
            }
            /*
            qDebug()
                << "Muted:" << muted
                << "Volume:" << current_volume
                << "Beat Length:" << groupFeatures.beat_length
                << "Tick Length:" << tick_length
                << "pos={"
                    << "i:" << i
                    << "beat:" << beat_pos
                    << "tick:" << tick_pos
                    << "slope:" << slope_pos
                << "}";
            */
        }
    }
    else {
        for (unsigned int i=0;i < numSamples;i+=2) {
            pOutput[i] = pInput[i];
            pOutput[i+1] = pInput[i];
        }
    }
    //qDebug() << "Ticks:" << ticks << "Tick Length:" << tick_length;
    /*
    qDebug()
        << "Has beat length: "
        << groupFeatures.has_beat_length
        << " fraction:"
        << groupFeatures.has_beat_fraction
        << "pos / length = "
        << groupFeatures.beat_fraction
        << " "
        << groupFeatures.beat_length
        << "slope:" << slope << "amount:" << slope_amount;
    */
}
