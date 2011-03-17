// enginemicrophone.h
// created 3/16/2011 by RJ Ryan (rryan@mit.edu)

#ifndef ENGINEMICROPHONE_H
#define ENGINEMICROPHONE_H

#include "circularbuffer.h"
#include "engine/enginechannel.h"
#include "engine/engineclipping.h"
#include "engine/enginevolume.h"
#include "engine/enginevumeter.h"
#include "soundmanagerutil.h"

// EngineMicrophone is an EngineChannel that implements a mixing source whose
// samples are fed directly from the SoundManager
class EngineMicrophone : public EngineChannel, public AudioDestination {
    Q_OBJECT
  public:
    EngineMicrophone(const char* pGroup);
    virtual ~EngineMicrophone();

    bool isActive();
    bool isPFL();
    bool isMaster();

    // Called by EngineMaster whenever is requesting a new buffer of audio.
    virtual void process(const CSAMPLE* pInput, const CSAMPLE* pOutput, const int iBufferSize);

    virtual void applyVolume(CSAMPLE* pBuff, const int iBufferSize);

    // This is called by SoundManager whenever there are new samples from the
    // microphone to be processed
    virtual void receiveBuffer(AudioInput input, const short* pBuffer, unsigned int iNumSamples);

  private:
    EngineVolume m_volume;
    EngineClipping m_clipping;
    EngineVuMeter m_vuMeter;
    CSAMPLE* m_pConversionBuffer;
    CircularBuffer<CSAMPLE> m_sampleBuffer;
};

#endif /* ENGINEMICROPHONE_H */
