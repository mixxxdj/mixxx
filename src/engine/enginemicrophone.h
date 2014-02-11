// enginemicrophone.h
// created 3/16/2011 by RJ Ryan (rryan@mit.edu)

#ifndef ENGINEMICROPHONE_H
#define ENGINEMICROPHONE_H

#include "util/circularbuffer.h"
#include "controlpushbutton.h"
#include "engine/enginechannel.h"
#include "engine/engineclipping.h"
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

    // Called by EngineMaster whenever is requesting a new buffer of audio.
    virtual void process(const CSAMPLE* pInput, CSAMPLE* pOutput, const int iBufferSize);

    // This is called by SoundManager whenever there are new samples from the
    // microphone to be processed
    virtual void receiveBuffer(AudioInput input, const CSAMPLE* pBuffer,
                               unsigned int iNumSamples);

    // Called by SoundManager whenever the microphone input is connected to a
    // soundcard input.
    virtual void onInputConnected(AudioInput input);

    // Called by SoundManager whenever the microphone input is disconnected from
    // a soundcard input.
    virtual void onInputDisconnected(AudioInput input);

    bool isSolo();
    double getSoloDamping();

  private:
    EngineClipping m_clipping;
    EngineVuMeter m_vuMeter;
    ControlObject* m_pEnabled;
    ControlPushButton* m_pControlTalkover;
    CSAMPLE* m_pConversionBuffer;
    CircularBuffer<CSAMPLE> m_sampleBuffer;
};

#endif /* ENGINEMICROPHONE_H */
