#include <QtDebug>

#include "effects/native/echoeffect.h"

#include "mathstuff.h"
#include "sampleutil.h"

#define OFFSET_RING(index, increment, length) (index + increment) % length
#define INCREMENT_RING(index, increment, length) index = (index + increment) % length

// static
QString EchoEffect::getId() {
    return "org.mixxx.effects.echo";
}

// static
EffectManifest EchoEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Echo"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr("Simple Echo.  Applies "
            "decay and runs a simple low-pass filter to reduce high "
            "frequencies"));

    EffectManifestParameter* time = manifest.addParameter();
    time->setId("delay_time");
    time->setName(QObject::tr("delay_time"));
    time->setDescription(QObject::tr("Delay time (seconds)"));
    time->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    time->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    time->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    time->setUnitsHint(EffectManifestParameter::UNITS_TIME);
    time->setMinimum(0.01);
    time->setDefault(0.25);
    time->setMaximum(2.0);

    time = manifest.addParameter();
    time->setId("decay_amount");
    time->setName(QObject::tr("decay_amount"));
    time->setDescription(QObject::tr("Decay amount"));
    time->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    time->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    time->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    time->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    time->setMinimum(0.00);
    time->setDefault(0.25);
    // Allow > 1.0 decay for DANGEROUS TESTING-ONLY feedback!
    time->setMaximum(1.2);

    // TODO(owilliams): Stereo pingpong parameter

    return manifest;
}

EchoEffect::EchoEffect(EngineEffect* pEffect, const EffectManifest& manifest)
        : m_pDelayParameter(pEffect->getParameterById("delay_time")),
          m_pDecayParameter(pEffect->getParameterById("decay_amount")) {
}

EchoEffect::~EchoEffect() {
    for (QMap<QString, GroupState*>::iterator it = m_groupState.begin();
            it != m_groupState.end();) {
        SampleUtil::free((*it)->delay_buf);
        SampleUtil::free((*it)->lowpass_buf);
        delete (*it)->decay_lowpass;
        delete it.value();
        it = m_groupState.erase(it);
    }
    qDebug() << debugString() << "destroyed";
}

int EchoEffect::getDelaySamples(double delay_time) const {
    int delay_samples = delay_time * 44100;
    if (delay_samples % 2 == 1) {
        --delay_samples;
    }
    if (delay_samples > MAX_BUFFER_LEN) {
        qWarning() << "Delay buffer requested is larger than max buffer!";
        delay_samples = MAX_BUFFER_LEN;
    }
    return delay_samples;
}

void EchoEffect::process(const QString& group, const CSAMPLE* pInput,
                          CSAMPLE* pOutput, const unsigned int numSamples) {
    GroupState* pGroupState = m_groupState.value(group, NULL);
    if (pGroupState == NULL) {
        pGroupState = new GroupState();
        m_groupState[group] = pGroupState;
    }
    GroupState& group_state = *pGroupState;

    double delay_time =
            m_pDelayParameter ? m_pDelayParameter->value().toDouble() : 1.0f;
    double decay_amount =
            m_pDecayParameter ? m_pDecayParameter->value().toDouble() : 0.25f;

    // TODO(owilliams): get actual sample rate from somewhere.

    int delay_samples = group_state.prev_delay_samples;

    if (delay_time < group_state.prev_delay_time) {
        // If the delay time has shrunk, we may need to wrap the write position.
        group_state.write_position = group_state.write_position % delay_samples;
        delay_samples = getDelaySamples(delay_time);
    } else if (delay_time > group_state.prev_delay_time) {
        // If the delay time has grown, we need to zero out the new portion
        // of the buffer we are using.
        SampleUtil::applyGain(
                group_state.delay_buf + group_state.prev_delay_samples,
                0,
                MAX_BUFFER_LEN - group_state.prev_delay_samples);
        delay_samples = getDelaySamples(delay_time);
    }

    int read_position = group_state.write_position;
    group_state.prev_delay_time = delay_time;
    group_state.prev_delay_samples = delay_samples;

    group_state.decay_lowpass->process(
            pInput, group_state.lowpass_buf, numSamples);

    for (unsigned int i = 0; i < numSamples; ++i) {
        group_state.delay_buf[group_state.write_position] *= decay_amount;
        group_state.delay_buf[group_state.write_position] += group_state.lowpass_buf[i];
        INCREMENT_RING(group_state.write_position, 1, delay_samples);
    }

    for (unsigned int i = 0; i < numSamples; ++i) {
        pOutput[i] = group_state.delay_buf[read_position];
        INCREMENT_RING(read_position, 1, delay_samples);
    }
}
