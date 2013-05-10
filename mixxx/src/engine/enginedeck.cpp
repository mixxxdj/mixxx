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
#include "enginebuffer.h"
#include "enginevinylsoundemu.h"
#include "enginedeck.h"
#include "engineclipping.h"
#include "enginepregain.h"
#include "engineflanger.h"
#include "enginefilterblock.h"
#include "enginevumeter.h"
#include "enginefilteriir.h"

#include "sampleutil.h"

EngineDeck::EngineDeck(const char* group,
                             ConfigObject<ConfigValue>* pConfig,
                             EngineChannel::ChannelOrientation defaultOrientation)
        : EngineChannel(group, defaultOrientation),
          m_pConfig(pConfig),
          m_pPassing(new ControlPushButton(ConfigKey(group, "passthrough_enabled"))),
          // Need a +1 here because the CircularBuffer only allows its size-1
          // items to be held at once (it keeps a blank spot open persistently)
          m_sampleBuffer(MAX_BUFFER_LEN+1) {

    // Set up passthrough utilities and fields
    m_pPassing->setButtonMode(ControlPushButton::TOGGLE);
    m_pConversionBuffer = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_bPassthroughIsActive = false;
    m_bPassthroughWasActive = false;

    // Set up passthrough toggle button
    connect(m_pPassing, SIGNAL(valueChanged(double)),
            this, SLOT(slotPassingToggle(double)),
            Qt::DirectConnection);

    // Set up additional engines
    m_pPregain = new EnginePregain(group);
    m_pFilter = new EngineFilterBlock(group);
    m_pFlanger = new EngineFlanger(group);
    m_pClipping = new EngineClipping(group);
    m_pBuffer = new EngineBuffer(group, pConfig);
    m_pVinylSoundEmu = new EngineVinylSoundEmu(pConfig, group);
    m_pVUMeter = new EngineVuMeter(group);
}

EngineDeck::~EngineDeck() {
    SampleUtil::free(m_pConversionBuffer);
    delete m_pPassing;

    delete m_pBuffer;
    delete m_pClipping;
    delete m_pFilter;
    delete m_pFlanger;
    delete m_pPregain;
    delete m_pVinylSoundEmu;
    delete m_pVUMeter;
}

void EngineDeck::process(const CSAMPLE*, const CSAMPLE * pOutput, const int iBufferSize) {
    CSAMPLE* pOut = const_cast<CSAMPLE*>(pOutput);

    // Feed the incoming audio through if passthrough is active
    if (isPassthroughActive()) {
        int samplesRead = m_sampleBuffer.read(pOut, iBufferSize);
        if (samplesRead < iBufferSize) {
            // Buffer underflow. There aren't getting samples fast enough. This
            // shouldn't happen since PortAudio should feed us samples just as fast
            // as we consume them, right?
            qWarning() << "ERROR: Buffer overflow in EngineDeck. Playing silence.";
            SampleUtil::applyGain(pOut + samplesRead, 0.0, iBufferSize - samplesRead);
        }
        m_bPassthroughWasActive = true;
    } else {
        // If passthrough is no longer enabled, zero out the buffer
        if (m_bPassthroughWasActive) {
            SampleUtil::applyGain(pOut, 0.0, iBufferSize);
            m_sampleBuffer.skip(iBufferSize);
            m_bPassthroughWasActive = false;
            return;
        }

        // Process the raw audio
        m_pBuffer->process(0, pOut, iBufferSize);
        // Emulate vinyl sounds
        m_pVinylSoundEmu->process(pOut, pOut, iBufferSize);
        m_bPassthroughWasActive = false;
    }

    // Apply pregain
    m_pPregain->process(pOut, pOut, iBufferSize);
    // Filter the channel with EQs
    m_pFilter->process(pOut, pOut, iBufferSize);
    // TODO(XXX) LADSPA
    m_pFlanger->process(pOut, pOut, iBufferSize);
    // Apply clipping
    m_pClipping->process(pOut, pOut, iBufferSize);
    // Update VU meter
    m_pVUMeter->process(pOut, pOut, iBufferSize);
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

void EngineDeck::receiveBuffer(AudioInput input, const short* pBuffer, unsigned int nFrames) {
    // Skip receiving audio input if passthrough is not active
    if (!m_bPassthroughIsActive) {
        return;
    }

    if (input.getType() != AudioPath::VINYLCONTROL) {
        // This is an error!
        qDebug() << "WARNING: EngineDeck receieved an AudioInput for a non-vinylcontrol type!";
        return;
    }

    const unsigned int iChannels = AudioInput::channelsNeededForType(input.getType());

    // Check that the number of mono samples doesn't exceed MAX_BUFFER_LEN/2
    // because thats our conversion buffer size.
    if (nFrames > MAX_BUFFER_LEN / iChannels) {
        qWarning() << "WARNING: Dropping passthrough samples because the input buffer is too large.";
        nFrames = MAX_BUFFER_LEN / iChannels;
    }

    if (iChannels == 1) {
        // Do mono -> stereo conversion. There isn't a suitable SampleUtil
        // method that can do mono->stereo and short->float in one pass.
        for (unsigned int i = 0; i < nFrames; ++i) {
            m_pConversionBuffer[i*2 + 0] = pBuffer[i];
            m_pConversionBuffer[i*2 + 1] = pBuffer[i];
        }
    } else if (iChannels == 2) {
        // Use the conversion buffer to both convert from short and double.
        SampleUtil::convert(m_pConversionBuffer, pBuffer, nFrames*iChannels);
    } else {
        qWarning() << "EnginePassthrough got greater than stereo input. Not currently handled.";
    }

    const int samplesToWrite = nFrames * iChannels;

    // TODO(rryan) do we need to verify the input is the one we asked for? Oh well.
    unsigned int samplesWritten = m_sampleBuffer.write(m_pConversionBuffer, samplesToWrite);
    if (samplesWritten < samplesToWrite) {
        // Buffer overflow. We aren't processing samples fast enough. This
        // shouldn't happen since the deck spits out samples just as fast as they
        // come in, right?
        qWarning() << "ERROR: Buffer overflow in EngineMicrophone. Dropping samples on the floor.";
    }
}

void EngineDeck::onInputConnected(AudioInput input) {
    if (input.getType() != AudioPath::VINYLCONTROL) {
        // This is an error!
        qDebug() << "WARNING: EngineDeck connected to AudioInput for a non-vinylcontrol type!";
        return;
    }
    m_sampleBuffer.clear();
}

void EngineDeck::onInputDisconnected(AudioInput input) {
    if (input.getType() != AudioPath::VINYLCONTROL) {
        // This is an error!
        qDebug() << "WARNING: EngineDeck connected to AudioInput for a non-vinylcontrol type!";
        return;
    }
    m_sampleBuffer.clear();
}

bool EngineDeck::isPassthroughActive() {
    return (m_bPassthroughIsActive && !m_sampleBuffer.isEmpty());
}

void EngineDeck::slotPassingToggle(double v) {
    m_bPassthroughIsActive = v > 0;
}

