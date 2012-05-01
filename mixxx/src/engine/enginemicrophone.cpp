// enginemicrophone.cpp
// created 3/16/2011 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "engine/enginemicrophone.h"

#include "configobject.h"
#include "sampleutil.h"

EngineMicrophone::EngineMicrophone(const char* pGroup)
        : EngineChannel(pGroup, EngineChannel::CENTER),
          m_clipping(pGroup),
          m_vuMeter(pGroup),
          m_pEnabled(new ControlObject(ConfigKey(pGroup, "enabled"))),
          m_pControlTalkover(new ControlPushButton(ConfigKey(pGroup, "talkover"))),
          m_pConversionBuffer(SampleUtil::alloc(MAX_BUFFER_LEN)),
          // Need a +1 here because the CircularBuffer only allows its size-1
          // items to be held at once (it keeps a blank spot open persistently)
          m_sampleBuffer(MAX_BUFFER_LEN+1) {
    m_pControlTalkover->setButtonMode(ControlPushButton::POWERWINDOW);
}

EngineMicrophone::~EngineMicrophone() {
    qDebug() << "~EngineMicrophone()";
    SampleUtil::free(m_pConversionBuffer);
    delete m_pEnabled;
    delete m_pControlTalkover;
}

bool EngineMicrophone::isActive() {
    bool enabled = m_pEnabled->get() > 0.0;
    return enabled && !m_sampleBuffer.isEmpty();
}

bool EngineMicrophone::isPFL() {
    // You normally don't expect to hear yourself in the headphones
    return false;
}

bool EngineMicrophone::isMaster() {
    return true;
}

void EngineMicrophone::onInputConnected(AudioInput input) {
    if (input.getType() != AudioPath::MICROPHONE ||
        AudioInput::channelsNeededForType(input.getType()) != 1) {
        // This is an error!
        qWarning() << "EngineMicrophone connected to AudioInput for a non-Microphone type or a non-mono buffer!";
        return;
    }
    m_sampleBuffer.clear();
    m_pEnabled->set(1.0f);
}

void EngineMicrophone::onInputDisconnected(AudioInput input) {
    if (input.getType() != AudioPath::MICROPHONE ||
        AudioInput::channelsNeededForType(input.getType()) != 1) {
        // This is an error!
        qWarning() << "EngineMicrophone connected to AudioInput for a non-Microphone type or a non-mono buffer!";
        return;
    }
    m_sampleBuffer.clear();
    m_pEnabled->set(0.0f);
}

void EngineMicrophone::receiveBuffer(AudioInput input, const short* pBuffer, unsigned int iNumSamples) {

    if (input.getType() != AudioPath::MICROPHONE ||
        AudioInput::channelsNeededForType(input.getType()) != 1) {
        // This is an error!
        qWarning() << "EngineMicrophone receieved an AudioInput for a non-Microphone type or a non-mono buffer!";
        return;
    }

    // Use the conversion buffer to both convert from short and double into
    // stereo.

    // Check that the number of mono samples doesn't exceed MAX_BUFFER_LEN/2
    // because thats our conversion buffer size.
    if (iNumSamples > MAX_BUFFER_LEN / 2) {
        qWarning() << "Dropping microphone samples because the input buffer is too large.";
        iNumSamples = MAX_BUFFER_LEN / 2;
    }

    // There isn't a suitable SampleUtil method that can do mono->stereo and
    // short->float in one pass.
    // SampleUtil::convert(m_pConversionBuffer, pBuffer, iNumSamples);
    for (unsigned int i = 0; i < iNumSamples; ++i) {
        m_pConversionBuffer[i*2 + 0] = pBuffer[i];
        m_pConversionBuffer[i*2 + 1] = pBuffer[i];
    }

    // m_pConversionBuffer is now stereo, so double the number of samples
    iNumSamples *= 2;

    // TODO(rryan) do we need to verify the input is the one we asked for? Oh well.
    unsigned int samplesWritten = m_sampleBuffer.write(m_pConversionBuffer, iNumSamples);
    if (samplesWritten < iNumSamples) {
        // Buffer overflow. We aren't processing samples fast enough. This
        // shouldn't happen since the mic spits out samples just as fast as they
        // come in, right?
        qWarning() << "Microphone buffer overflow";
    }
}

void EngineMicrophone::process(const CSAMPLE* pInput, const CSAMPLE* pOutput, const int iBufferSize) {
    Q_UNUSED(pInput);
    CSAMPLE* pOut = const_cast<CSAMPLE*>(pOutput);

    // If talkover is enabled, then read into the output buffer. Otherwise, skip
    // the appropriate number of samples to throw them away.
    if (m_pControlTalkover->get() > 0.0f) {
        int samplesRead = m_sampleBuffer.read(pOut, iBufferSize);
        if (samplesRead < iBufferSize) {
            // Buffer underflow. There aren't getting samples fast enough. This
            // shouldn't happen since PortAudio should feed us samples just as fast
            // as we consume them, right?
            qWarning() << "Microphone buffer underflow";
        }
    } else {
        SampleUtil::applyGain(pOut, 0.0, iBufferSize);
        m_sampleBuffer.skip(iBufferSize);
    }

    // Apply clipping
    m_clipping.process(pOut, pOut, iBufferSize);
    // Update VU meter
    m_vuMeter.process(pOut, pOut, iBufferSize);
}
