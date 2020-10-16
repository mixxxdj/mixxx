// enginemicrophone.cpp
// created 3/16/2011 by RJ Ryan (rryan@mit.edu)

#include "engine/channels/enginemicrophone.h"

#include <QtDebug>

#include "preferences/usersettings.h"
#include "control/control.h"
#include "control/controlaudiotaperpot.h"
#include "effects/effectsmanager.h"
#include "engine/effects/engineeffectsmanager.h"
#include "util/sample.h"

EngineMicrophone::EngineMicrophone(const ChannelHandleAndGroup& handle_group,
        EffectsManager* pEffectsManager)
        : EngineChannel(handle_group, EngineChannel::CENTER, pEffectsManager,
                  /*isTalkoverChannel*/ true,
                  /*isPrimaryDeck*/ false),
          m_pInputConfigured(new ControlObject(ConfigKey(getGroup(), "input_configured"))),
          m_pPregain(new ControlAudioTaperPot(ConfigKey(getGroup(), "pregain"), -12, 12, 0.5)),
          m_wasActive(false) {
    // Make input_configured read-only.
    m_pInputConfigured->setReadOnly();
    ControlDoublePrivate::insertAlias(ConfigKey(getGroup(), "enabled"),
                                      ConfigKey(getGroup(), "input_configured"));

    setMaster(false); // Use "talkover" button to enable microphones
}

EngineMicrophone::~EngineMicrophone() {
    delete m_pPregain;
}

bool EngineMicrophone::isActive() {
    bool enabled = m_pInputConfigured->toBool();
    if (enabled && m_sampleBuffer) {
        m_wasActive = true;
    } else if (m_wasActive) {
        m_vuMeter.reset();
        m_wasActive = false;
    }
    return m_wasActive;
}

void EngineMicrophone::onInputConfigured(AudioInput input) {
    if (input.getType() != AudioPath::MICROPHONE) {
        // This is an error!
        qWarning() << "EngineMicrophone connected to AudioInput for a non-Microphone type!";
        return;
    }
    m_sampleBuffer = NULL;
    m_pInputConfigured->forceSet(1.0);
}

void EngineMicrophone::onInputUnconfigured(AudioInput input) {
    if (input.getType() != AudioPath::MICROPHONE) {
        // This is an error!
        qWarning() << "EngineMicrophone connected to AudioInput for a non-Microphone type!";
        return;
    }
    m_sampleBuffer = NULL;
    m_pInputConfigured->forceSet(0.0);
}

void EngineMicrophone::receiveBuffer(AudioInput input, const CSAMPLE* pBuffer,
                                     unsigned int nFrames) {
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
            pEngineEffectsManager->processPreFaderInPlace(
                    m_group.handle(), m_pEffectsManager->getMasterHandle(), pOut, iBufferSize,
                    // TODO(jholthuis): Use mixxx::audio::SampleRate instead
                    static_cast<unsigned int>(m_pSampleRate->get()));
        }
    } else {
        SampleUtil::clear(pOut, iBufferSize);
    }
    m_sampleBuffer = NULL;

    // Update VU meter
    m_vuMeter.process(pOut, iBufferSize);
}

void EngineMicrophone::collectFeatures(GroupFeatureState* pGroupFeatures) const {
    m_vuMeter.collectFeatures(pGroupFeatures);
}
