#pragma once

#include "control/controlproxy.h"
#include "effects/effectsmanager.h"
#include "engine/engineobject.h"
#include "engine/channelhandle.h"
#include "engine/enginevumeter.h"
#include "preferences/usersettings.h"

class ControlObject;
class EngineBuffer;
class EngineFilterBlock;
class ControlPushButton;

class EngineChannel : public EngineObject {
    Q_OBJECT
  public:
    enum ChannelOrientation {
        LEFT = 0,
        CENTER,
        RIGHT,
    };

    EngineChannel(const ChannelHandleAndGroup& handleGroup,
            ChannelOrientation defaultOrientation,
            EffectsManager* pEffectsManager,
            bool isTalkoverChannel,
            bool isPrimaryDeck);
    virtual ~EngineChannel();

    virtual ChannelOrientation getOrientation() const;

    inline const ChannelHandle& getHandle() const {
        return m_group.handle();
    }

    const QString& getGroup() const {
        return m_group.name();
    }

    virtual bool isActive() = 0;
    void setPfl(bool enabled);
    virtual bool isPflEnabled() const;
    void setMaster(bool enabled);
    virtual bool isMasterEnabled() const;
    void setTalkover(bool enabled);
    virtual bool isTalkoverEnabled() const;
    inline bool isTalkoverChannel() { return m_bIsTalkoverChannel; };
    inline bool isPrimaryDeck() {
        return m_bIsPrimaryDeck;
    };

    virtual void process(CSAMPLE* pOut, const int iBufferSize) = 0;
    virtual void collectFeatures(GroupFeatureState* pGroupFeatures) const = 0;
    virtual void postProcess(const int iBuffersize) = 0;

    // TODO(XXX) This hack needs to be removed.
    virtual EngineBuffer* getEngineBuffer() {
        return NULL;
    }

  protected:
    const ChannelHandleAndGroup m_group;
    EffectsManager* m_pEffectsManager;

    EngineVuMeter m_vuMeter;
    ControlProxy* m_pSampleRate;
    const CSAMPLE* volatile m_sampleBuffer;

    // If set to true, this engine channel represents one of the primary playback decks.
    // It is used to check for valid bpm targets by the sync code.
    const bool m_bIsPrimaryDeck;

  private slots:
    void slotOrientationLeft(double v);
    void slotOrientationRight(double v);
    void slotOrientationCenter(double v);

  private:
    ControlPushButton* m_pMaster;
    ControlPushButton* m_pPFL;
    ControlPushButton* m_pOrientation;
    ControlPushButton* m_pOrientationLeft;
    ControlPushButton* m_pOrientationRight;
    ControlPushButton* m_pOrientationCenter;
    ControlPushButton* m_pTalkover;
    bool m_bIsTalkoverChannel;
};
