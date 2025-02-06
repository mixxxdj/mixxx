#include "engine/channels/enginemicrophone.h"

#include <QtDebug>

#include "audio/types.h"
#include "control/controlaudiotaperpot.h"
#include "effects/effectsmanager.h"
#include "engine/effects/engineeffectsmanager.h"
#include "moc_enginemicrophone.cpp"
#include "util/sample.h"

EngineMicrophone::EngineMicrophone(const ChannelHandleAndGroup& handleGroup,
        EffectsManager* pEffectsManager)
        : EngineChannel(handleGroup, EngineChannel::CENTER, pEffectsManager,
                  /*isTalkoverChannel*/ true,
                  /*isPrimaryDeck*/ false),
          m_pInputConfigured(new ControlObject(ConfigKey(getGroup(), "input_configured"))),
          m_pPregain(new ControlAudioTaperPot(ConfigKey(getGroup(), "pregain"), -12, 12, 0.5)) {
    // Make input_configured read-only.
    m_pInputConfigured->setReadOnly();
    m_pInputConfigured->addAlias(ConfigKey(getGroup(), QStringLiteral("enabled")));

    setMainMix(false); // Use "talkover" button to enable microphones
}

EngineMicrophone::~EngineMicrophone() {
    delete m_pPregain;
}

EngineChannel::ActiveState EngineMicrophone::updateActiveState() {
    bool enabled = m_pInputConfigured->toBool();
    if (enabled && m_sampleBuffer) {
        m_active = true;
        return ActiveState::Active;
    }
    if (m_active) {
        m_vuMeter.reset();
        m_active = false;
        return ActiveState::WasActive;
    }
    return ActiveState::Inactive;
}

void EngineMicrophone::onInputConfigured(const AudioInput& input) {
    if (input.getType() != AudioPathType::Microphone) {
        // This is an error!
        qWarning() << "EngineMicrophone connected to AudioInput for a non-Microphone type!";
        return;
    }
    m_sampleBuffer = nullptr;
    m_pInputConfigured->forceSet(1.0);
}

void EngineMicrophone::onInputUnconfigured(const AudioInput& input) {
    if (input.getType() != AudioPathType::Microphone) {
        // This is an error!
        qWarning() << "EngineMicrophone connected to AudioInput for a non-Microphone type!";
        return;
    }
    m_sampleBuffer = nullptr;
    m_pInputConfigured->forceSet(0.0);
}

void EngineMicrophone::receiveBuffer(
        const AudioInput& input, const CSAMPLE* pBuffer, unsigned int nFrames) {
    Q_UNUSED(input);
    Q_UNUSED(nFrames);
    m_sampleBuffer = pBuffer;
}

void EngineMicrophone::process(CSAMPLE* pOut, const int iBufferSize) {
    // If configured read into the output buffer.
    // Otherwise, skip the appropriate number of samples to throw them away.
    const CSAMPLE* sampleBuffer = m_sampleBuffer; // save pointer on stack
    CSAMPLE_GAIN pregain = static_cast<CSAMPLE_GAIN>(m_pPregain->get());
    if (sampleBuffer) {
        SampleUtil::copyWithGain(pOut, sampleBuffer, pregain, iBufferSize);
        EngineEffectsManager* pEngineEffectsManager = m_pEffectsManager->getEngineEffectsManager();
        if (pEngineEffectsManager != nullptr) {
            pEngineEffectsManager->processPreFaderInPlace(m_group.handle(),
                    m_pEffectsManager->getMainHandle(),
                    pOut,
                    iBufferSize,
                    mixxx::audio::SampleRate::fromDouble(m_sampleRate.get()));
        }
    } else {
        SampleUtil::clear(pOut, iBufferSize);
    }
    m_sampleBuffer = nullptr;

    // Update VU meter
    m_vuMeter.process(pOut, iBufferSize);
}

void EngineMicrophone::collectFeatures(GroupFeatureState* pGroupFeatures) const {
    m_vuMeter.collectFeatures(pGroupFeatures);
}
