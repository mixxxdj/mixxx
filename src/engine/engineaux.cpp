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

EngineAux::EngineAux(const ChannelHandleAndGroup& handle_group, EffectsManager* pEffectsManager)
        : EngineChannel(handle_group, EngineChannel::CENTER),
          m_pEngineEffectsManager(pEffectsManager ? pEffectsManager->getEngineEffectsManager() : NULL),
          m_vuMeter(getGroup()),
          m_pEnabled(new ControlObject(ConfigKey(getGroup(), "enabled"))),
          m_pPregain(new ControlAudioTaperPot(ConfigKey(getGroup(), "pregain"), -12, 12, 0.5)),
          m_sampleBuffer(NULL),
          m_wasActive(false) {
    if (pEffectsManager != NULL) {
        pEffectsManager->registerChannel(handle_group);
    }

    // by default Aux is enabled on the master and disabled on PFL. User
    // can over-ride by setting the "pfl" or "master" controls.
    setMaster(true);

    m_pSampleRate = new ControlObjectSlave("[Master]", "samplerate");
}

EngineAux::~EngineAux() {
    qDebug() << "~EngineAux()";
    delete m_pEnabled;
    delete m_pPregain;
    delete m_pSampleRate;
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
    m_sampleBuffer = pBuffer;
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
        m_pEngineEffectsManager->process(getHandle(), pOut, iBufferSize,
                                         m_pSampleRate->get(), features);
    }
    // Update VU meter
    m_vuMeter.process(pOut, iBufferSize);
}
