// engineaux.cpp
// created 4/8/2011 by Bill Good (bkgood@gmail.com)
// shameless stolen from enginemicrophone.cpp (from RJ)

#include <QtDebug>

#include "engine/engineaux.h"

#include "configobject.h"
#include "sampleutil.h"

EngineAux::EngineAux(const char* pGroup)
        : EngineChannel(pGroup, EngineChannel::CENTER),
          m_clipping(pGroup),
          m_vuMeter(pGroup),
          m_pConfigured(new ControlObject(ConfigKey(pGroup, "configured"))),
          m_pPassing(new ControlPushButton(ConfigKey(pGroup, "passthrough"))),
          m_pConversionBuffer(SampleUtil::alloc(MAX_BUFFER_LEN)),
          // Need a +1 here because the CircularBuffer only allows its size-1
          // items to be held at once (it keeps a blank spot open persistently)
          m_sampleBuffer(MAX_BUFFER_LEN + 1),
          m_wasActive(false) {
    m_pPassing->setButtonMode(ControlPushButton::POWERWINDOW);

    // Default passthrough to enabled on the master and disabled on PFL. User
    // can over-ride by setting the "pfl" or "master" controls.
    setMaster(true);
    setPFL(false);
}

EngineAux::~EngineAux() {
    qDebug() << "~EngineAux()";
    SampleUtil::free(m_pConversionBuffer);
    delete m_pConfigured;
    delete m_pPassing;
}

bool EngineAux::isActive() {
    bool configured = m_pConfigured->get() > 0.0;
    bool samplesAvailable = !m_sampleBuffer.isEmpty();
    if (configured && samplesAvailable) {
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
        qDebug() << "WARNING: EngineAux connected to AudioInput for a non-passthrough type!";
        return;
    }
    m_sampleBuffer.clear();
    m_pConfigured->set(1.0);
}

void EngineAux::onInputUnconfigured(AudioInput input) {
    if (input.getType() != AudioPath::AUXILIARY) {
        // This is an error!
        qDebug() << "WARNING: EngineAux connected to AudioInput for a non-passthrough type!";
        return;
    }
    m_sampleBuffer.clear();
    m_pConfigured->set(0.0);
}

void EngineAux::receiveBuffer(AudioInput input, const CSAMPLE* pBuffer,
                                      unsigned int nFrames) {
    if (m_pPassing->get() <= 0.0) {
        return;
    }

    if (input.getType() != AudioPath::AUXILIARY) {
        // This is an error!
        qDebug() << "WARNING: EngineAux received an AudioInput for a non-auxiliary type!";
        return;
    }

    const unsigned int iChannels = input.getChannelGroup().getChannelCount();

    // Check that the number of mono frames doesn't exceed MAX_BUFFER_LEN/2
    // because thats our conversion buffer size.
    if (nFrames > MAX_BUFFER_LEN / iChannels) {
        qDebug() << "WARNING: Dropping aux samples because the input buffer is too large.";
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
        qWarning() << "EngineAux got greater than stereo input. Not currently handled.";
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
            qWarning() << "ERROR: Buffer overflow in EngineAux. Dropping samples on the floor.";
        }
    }
}

void EngineAux::process(const CSAMPLE* pInput, CSAMPLE* pOut, const int iBufferSize) {
    Q_UNUSED(pInput);

    int samplesRead = m_sampleBuffer.read(pOut, iBufferSize);
    if (samplesRead < iBufferSize) {
        // Buffer underflow. There aren't getting samples fast enough. This
        // shouldn't happen since PortAudio should feed us samples just as fast
        // as we consume them, right?
        qWarning() << "ERROR: Buffer underflow in EngineAux. Playing silence.";
        SampleUtil::clear(pOut + samplesRead, iBufferSize - samplesRead);
    }
    // Apply clipping
    m_clipping.process(pOut, pOut, iBufferSize);
    // Update VU meter
    m_vuMeter.process(pOut, pOut, iBufferSize);
}
