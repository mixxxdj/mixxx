#pragma once

#include "control/controlproxy.h"
#include "control/grouphandle.h"
#include "effects/effectsmanager.h"
#include "engine/engineobject.h"
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

    EngineChannel(GroupHandle groupHandle,
            ChannelOrientation defaultOrientation,
            EffectsManager* pEffectsManager,
            bool isTalkoverChannel,
            bool isPrimaryDeck);
    virtual ~EngineChannel();

    virtual ChannelOrientation getOrientation() const;

    GroupHandle getHandle() const {
        return m_groupHandle;
    }

    QString getGroup() const {
        return nameOfGroupHandle(m_groupHandle);
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
    int getChannelIndex() {
        return m_channelIndex;
    }
    void setChannelIndex(int channelIndex) {
        m_channelIndex = channelIndex;
    }

    virtual void process(CSAMPLE* pOut, const int iBufferSize) = 0;
    virtual void collectFeatures(GroupFeatureState* pGroupFeatures) const = 0;
    virtual void postProcess(const int iBuffersize) = 0;

    // TODO(XXX) This hack needs to be removed.
    virtual EngineBuffer* getEngineBuffer() {
        return NULL;
    }

  protected:
    const GroupHandle m_groupHandle;
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
    int m_channelIndex;
};
