// engineaux.h
// created 4/8/2011 by Bill Good (bkgood@gmail.com)
// unapologetically copied from enginemicrophone.h from RJ

#ifndef ENGINEAUX_H
#define ENGINEAUX_H

#include "util/circularbuffer.h"
#include "controlpushbutton.h"
#include "engine/enginechannel.h"
#include "engine/engineclipping.h"
#include "engine/enginevumeter.h"
#include "soundmanagerutil.h"

// EngineAux is an EngineChannel that implements a mixing source whose
// samples are fed directly from the SoundManager
class EngineAux : public EngineChannel, public AudioDestination {
    Q_OBJECT
  public:
    EngineAux(const char* pGroup);
    virtual ~EngineAux();

    bool isActive() const;

    // Called by EngineMaster whenever is requesting a new buffer of audio.
    virtual void process(const CSAMPLE* pInput, CSAMPLE* pOutput, const int iBufferSize);

    // This is called by SoundManager whenever there are new samples from the
    // deck to be processed
    virtual void receiveBuffer(AudioInput input, const CSAMPLE* pBuffer,
                               unsigned int nFrames);

    // Called by SoundManager whenever the aux input is connected to a
    // soundcard input.
    virtual void onInputConnected(AudioInput input);

    // Called by SoundManager whenever the aux input is disconnected from
    // a soundcard input.
    virtual void onInputDisconnected(AudioInput input);

  private:
    EngineClipping m_clipping;
    EngineVuMeter m_vuMeter;
    ControlObject* m_pEnabled;
    ControlPushButton* m_pPassing;
    CSAMPLE* m_pConversionBuffer;
    CircularBuffer<CSAMPLE> m_sampleBuffer;
};

#endif // ENGINEAUX_H
