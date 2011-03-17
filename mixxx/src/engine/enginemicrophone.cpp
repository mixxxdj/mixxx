// enginemicrophone.cpp
// created 3/16/2011 by RJ Ryan (rryan@mit.edu)

#include "engine/enginemicrophone.h"

#include "configobject.h"
#include "sampleutil.h"

EngineMicrophone::EngineMicrophone(const char* pGroup)
        : EngineChannel(pGroup, EngineChannel::CENTER),
          m_volume(ConfigKey(pGroup, "volume")),
          m_clipping(pGroup),
          m_vuMeter(pGroup),
          m_sampleBuffer(MAX_BUFFER_LEN) {
}

EngineMicrophone::~EngineMicrophone() {
}

bool EngineMicrophone::isActive() const {
    return !m_sampleBuffer.isEmpty();
}

bool EngineMicrophone::isPFL() const {
    return true;
}

bool EngineMicrophone::isMaster() const {
    return true;
}

void EngineMicrophone::receiveBuffer(AudioInput input, const CSAMPLE* pBuffer, unsigned int iNumSamples) {
    // TODO(rryan) do we need to verify the input is the one we asked for? Oh well.
    int samplesWritten = m_sampleBuffer.write(pBuffer, iNumSamples);
    if (samplesWritten < iNumSamples) {
        // Buffer overflow. We aren't processing samples fast enough. This
        // shouldn't happen since the mic spits out samples just as fast as they
        // come in, right?
        Q_ASSERT(false);
    }
}

void EngineMicrophone::process(const CSAMPLE* pInput, const CSAMPLE* pOutput, const int iBufferSize) {
    CSAMPLE* pOut = const_cast<CSAMPLE*>(pOutput);
    int samplesRead = m_sampleBuffer.read(pOut, iBufferSize);
    if (samplesRead < iBufferSize) {
        // Buffer underflow. There aren't getting samples fast enough. This
        // shouldn't happen since PortAudio should feed us samples just as fast
        // as we consume them, right?
        Q_ASSERT(false);
    }
}
