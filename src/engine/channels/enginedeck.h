#pragma once

#include <QScopedPointer>

#include "engine/channels/enginechannel.h"
#include "preferences/usersettings.h"
#include "soundio/soundmanagerutil.h"
#include "track/track_decl.h"
#include "util/samplebuffer.h"

class EnginePregain;
class EngineBuffer;
class EngineMixer;
class ControlPushButton;
class ControlPotmeter;

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

    void process(CSAMPLE* pOutput, const std::size_t bufferSize) override;
    void collectFeatures(GroupFeatureState* pGroupFeatures) const override;

    // postProcessLocalBpm() is called on all decks to update the localBpm after
    // process() is done. Updated localBpms for all decks are required for the
    // postProcess() step, to avoid issues with the order they are processed.
    // It cannot be done during process() because it relies that the localBpm
    // of all decks are on their old values.
    void postProcessLocalBpm() override;

    // Update beat distances, sync modes, and other values that are only known
    // after all other processing is done.
    void postProcess(const std::size_t bufferSize) override;

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

#ifdef __STEM__
    // Clone the stem state (gain and volume) from deckToClone to this. Doesn't
    // check if the loaded track is a stem so this should only be used in case
    // of stem track
    void cloneStemState(const EngineDeck* deckToClone);
    void addStemHandle(const ChannelHandleAndGroup& stemHandleGroup);
    static QString getGroupForStem(QStringView deckGroup, int stemIdx);

    // Sub-channel interface used by ChannelMixer to apply the per-stem
    // Postfader effect chains after the deck's fader gains (post-fader, see
    // issue #16718).
    int subChannelCount() const override {
        return m_activeStemCount;
    }
    ChannelHandle subChannelHandle(int index) const override;
    void copySubChannel(CSAMPLE* pDest, int index, std::size_t numSamples) const override;
#endif

  signals:
    void noPassthroughInputConfigured();

  public slots:
    void slotPassthroughToggle(double v);
    void slotPassthroughChangeRequest(double v);
#ifdef __STEM__
    void slotTrackLoaded(TrackPointer pNewTrack, TrackPointer);
#endif

  private:
#ifdef __STEM__
    // Process multiple channels and mix them together into the passed buffer
    void processStem(CSAMPLE* pOutput, const std::size_t bufferSize);
#endif

    std::vector<ChannelHandleAndGroup> m_stems;
    std::vector<CSAMPLE_GAIN> m_stemsGainCache;

    UserSettingsPointer m_pConfig;
    EngineBuffer* m_pBuffer;
    EnginePregain* m_pPregain;

#ifdef __STEM__
    // Stem buffer used to retrieve all the channel to mix together
    mixxx::SampleBuffer m_stemBuffer;
    std::unique_ptr<ControlObject> m_pStemCount;
    std::vector<std::unique_ptr<ControlPotmeter>> m_stemGain;
    std::vector<std::unique_ptr<ControlPushButton>> m_stemMute;
    bool m_stemClonedState;
    // Number of stems prepared by processStem() for ChannelMixer in the current
    // engine callback. Reset to 0 at the beginning of process(), so that
    // ChannelMixer does not pick up stale sub-channel data (e.g. while
    // passthrough is active or when playing a regular stereo track).
    int m_activeStemCount;
#endif

    // Begin vinyl passthrough fields
    QScopedPointer<ControlObject> m_pInputConfigured;
    ControlPushButton* m_pPassing;
    bool m_bPassthroughIsActive;
    bool m_bPassthroughWasActive;
};
