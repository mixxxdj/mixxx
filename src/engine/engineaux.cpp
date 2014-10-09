// engineaux.cpp
// created 4/8/2011 by Bill Good (bkgood@gmail.com)
// shameless stolen from enginemicrophone.cpp (from RJ)

#include <QtDebug>

#include "engine/engineaux.h"

#include "configobject.h"
#include "sampleutil.h"
#include "effects/effectsmanager.h"
#include "engine/effects/engineeffectsmanager.h"
#include "controlaudiotaperpot.h"

EngineAux::EngineAux(const char* pGroup, EffectsManager* pEffectsManager)
        : EngineChannel(pGroup, EngineChannel::CENTER),
          m_pEngineEffectsManager(pEffectsManager ? pEffectsManager->getEngineEffectsManager() : NULL),
          m_vuMeter(pGroup),
          m_pEnabled(new ControlObject(ConfigKey(pGroup, "enabled"))),
          m_pPassing(new ControlPushButton(ConfigKey(pGroup, "passthrough"))),
          m_pPregain(new ControlAudioTaperPot(ConfigKey(pGroup, "pregain"), -12, 12, 0.5)),
          m_sampleBuffer(NULL),
          m_wasActive(false) {
    if (pEffectsManager != NULL) {
        pEffectsManager->registerGroup(getGroup());
    }
    m_pPassing->setButtonMode(ControlPushButton::POWERWINDOW);

    // Default passthrough to enabled on the master and disabled on PFL. User
    // can over-ride by setting the "pfl" or "master" controls.
    setMaster(true);
    setPFL(false);

    m_pSampleRate = new ControlObjectSlave("[Master]", "samplerate");
}

EngineAux::~EngineAux() {
    qDebug() << "~EngineAux()";
    delete m_pEnabled;
    delete m_pPassing;
    delete m_pPregain;
}

bool EngineAux::isActive() {
    bool enabled = m_pEnabled->get() > 0.0;
    if (enabled && m_sampleBuffer) {
        m_wasActive = true;
    } else if (m_wasActive) {
        m_vuMeter.reset();
        m_wasActive = false;
    }
    return m_wasActive;
}

void EngineAux::onInputConfigured(AudioInput input) {
    if (input.getType() != AudioPath::AUXILIARY) {
        // This is an error!
        qDebug() << "WARNING: EngineAux connected to AudioInput for a non-auxiliary type!";
        return;
    }
    m_sampleBuffer = NULL;
    m_pEnabled->set(1.0);
}

void EngineAux::onInputUnconfigured(AudioInput input) {
    if (input.getType() != AudioPath::AUXILIARY) {
        // This is an error!
        qDebug() << "WARNING: EngineAux connected to AudioInput for a non-auxiliary type!";
        return;
    }
    m_sampleBuffer = NULL;
    m_pEnabled->set(0.0);
}

void EngineAux::receiveBuffer(AudioInput input, const CSAMPLE* pBuffer,
                                      unsigned int nFrames) {
    Q_UNUSED(input);
    Q_UNUSED(nFrames);
    if (m_pPassing->get() <= 0.0) {
        m_sampleBuffer = NULL;
    } else {
        m_sampleBuffer = pBuffer;
    }
}

void EngineAux::process(CSAMPLE* pOut, const int iBufferSize) {
    const CSAMPLE* sampleBuffer = m_sampleBuffer; // save pointer on stack
    double pregain =  m_pPregain->get();
    if (sampleBuffer) {
        SampleUtil::copyWithGain(pOut, sampleBuffer, pregain, iBufferSize);
        m_sampleBuffer = NULL;
    } else {
        SampleUtil::clear(pOut, iBufferSize);
    }

    if (m_pEngineEffectsManager != NULL) {
        GroupFeatureState features;
        // This is out of date by a callback but some effects will want the RMS
        // volume.
        m_vuMeter.collectFeatures(&features);
        // Process effects enabled for this channel
        m_pEngineEffectsManager->process(getGroup(), pOut, iBufferSize,
                                         m_pSampleRate->get(), features);
    }
    // Update VU meter
    m_vuMeter.process(pOut, iBufferSize);
}
