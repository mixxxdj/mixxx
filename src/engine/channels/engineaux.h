#pragma once

#include <QScopedPointer>

#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "engine/channels/enginechannel.h"
#include "util/circularbuffer.h"
#include "soundio/soundmanagerutil.h"

class ControlAudioTaperPot;

/// EngineAux is an EngineChannel that implements a mixing source whose
/// samples are fed directly from the SoundManager
class EngineAux : public EngineChannel, public AudioDestination {
    Q_OBJECT
  public:
    EngineAux(const ChannelHandleAndGroup& handle_group, EffectsManager* pEffectsManager);
    virtual ~EngineAux();

    bool isActive();

    /// Called by EngineMaster whenever is requesting a new buffer of audio.
    virtual void process(CSAMPLE* pOutput, const int iBufferSize);
    virtual void collectFeatures(GroupFeatureState* pGroupFeatures) const;
    virtual void postProcess(const int iBufferSize) { Q_UNUSED(iBufferSize) }

    /// This is called by SoundManager whenever there are new samples from the
    /// configured input to be processed. This is run in the callback thread of
    /// the soundcard this AudioDestination was registered for! Beware, in the
    /// case of multiple soundcards, this method is not re-entrant but it may be
    /// concurrent with EngineMaster processing.
    virtual void receiveBuffer(const AudioInput& input,
            const CSAMPLE* pBuffer,
            unsigned int nFrames);

    /// Called by SoundManager whenever the aux input is connected to a
    /// soundcard input.
    virtual void onInputConfigured(const AudioInput& input);

    /// Called by SoundManager whenever the aux input is disconnected from
    /// a soundcard input.
    virtual void onInputUnconfigured(const AudioInput& input);

  private:
    QScopedPointer<ControlObject> m_pInputConfigured;
    ControlAudioTaperPot* m_pPregain;
    bool m_wasActive;
};
