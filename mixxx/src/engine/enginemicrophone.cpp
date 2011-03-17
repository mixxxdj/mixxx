// enginemicrophone.cpp
// created 3/16/2011 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "engine/enginemicrophone.h"

#include "configobject.h"
#include "sampleutil.h"

EngineMicrophone::EngineMicrophone(const char* pGroup)
        : EngineChannel(pGroup, EngineChannel::CENTER),
          m_volume(ConfigKey(pGroup, "volume")),
          m_clipping(pGroup),
          m_vuMeter(pGroup),
          m_pControlTalkover(new ControlPushButton(ConfigKey(pGroup, "talkover"))),
          m_pConversionBuffer(SampleUtil::alloc(MAX_BUFFER_LEN)),
          // Need a +1 here because the CircularBuffer only allows its size-1
          // items to be held at once (it keeps a blank spot open persistently)
          m_sampleBuffer(MAX_BUFFER_LEN+1) {
}

EngineMicrophone::~EngineMicrophone() {
    qDebug() << "~EngineMicrophone()";
    SampleUtil::free(m_pConversionBuffer);
}

bool EngineMicrophone::isActive() {
    return !m_sampleBuffer.isEmpty();
}

bool EngineMicrophone::isPFL() {
    return true;
}

bool EngineMicrophone::isMaster() {
    return true;
}

void EngineMicrophone::receiveBuffer(AudioInput input, const short* pBuffer, unsigned int iNumSamples) {
    // Use the conversion buffer to both convert from short and double into
    // stereo.

    // Check that the number of mono samples doesn't exceed MAX_BUFFER_LEN/2
    // because thats our conversion buffer size.
    if (iNumSamples > MAX_BUFFER_LEN / 2) {
        qDebug() << "WARNING: Dropping microphone samples because the input buffer is too large.";
        iNumSamples = MAX_BUFFER_LEN / 2;
    }

    // There isn't a suitable SampleUtil method that can do mono->stereo and
    // short->float in one pass.
    // SampleUtil::convert(m_pConversionBuffer, pBuffer, iNumSamples);
    for (int i = 0; i < iNumSamples; ++i) {
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
        Q_ASSERT(false);
    }
}

void EngineMicrophone::applyVolume(CSAMPLE *pBuff, const int iBufferSize) {
    m_volume.process(pBuff, pBuff, iBufferSize);
}

void EngineMicrophone::process(const CSAMPLE* pInput, const CSAMPLE* pOutput, const int iBufferSize) {
    CSAMPLE* pOut = const_cast<CSAMPLE*>(pOutput);


    // If talkover is enabled, then read into the output buffer. Otherwise, skip
    // the appropriate number of samples to throw them away.
    if (m_pControlTalkover->get() > 0.0f) {
        int samplesRead = m_sampleBuffer.read(pOut, iBufferSize);
        if (samplesRead < iBufferSize) {
            // Buffer underflow. There aren't getting samples fast enough. This
            // shouldn't happen since PortAudio should feed us samples just as fast
            // as we consume them, right?
            Q_ASSERT(false);
        }
    } else {
        m_sampleBuffer.skip(iBufferSize);
    }

    // Apply channel volume if we aren't PFL. TODO(rryan) this is a quirk of
    // EngineMaster, and we should keep the logic here just in case, even though
    // isPFL currently just returns true. In the future, the EngineMaster should
    // deal with volume.
    if (!isPFL()) {
        m_volume.process(pOut, pOut, iBufferSize);
    }
}
