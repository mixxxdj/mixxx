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
