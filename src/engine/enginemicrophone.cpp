// enginemicrophone.cpp
// created 3/16/2011 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "engine/enginemicrophone.h"

#include "configobject.h"
#include "sampleutil.h"
#include "effects/effectsmanager.h"
#include "engine/effects/engineeffectsmanager.h"

EngineMicrophone::EngineMicrophone(const char* pGroup, EffectsManager* pEffectsManager)
        : EngineChannel(pGroup, EngineChannel::CENTER),
          m_pEngineEffectsManager(pEffectsManager ? pEffectsManager->getEngineEffectsManager() : NULL),
          m_clipping(pGroup),
          m_vuMeter(pGroup),
          m_pEnabled(new ControlObject(ConfigKey(pGroup, "enabled"))),
          // Need a +1 here because the CircularBuffer only allows its size-1
          // items to be held at once (it keeps a blank spot open persistently)
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
}

EngineMicrophone::~EngineMicrophone() {
    qDebug() << "~EngineMicrophone()";
    delete m_pEnabled;
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

void EngineMicrophone::process(const CSAMPLE* pInput, CSAMPLE* pOut, const int iBufferSize) {
    Q_UNUSED(pInput);

    // If talkover is enabled, then read into the output buffer. Otherwise, skip
    // the appropriate number of samples to throw them away.
    const CSAMPLE* sampleBuffer = m_sampleBuffer; // save pointer on stack
    qDebug() << "EngineMicrophone::process()" << sampleBuffer << sampleBuffer[0]  << sampleBuffer[1];
    if (isTalkover() && sampleBuffer) {
        memcpy(pOut, sampleBuffer, iBufferSize * sizeof(pOut[0]));
        m_sampleBuffer = NULL;
    } else {
        SampleUtil::clear(pOut, iBufferSize);
    }

    if (m_pEngineEffectsManager != NULL) {
        // Process effects enabled for this channel
        m_pEngineEffectsManager->process(getGroup(), pOut, pOut, iBufferSize);
    }
    // Apply clipping
    m_clipping.process(pOut, pOut, iBufferSize);
    // Update VU meter
    m_vuMeter.process(pOut, pOut, iBufferSize);
}
