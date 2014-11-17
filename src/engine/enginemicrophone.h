// enginemicrophone.h
// created 3/16/2011 by RJ Ryan (rryan@mit.edu)

#ifndef ENGINEMICROPHONE_H
#define ENGINEMICROPHONE_H

#include "controlobjectslave.h"
#include "controlpushbutton.h"
#include "engine/enginechannel.h"
#include "engine/enginevumeter.h"
#include "util/circularbuffer.h"

#include "soundmanagerutil.h"

class EffectsManager;
class EngineEffectsManager;
class ControlAudioTaperPot;

// EngineMicrophone is an EngineChannel that implements a mixing source whose
// samples are fed directly from the SoundManager
class EngineMicrophone : public EngineChannel, public AudioDestination {
    Q_OBJECT
  public:
    EngineMicrophone(QString pGroup, EffectsManager* pEffectsManager);
    virtual ~EngineMicrophone();

    bool isActive();

    // Called by EngineMaster whenever is requesting a new buffer of audio.
    virtual void process(CSAMPLE* pOutput, const int iBufferSize);
    virtual void postProcess(const int iBufferSize) { Q_UNUSED(iBufferSize) }

    // This is called by SoundManager whenever there are new samples from the
    // configured input to be processed. This is run in the callback thread of
    // the soundcard this AudioDestination was registered for! Beware, in the
    // case of multiple soundcards, this method is not re-entrant but it may be
    // concurrent with EngineMaster processing.
    virtual void receiveBuffer(AudioInput input, const CSAMPLE* pBuffer,
                               unsigned int iNumSamples);

    // Called by SoundManager whenever the microphone input is connected to a
    // soundcard input.
    virtual void onInputConfigured(AudioInput input);

    // Called by SoundManager whenever the microphone input is disconnected from
    // a soundcard input.
    virtual void onInputUnconfigured(AudioInput input);

    bool isSolo();
    double getSoloDamping();

  private:
    EngineEffectsManager* m_pEngineEffectsManager;
    EngineVuMeter m_vuMeter;
    ControlObject* m_pEnabled;
    ControlAudioTaperPot* m_pPregain;
    ControlObjectSlave* m_pSampleRate;
    const CSAMPLE* volatile m_sampleBuffer;

    bool m_wasActive;
};

#endif /* ENGINEMICROPHONE_H */
