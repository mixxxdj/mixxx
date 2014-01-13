// enginepassthrough.cpp
// created 4/8/2011 by Bill Good (bkgood@gmail.com)
// shameless stolen from enginemicrophone.cpp (from RJ)

#include <QtDebug>

#include "engine/enginepassthrough.h"

#include "configobject.h"
#include "sampleutil.h"

EnginePassthrough::EnginePassthrough(const char* pGroup)
        : EngineChannel(pGroup, EngineChannel::CENTER),
          m_clipping(pGroup),
          m_vuMeter(pGroup),
          m_pEnabled(new ControlObject(ConfigKey(pGroup, "passthrough_enabled"))),
          m_pPassing(new ControlPushButton(ConfigKey(pGroup, "passthrough"))),
          m_pConversionBuffer(SampleUtil::alloc(MAX_BUFFER_LEN)),
          // Need a +1 here because the CircularBuffer only allows its size-1
          // items to be held at once (it keeps a blank spot open persistently)
          m_sampleBuffer(MAX_BUFFER_LEN+1) {
    m_pPassing->setButtonMode(ControlPushButton::POWERWINDOW);
}

EnginePassthrough::~EnginePassthrough() {
    qDebug() << "~EnginePassthrough()";
    SampleUtil::free(m_pConversionBuffer);
    delete m_pEnabled;
    delete m_pPassing;
}

bool EnginePassthrough::isActive() {
    bool enabled = m_pEnabled->get() > 0.0;
    return enabled && !m_sampleBuffer.isEmpty();
}

bool EnginePassthrough::isPFL() {
    return true;
}

bool EnginePassthrough::isMaster() {
    return true;
}

void EnginePassthrough::onInputConnected(AudioInput input) {
    if (input.getType() != AudioPath::EXTPASSTHROUGH) {
        // This is an error!
        qDebug() << "WARNING: EnginePassthrough connected to AudioInput for a non-passthrough type!";
        return;
    }
    m_sampleBuffer.clear();
    m_pEnabled->set(1.0f);
}

void EnginePassthrough::onInputDisconnected(AudioInput input) {
    if (input.getType() != AudioPath::EXTPASSTHROUGH) {
        // This is an error!
        qDebug() << "WARNING: EnginePassthrough connected to AudioInput for a non-passthrough type!";
        return;
    }
    m_sampleBuffer.clear();
    m_pEnabled->set(0.0f);
}

void EnginePassthrough::receiveBuffer(AudioInput input, const CSAMPLE* pBuffer,
                                      unsigned int nFrames) {
    if (m_pPassing->get() <= 0.0) {
        return;
    }

    if (input.getType() != AudioPath::EXTPASSTHROUGH) {
        // This is an error!
        qDebug() << "WARNING: EnginePassthrough receieved an AudioInput for a non-passthrough type!";
        return;
    }

    const unsigned int iChannels = input.getChannelGroup().getChannelCount();

    // Check that the number of mono frames doesn't exceed MAX_BUFFER_LEN/2
    // because thats our conversion buffer size.
    if (nFrames > MAX_BUFFER_LEN / iChannels) {
        qDebug() << "WARNING: Dropping passthrough samples because the input buffer is too large.";
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
        qWarning() << "EnginePassthrough got greater than stereo input. Not currently handled.";
    }


    if (pWriteBuffer != NULL) {
        // TODO(rryan) do we need to verify the input is the one we asked for?
        // Oh well.
        unsigned int samplesWritten = m_sampleBuffer.write(pWriteBuffer,
                                                           samplesToWrite);
        if (samplesWritten < samplesToWrite) {
            // Buffer overflow. We aren't processing samples fast enough. This
            // shouldn't happen since the deck spits out samples just as fast as they
            // come in, right?
            qWarning() << "ERROR: Buffer overflow in EnginePassthrough. Dropping samples on the floor.";
        }
    }
}

void EnginePassthrough::process(const CSAMPLE* pInput, CSAMPLE* pOut, const int iBufferSize) {
    Q_UNUSED(pInput);

    // If passthrough is enabled, then read into the output buffer. Otherwise,
    // skip the appropriate number of samples to throw them away.
    if (m_pPassing->get() > 0.0f) {
        int samplesRead = m_sampleBuffer.read(pOut, iBufferSize);
        if (samplesRead < iBufferSize) {
            // Buffer underflow. There aren't getting samples fast enough. This
            // shouldn't happen since PortAudio should feed us samples just as fast
            // as we consume them, right?
            qWarning() << "ERROR: Buffer underflow in EnginePassthrough. Playing silence.";
            SampleUtil::applyGain(pOut + samplesRead, 0.0, iBufferSize - samplesRead);
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
