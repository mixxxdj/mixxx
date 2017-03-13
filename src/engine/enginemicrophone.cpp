// enginemicrophone.cpp
// created 3/16/2011 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "engine/enginemicrophone.h"

#include "configobject.h"
#include "sampleutil.h"
#include "effects/effectsmanager.h"
#include "engine/effects/engineeffectsmanager.h"
#include "controlaudiotaperpot.h"


EngineMicrophone::EngineMicrophone(QString pGroup, EffectsManager* pEffectsManager)
        : EngineChannel(pGroup, EngineChannel::CENTER),
          m_pEngineEffectsManager(pEffectsManager ? pEffectsManager->getEngineEffectsManager() : NULL),
          m_vuMeter(pGroup),
          m_pEnabled(new ControlObject(ConfigKey(pGroup, "enabled"))),
          m_pPregain(new ControlAudioTaperPot(ConfigKey(pGroup, "pregain"), -12, 12, 0.5)),
          m_sampleBuffer(NULL),
          m_wasActive(false) {
    if (pEffectsManager != NULL) {
        pEffectsManager->registerGroup(getGroup());
    }

    // You normally don't expect to hear yourself in the headphones. Default PFL
    // setting for mic to false. User can over-ride by setting the "pfl" or
    // "master" controls.
    setMaster(true);
    setPFL(false);

    m_pSampleRate = new ControlObjectSlave("[Master]", "samplerate");

}

EngineMicrophone::~EngineMicrophone() {
    qDebug() << "~EngineMicrophone()";
    delete m_pSampleRate;
    delete m_pEnabled;
    delete m_pPregain;
}

bool EngineMicrophone::isActive() {
    bool enabled = m_pEnabled->get() > 0.0;
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
    m_pEnabled->set(1.0);
}

void EngineMicrophone::onInputUnconfigured(AudioInput input) {
    if (input.getType() != AudioPath::MICROPHONE) {
        // This is an error!
        qWarning() << "EngineMicrophone connected to AudioInput for a non-Microphone type!";
        return;
    }
    m_sampleBuffer = NULL;
    m_pEnabled->set(0.0);
}

void EngineMicrophone::receiveBuffer(AudioInput input, const CSAMPLE* pBuffer,
                                     unsigned int nFrames) {
    Q_UNUSED(input);
    Q_UNUSED(nFrames);
    if (!isTalkover()) {
        m_sampleBuffer = NULL;
        return;
    } else {
        m_sampleBuffer = pBuffer;
    }
}

void EngineMicrophone::process(CSAMPLE* pOut, const int iBufferSize) {

    // If talkover is enabled, then read into the output buffer. Otherwise, skip
    // the appropriate number of samples to throw them away.
    const CSAMPLE* sampleBuffer = m_sampleBuffer; // save pointer on stack
    double pregain =  m_pPregain->get();
    if (isTalkover() && sampleBuffer) {
        SampleUtil::copyWithGain(pOut, sampleBuffer, pregain, iBufferSize);
        m_sampleBuffer = NULL;
    } else {
        SampleUtil::clear(pOut, iBufferSize);
    }


    if (m_pEngineEffectsManager != NULL) {
        // Process effects enabled for this channel
        GroupFeatureState features;
        // This is out of date by a callback but some effects will want the RMS
        // volume.
        m_vuMeter.collectFeatures(&features);
        m_pEngineEffectsManager->process(getGroup(), pOut, iBufferSize,
                                         m_pSampleRate->get(), features);
    }
    // Update VU meter
    m_vuMeter.process(pOut, iBufferSize);
}
