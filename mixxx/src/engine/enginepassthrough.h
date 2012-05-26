// enginepassthrough.h
// created 4/8/2011 by Bill Good (bkgood@gmail.com)
// unapologetically copied from enginemicrophone.h from RJ

// Deprecated!!! - 05-25-12 mattmik

#ifndef ENGINEPASSTHROUGH_H
#define ENGINEPASSTHROUGH_H

#include "circularbuffer.h"
#include "controlpushbutton.h"
#include "engine/enginechannel.h"
#include "engine/engineclipping.h"
#include "engine/enginevumeter.h"

#include "engine/enginepregain.h"
#include "engine/enginefilterblock.h"
#include "engine/engineflanger.h"

#include "soundmanagerutil.h"

// EnginePassthrough is an EngineChannel that implements a mixing source whose
// samples are fed directly from the SoundManager
class EnginePassthrough : public EngineChannel, public AudioDestination {
    Q_OBJECT
  public:
    EnginePassthrough(const char *pGroup, const char* pDeckGroup);
    virtual ~EnginePassthrough();

    bool isActive();
    bool isPFL();
    bool isMaster();

    // Called by EngineMaster whenever is requesting a new buffer of audio.
    virtual void process(const CSAMPLE *pInput, const CSAMPLE *pOutput, const int iBufferSize);

    // This is called by SoundManager whenever there are new samples from the
    // deck to be processed
    virtual void receiveBuffer(AudioInput input, const short *pBuffer, unsigned int nFrames);

    // Called by SoundManager whenever the passthrough input is connected to a
    // soundcard input.
    virtual void onInputConnected(AudioInput input);

    // Called by SoundManager whenever the passthrough input is disconnected from
    // a soundcard input.
    virtual void onInputDisconnected(AudioInput input);

  private:
    EngineClipping m_clipping;
    EngineVuMeter m_vuMeter;
    ControlObject* m_pEnabled;
    ControlPushButton* m_pPassing;
    CSAMPLE* m_pConversionBuffer;
    CircularBuffer<CSAMPLE> m_sampleBuffer;

    EngineClipping* m_pClipping;
    EnginePregain* m_pPregain;
    EngineFilterBlock* m_pFilter;
    EngineFlanger* m_pFlanger;
    EngineVuMeter* m_pVUMeter;
};

#endif /* ENGINEPASSTHROUGH_H */
