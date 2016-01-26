// enginemicrophone.cpp
// created 3/16/2011 by RJ Ryan (rryan@mit.edu)

#include "engine/enginemicrophone.h"

#include <QtDebug>

#include "preferences/usersettings.h"
#include "control/control.h"
#include "controlaudiotaperpot.h"
#include "effects/effectsmanager.h"
#include "engine/effects/engineeffectsmanager.h"
#include "util/sample.h"

EngineMicrophone::EngineMicrophone(const ChannelHandleAndGroup& handle_group,
                                   EffectsManager* pEffectsManager)
        : EngineChannel(handle_group, EngineChannel::CENTER),
          m_pEngineEffectsManager(pEffectsManager ? pEffectsManager->getEngineEffectsManager() : NULL),
          m_vuMeter(getGroup()),
          m_pInputConfigured(new ControlObject(ConfigKey(getGroup(), "input_configured"))),
          m_pPregain(new ControlAudioTaperPot(ConfigKey(getGroup(), "pregain"), -12, 12, 0.5)),
          m_sampleBuffer(NULL),
          m_wasActive(false) {
    if (pEffectsManager != NULL) {
        pEffectsManager->registerChannel(handle_group);
    }

    // Make input_configured read-only.
    m_pInputConfigured->connectValueChangeRequest(
        this, SLOT(slotInputConfiguredChangeRequest(double)),
        Qt::DirectConnection);
    ControlDoublePrivate::insertAlias(ConfigKey(getGroup(), "enabled"),
                                      ConfigKey(getGroup(), "input_configured"));

    setMaster(false); // Use "talkover" button to enable microphones

    m_pSampleRate = new ControlObjectSlave("[Master]", "samplerate");
}

EngineMicrophone::~EngineMicrophone() {
    delete m_pSampleRate;
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
    m_pInputConfigured->setAndConfirm(1.0);
}

void EngineMicrophone::onInputUnconfigured(AudioInput input) {
    if (input.getType() != AudioPath::MICROPHONE) {
        // This is an error!
        qWarning() << "EngineMicrophone connected to AudioInput for a non-Microphone type!";
        return;
    }
    m_sampleBuffer = NULL;
    m_pInputConfigured->setAndConfirm(0.0);
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
    double pregain =  m_pPregain->get();
    if (sampleBuffer) {
        SampleUtil::copyWithGain(pOut, sampleBuffer, pregain, iBufferSize);
    } else {
        SampleUtil::clear(pOut, iBufferSize);
    }
    m_sampleBuffer = NULL;

    if (m_pEngineEffectsManager != NULL) {
        // Process effects enabled for this channel
        GroupFeatureState features;
        // This is out of date by a callback but some effects will want the RMS
        // volume.
        m_vuMeter.collectFeatures(&features);
        m_pEngineEffectsManager->process(getHandle(), pOut, iBufferSize,
                                         m_pSampleRate->get(), features);
    }
    // Update VU meter
    m_vuMeter.process(pOut, iBufferSize);
}
