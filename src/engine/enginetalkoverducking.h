#pragma once

#include "control/controlpushbutton.h"
#include "engine/enginesidechaincompressor.h"
#include "util/parented_ptr.h"

class ControlProxy;
class ControlPotmeter;

class EngineTalkoverDucking : public QObject, public EngineSideChainCompressor {
  Q_OBJECT
  public:

    enum TalkoverDuckSetting {
        OFF = 0,
        AUTO,
        MANUAL,
    };

    EngineTalkoverDucking(UserSettingsPointer pConfig, const QString& group);

    TalkoverDuckSetting getMode() const {
        return static_cast<TalkoverDuckSetting>(int(m_pTalkoverDucking->get()));
    }

    CSAMPLE getGain(int numFrames);

  public slots:
    void slotSampleRateChanged(double);
    void slotDuckStrengthChanged(double);
    void slotDuckAttackTimeChanged(double);
    void slotDuckDecayTimeChanged(double);

  private:
    UserSettingsPointer m_pConfig;
    const QString m_group;

    parented_ptr<ControlProxy> m_pSampleRate;
    std::unique_ptr<ControlPotmeter> m_pDuckStrength;
    std::unique_ptr<ControlPotmeter> m_pDuckAttackTime;
    std::unique_ptr<ControlPotmeter> m_pDuckDecayTime;
    std::unique_ptr<ControlPushButton> m_pTalkoverDucking;
};
