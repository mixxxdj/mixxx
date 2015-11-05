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
#include "engine/enginevinylsoundemu.h"
#include "engine/enginedeck.h"
#include "engine/enginepregain.h"
#include "engine/enginevumeter.h"
#include "engine/enginefilterbessel4.h"

#include "sampleutil.h"

EngineDeck::EngineDeck(QString group,
                       ConfigObject<ConfigValue>* pConfig,
                       EngineMaster* pMixingEngine,
                       EffectsManager* pEffectsManager,
                       EngineChannel::ChannelOrientation defaultOrientation)
        : EngineChannel(group, defaultOrientation),
          m_pConfig(pConfig),
          m_pEngineEffectsManager(pEffectsManager ? pEffectsManager->getEngineEffectsManager() : NULL),
          m_pPassing(new ControlPushButton(ConfigKey(group, "passthrough"))),
          // Need a +1 here because the CircularBuffer only allows its size-1
          // items to be held at once (it keeps a blank spot open persistently)
          m_sampleBuffer(NULL) {
    if (pEffectsManager != NULL) {
        pEffectsManager->registerGroup(getGroup());
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
    m_pPregain = new EnginePregain(group);
    m_pVUMeter = new EngineVuMeter(group);
    m_pBuffer = new EngineBuffer(group, pConfig, this, pMixingEngine);
    m_pVinylSoundEmu = new EngineVinylSoundEmu(group);
}

EngineDeck::~EngineDeck() {
    delete m_pPassing;

    delete m_pBuffer;
    delete m_pPregain;
    delete m_pVinylSoundEmu;
    delete m_pVUMeter;
}

void EngineDeck::process(CSAMPLE* pOut, const int iBufferSize) {
    GroupFeatureState features;
    // Feed the incoming audio through if passthrough is active
    const CSAMPLE* sampleBuffer = m_sampleBuffer; // save pointer on stack
    if (isPassthroughActive() && sampleBuffer) {
        SampleUtil::copy(pOut, sampleBuffer, iBufferSize);
        m_bPassthroughWasActive = true;
        m_sampleBuffer = NULL;
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
        // Emulate vinyl sounds
        m_pVinylSoundEmu->setSpeed(m_pBuffer->getSpeed());
        m_pVinylSoundEmu->process(pOut, iBufferSize);
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
                getGroup(), pOut, iBufferSize,
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
    if (m_bPassthroughWasActive && !m_bPassthroughIsActive) {
        return true;
    }

    return (m_pBuffer->isTrackLoaded() || isPassthroughActive());
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
