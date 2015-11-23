/***************************************************************************
                          enginedeck.cpp  -  description
                             -------------------
    begin                : Sun Apr 28 2002
    copyright            : (C) 2002 by
    email                :
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "controlpushbutton.h"
#include "effects/effectsmanager.h"
#include "engine/effects/engineeffectsmanager.h"
#include "engine/enginebuffer.h"
#include "engine/enginedeck.h"
#include "engine/enginepregain.h"
#include "engine/enginevumeter.h"
#include "engine/enginefilterbessel4.h"

#include "sampleutil.h"

EngineDeck::EngineDeck(const ChannelHandleAndGroup& handle_group,
                       ConfigObject<ConfigValue>* pConfig,
                       EngineMaster* pMixingEngine,
                       EffectsManager* pEffectsManager,
                       EngineChannel::ChannelOrientation defaultOrientation)
        : EngineChannel(handle_group, defaultOrientation),
          m_pConfig(pConfig),
          m_pEngineEffectsManager(pEffectsManager ? pEffectsManager->getEngineEffectsManager() : NULL),
          m_pPassing(new ControlPushButton(ConfigKey(getGroup(), "passthrough"))),
          // Need a +1 here because the CircularBuffer only allows its size-1
          // items to be held at once (it keeps a blank spot open persistently)
          m_sampleBuffer(NULL),
          m_wasActive(false) {
    if (pEffectsManager != NULL) {
        pEffectsManager->registerChannel(handle_group);
    }

    // Set up passthrough utilities and fields
    m_pPassing->setButtonMode(ControlPushButton::POWERWINDOW);
    m_bPassthroughIsActive = false;
    m_bPassthroughWasActive = false;

    // Set up passthrough toggle button
    connect(m_pPassing, SIGNAL(valueChanged(double)),
            this, SLOT(slotPassingToggle(double)),
            Qt::DirectConnection);

    m_pSampleRate = new ControlObjectSlave("[Master]", "samplerate");

    // Set up additional engines
    m_pPregain = new EnginePregain(getGroup());
    m_pVUMeter = new EngineVuMeter(getGroup());
    m_pBuffer = new EngineBuffer(getGroup(), pConfig, this, pMixingEngine);
}

EngineDeck::~EngineDeck() {
    delete m_pPassing;

    delete m_pBuffer;
    delete m_pPregain;
    delete m_pVUMeter;
    delete m_pSampleRate;
}

void EngineDeck::process(CSAMPLE* pOut, const int iBufferSize) {
    GroupFeatureState features;
    // Feed the incoming audio through if passthrough is active
    const CSAMPLE* sampleBuffer = m_sampleBuffer; // save pointer on stack
    if (isPassthroughActive() && sampleBuffer) {
        SampleUtil::copy(pOut, sampleBuffer, iBufferSize);
        m_bPassthroughWasActive = true;
        m_sampleBuffer = NULL;
        m_pPregain->setSpeed(1);
        m_pPregain->setScratching(false);
    } else {
        // If passthrough is no longer enabled, zero out the buffer
        if (m_bPassthroughWasActive) {
            SampleUtil::clear(pOut, iBufferSize);
            m_bPassthroughWasActive = false;
            return;
        }

        // Process the raw audio
        m_pBuffer->process(pOut, iBufferSize);
        m_pBuffer->collectFeatures(&features);
        m_pPregain->setSpeed(m_pBuffer->getSpeed());
        m_pPregain->setScratching(m_pBuffer->getScratching());
        m_bPassthroughWasActive = false;
    }

    // Apply pregain
    m_pPregain->process(pOut, iBufferSize);
    // Process effects enabled for this channel
    if (m_pEngineEffectsManager != NULL) {
        // This is out of date by a callback but some effects will want the RMS
        // volume.
        m_pVUMeter->collectFeatures(&features);
        m_pEngineEffectsManager->process(
                getHandle(), pOut, iBufferSize,
                static_cast<unsigned int>(m_pSampleRate->get()), features);
    }
    // Update VU meter
    m_pVUMeter->process(pOut, iBufferSize);
}

void EngineDeck::postProcess(const int iBufferSize) {
    m_pBuffer->postProcess(iBufferSize);
}

EngineBuffer* EngineDeck::getEngineBuffer() {
    return m_pBuffer;
}

bool EngineDeck::isActive() {
    bool active = false;
    if (m_bPassthroughWasActive && !m_bPassthroughIsActive) {
        active = true;
    } else {
        active = m_pBuffer->isTrackLoaded() || isPassthroughActive();
    }

    if (!active && m_wasActive) {
        m_pVUMeter->reset();
    }
    m_wasActive = active;
    return active;
}

void EngineDeck::receiveBuffer(AudioInput input, const CSAMPLE* pBuffer, unsigned int nFrames) {
    Q_UNUSED(input);
    Q_UNUSED(nFrames);
    // Skip receiving audio input if passthrough is not active
    if (!m_bPassthroughIsActive) {
        m_sampleBuffer = NULL;
        return;
    } else {
        m_sampleBuffer = pBuffer;
    }
}

void EngineDeck::onInputConfigured(AudioInput input) {
    if (input.getType() != AudioPath::VINYLCONTROL) {
        // This is an error!
        qDebug() << "WARNING: EngineDeck connected to AudioInput for a non-vinylcontrol type!";
        return;
    }
    m_sampleBuffer =  NULL;
}

void EngineDeck::onInputUnconfigured(AudioInput input) {
    if (input.getType() != AudioPath::VINYLCONTROL) {
        // This is an error!
        qDebug() << "WARNING: EngineDeck connected to AudioInput for a non-vinylcontrol type!";
        return;
    }
    m_sampleBuffer = NULL;
}

bool EngineDeck::isPassthroughActive() const {
    return (m_bPassthroughIsActive && m_sampleBuffer);
}

void EngineDeck::slotPassingToggle(double v) {
    m_bPassthroughIsActive = v > 0;
}
