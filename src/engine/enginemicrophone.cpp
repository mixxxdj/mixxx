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
          m_pConversionBuffer(SampleUtil::alloc(MAX_BUFFER_LEN)),
          // Need a +1 here because the CircularBuffer only allows its size-1
          // items to be held at once (it keeps a blank spot open persistently)
          m_sampleBuffer(MAX_BUFFER_LEN+1),
          m_wasActive(false) {
    // You normally don't expect to hear yourself in the headphones. Default PFL
    // setting for mic to false. User can over-ride by setting the "pfl" or
    // "master" controls.
    setMaster(true);
    setPFL(false);
}

EngineMicrophone::~EngineMicrophone() {
    qDebug() << "~EngineMicrophone()";
    SampleUtil::free(m_pConversionBuffer);
    delete m_pEnabled;
}

bool EngineMicrophone::isActive() {
    bool enabled = m_pEnabled->get() > 0.0;
    bool samplesAvailable = !m_sampleBuffer.isEmpty();
    if (enabled && samplesAvailable) {
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
    m_sampleBuffer.clear();
    m_pEnabled->set(1.0);
}

void EngineMicrophone::onInputUnconfigured(AudioInput input) {
    if (input.getType() != AudioPath::MICROPHONE) {
        // This is an error!
        qWarning() << "EngineMicrophone connected to AudioInput for a non-Microphone type!";
        return;
    }
    m_sampleBuffer.clear();
    m_pEnabled->set(0.0);
}

void EngineMicrophone::receiveBuffer(AudioInput input, const CSAMPLE* pBuffer,
                                     unsigned int nFrames) {
    if (!isTalkover()) {
        return;
    }

    if (input.getType() != AudioPath::MICROPHONE) {
        // This is an error!
        qWarning() << "EngineMicrophone receieved an AudioInput for a non-Microphone type!";
        return;
    }

    const unsigned int iChannels = input.getChannelGroup().getChannelCount();

    // Check that the number of mono frames doesn't exceed MAX_BUFFER_LEN/2
    // because thats our conversion buffer size.
    if (nFrames > MAX_BUFFER_LEN / iChannels) {
        qWarning() << "Dropping microphone samples because the input buffer is too large.";
        nFrames = MAX_BUFFER_LEN / iChannels;
    }

    const CSAMPLE* pWriteBuffer = NULL;
    unsigned int samplesToWrite = 0;

    if (iChannels == 1) {
        // Do mono -> stereo conversion.
        for (unsigned int i = 0; i < nFrames; ++i) {
            m_pConversionBuffer[i*2 + 0] = pBuffer[i];
            m_pConversionBuffer[i*2 + 1] = pBuffer[i];
        }
        pWriteBuffer = m_pConversionBuffer;
        samplesToWrite = nFrames * 2;
    } else if (iChannels == 2) {
        // Already in stereo. Use pBuffer as-is.
        pWriteBuffer = pBuffer;
        samplesToWrite = nFrames * iChannels;
    } else {
        qWarning() << "EngineMicrophone got greater than stereo input. Not currently handled.";
    }

    if (pWriteBuffer != NULL) {
        // TODO(rryan) do we need to verify the input is the one we asked for?
        // Oh well.
        unsigned int samplesWritten = m_sampleBuffer.write(pWriteBuffer,
                                                           samplesToWrite);
        if (samplesWritten < samplesToWrite) {
            // Buffer overflow. We aren't processing samples fast enough. This
            // shouldn't happen since the mic spits out samples just as fast as they
            // come in, right?
            qWarning() << "ERROR: Buffer overflow in EngineMicrophone. Dropping samples on the floor.";
        }
    }
}

void EngineMicrophone::process(const CSAMPLE* pInput, CSAMPLE* pOut, const int iBufferSize) {
    Q_UNUSED(pInput);

    // If talkover is enabled, then read into the output buffer. Otherwise, skip
    // the appropriate number of samples to throw them away.
    if (isTalkover()) {
        int samplesRead = m_sampleBuffer.read(pOut, iBufferSize);
        if (samplesRead < iBufferSize) {
            // Buffer underflow. There aren't getting samples fast enough. This
            // shouldn't happen since PortAudio should feed us samples just as fast
            // as we consume them, right?
            qWarning() << "ERROR: Buffer underflow in EngineMicrophone. Playing silence.";
            SampleUtil::clear(pOut + samplesRead, iBufferSize - samplesRead);
        }
    } else {
        SampleUtil::clear(pOut, iBufferSize);
        m_sampleBuffer.skip(iBufferSize);
    }

    // Apply clipping
    m_clipping.process(pOut, pOut, iBufferSize);
    // Update VU meter
    m_vuMeter.process(pOut, pOut, iBufferSize);
}
