#pragma once

#include "control/pollingcontrolproxy.h"
#include "engine/channelhandle.h"
#include "engine/engineobject.h"
#include "engine/enginevumeter.h"

class EffectsManager;
class EngineBuffer;
class ControlPushButton;

class EngineChannel : public EngineObject {
    Q_OBJECT
  public:
    enum ChannelOrientation {
        LEFT = 0,
        CENTER,
        RIGHT,
    };

    enum class ActiveState {
        Inactive = 0,
        Active,
        WasActive
    };

    EngineChannel(const ChannelHandleAndGroup& handleGroup,
            ChannelOrientation defaultOrientation,
            EffectsManager* pEffectsManager,
            bool isTalkoverChannel,
            bool isPrimaryDeck);
    ~EngineChannel() override;

    virtual ChannelOrientation getOrientation() const;

    inline ChannelHandle getHandle() const {
        return m_group.handle();
    }

    const QString& getGroup() const {
        return m_group.name();
    }

    virtual ActiveState updateActiveState() = 0;
    virtual bool isActive() {
        return m_active;
    }

    void setPfl(bool enabled);
    virtual bool isPflEnabled() const;
    void setMainMix(bool enabled);
    virtual bool isMainMixEnabled() const;
    void setTalkover(bool enabled);
    virtual bool isTalkoverEnabled() const;
    inline bool isTalkoverChannel() { return m_bIsTalkoverChannel; };
    inline bool isPrimaryDeck() const {
        return m_bIsPrimaryDeck;
    };
    int getChannelIndex() {
        return m_channelIndex;
    }
    void setChannelIndex(int channelIndex) {
        m_channelIndex = channelIndex;
    }

    virtual void postProcessLocalBpm() {
    }

    virtual void postProcess(const std::size_t bufferSize) {
        Q_UNUSED(bufferSize)
    }

    // TODO(XXX) This hack needs to be removed.
    virtual EngineBuffer* getEngineBuffer() {
        return nullptr;
    }

    // Sub-channels of this channel, e.g. the four stereo stems of a stem track
    // on a deck. ChannelMixer applies the Postfader effect chains of each
    // sub-channel after the fader gains have been applied (post-fader), so
    // that effect tails stay audible when the channel is faded out or the deck
    // is paused (see issue #16718).
    // Returns 0 for channels without sub-channels, which is the case for all
    // channels except stem decks.
    virtual int subChannelCount() const {
        return 0;
    }
    // Identifies the sub-channel with the given index for the effects system.
    // Only called if subChannelCount() returns a value greater than index.
    virtual ChannelHandle subChannelHandle(int) const {
        return m_group.handle();
    }
    // Copies numSamples of the sub-channel's current signal (i.e. after pregain
    // and the sub-channel's own volume/mute have been applied) into pDest.
    // Only called if subChannelCount() returns a value greater than index.
    virtual void copySubChannel(CSAMPLE*, int, std::size_t) const {
    }

  protected:
    const ChannelHandleAndGroup m_group;
    EffectsManager* m_pEffectsManager;

    EngineVuMeter m_vuMeter;
    PollingControlProxy m_sampleRate;
    const CSAMPLE* volatile m_sampleBuffer;

    // If set to true, this engine channel represents one of the primary playback decks.
    // It is used to check for valid bpm targets by the sync code.
    const bool m_bIsPrimaryDeck;
    bool m_active;

  private slots:
    void slotOrientationLeft(double v);
    void slotOrientationRight(double v);
    void slotOrientationCenter(double v);

  private:
    ControlPushButton* m_pMainMix;
    ControlPushButton* m_pPFL;
    ControlPushButton* m_pOrientation;
    ControlPushButton* m_pOrientationLeft;
    ControlPushButton* m_pOrientationRight;
    ControlPushButton* m_pOrientationCenter;
    ControlPushButton* m_pTalkover;
    bool m_bIsTalkoverChannel;
    int m_channelIndex;
};
