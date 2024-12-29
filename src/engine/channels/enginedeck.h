#pragma once

#include <QScopedPointer>

#include "preferences/usersettings.h"
#include "engine/channels/enginechannel.h"
#include "soundio/soundmanagerutil.h"

class EnginePregain;
class EngineBuffer;
class EngineMixer;
class ControlPushButton;

class EngineDeck : public EngineChannel, public AudioDestination {
    Q_OBJECT
  public:
    EngineDeck(
            const ChannelHandleAndGroup& handleGroup,
            UserSettingsPointer pConfig,
            EngineMixer* pMixingEngine,
            EffectsManager* pEffectsManager,
            EngineChannel::ChannelOrientation defaultOrientation,
            bool primaryDeck);
    ~EngineDeck() override;

    void process(CSAMPLE* pOutput, const int iBufferSize) override;
    void collectFeatures(GroupFeatureState* pGroupFeatures) const override;

    // postProcessLocalBpm() is called on all decks to update the localBpm after
    // process() is done. Updated localBpms for all decks are required for the
    // postProcess() step, to avoid issues with the order they are processed.
    // It cannot be done during process() because it relies that the localBpm
    // of all decks are on their old values.
    void postProcessLocalBpm() override;

    // Update beat distances, sync modes, and other values that are only known
    // after all other processing is done.
    void postProcess(const int iBufferSize) override;

    // TODO(XXX) This hack needs to be removed.
    EngineBuffer* getEngineBuffer() override;

    EngineChannel::ActiveState updateActiveState() override;

    // This is called by SoundManager whenever there are new samples from the
    // configured input to be processed. This is run in the callback thread of
    // the soundcard this AudioDestination was registered for! Beware, in the
    // case of multiple soundcards, this method is not re-entrant but it may be
    // concurrent with EngineMixer processing.
    void receiveBuffer(const AudioInput& input,
            const CSAMPLE* pBuffer,
            unsigned int nFrames) override;

    // Called by SoundManager whenever the passthrough input is connected to a
    // soundcard input.
    void onInputConfigured(const AudioInput& input) override;

    // Called by SoundManager whenever the passthrough input is disconnected
    // from a soundcard input.
    void onInputUnconfigured(const AudioInput& input) override;

    // Return whether or not passthrough is active
    bool isPassthroughActive() const;

  signals:
    void noPassthroughInputConfigured();

  public slots:
    void slotPassthroughToggle(double v);
    void slotPassthroughChangeRequest(double v);

  private:
    UserSettingsPointer m_pConfig;
    EngineBuffer* m_pBuffer;
    EnginePregain* m_pPregain;

    // Begin vinyl passthrough fields
    QScopedPointer<ControlObject> m_pInputConfigured;
    ControlPushButton* m_pPassing;
    bool m_bPassthroughIsActive;
    bool m_bPassthroughWasActive;
};
