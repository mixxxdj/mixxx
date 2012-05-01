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
          m_pPassing(new ControlPushButton(ConfigKey(pGroup, "passthrough_passing"))),
          m_pConversionBuffer(SampleUtil::alloc(MAX_BUFFER_LEN)),
          // Need a +1 here because the CircularBuffer only allows its size-1
          // items to be held at once (it keeps a blank spot open persistently)
          m_sampleBuffer(MAX_BUFFER_LEN+1) {
    m_pPassing->setButtonMode(ControlPushButton::TOGGLE);
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

void EnginePassthrough::receiveBuffer(AudioInput input, const short* pBuffer, unsigned int nFrames) {

    if (input.getType() != AudioPath::EXTPASSTHROUGH) {
        // This is an error!
        qDebug() << "WARNING: EnginePassthrough receieved an AudioInput for a non-passthrough type!";
        return;
    }

    // Use the conversion buffer to both convert from short and double into
    // stereo.

    // Check that the number of mono samples doesn't exceed MAX_BUFFER_LEN/2
    // because thats our conversion buffer size.
    if (nFrames > MAX_BUFFER_LEN / 2) {
        qDebug() << "WARNING: Dropping passthrough samples because the input buffer is too large.";
        nFrames = MAX_BUFFER_LEN / 2;
    }

    // There isn't a suitable SampleUtil method that can do mono->stereo and
    // short->float in one pass.
    // SampleUtil::convert(m_pConversionBuffer, pBuffer, iNumSamples);
    SampleUtil::convert(m_pConversionBuffer, pBuffer, nFrames*2);

    // TODO(rryan) (or bkgood?) do we need to verify the input is the one we asked for? Oh well.
    unsigned int samplesWritten = m_sampleBuffer.write(m_pConversionBuffer, nFrames*2);
    if (samplesWritten < nFrames*2) {
        // Buffer overflow. We aren't processing samples fast enough. This
        // shouldn't happen since the deck spits out samples just as fast as they
        // come in, right?
        Q_ASSERT(false);
    }
}

void EnginePassthrough::process(const CSAMPLE* pInput, const CSAMPLE* pOutput, const int iBufferSize) {
    CSAMPLE* pOut = const_cast<CSAMPLE*>(pOutput);
    Q_UNUSED(pInput);

    // If passthrough is enabled, then read into the output buffer. Otherwise,
    // skip the appropriate number of samples to throw them away.
    if (m_pPassing->get() > 0.0f) {
        int samplesRead = m_sampleBuffer.read(pOut, iBufferSize);
        if (samplesRead < iBufferSize) {
            // Buffer underflow. There aren't getting samples fast enough. This
            // shouldn't happen since PortAudio should feed us samples just as fast
            // as we consume them, right?
            Q_ASSERT(false);
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
