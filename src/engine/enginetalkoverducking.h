#include "engine/enginesidechaincompressor.h"
#include "controlpotmeter.h"
#include "controlpushbutton.h"

class ConfigValue;
class ControlObjectSlave;

class EngineTalkoverDucking : public QObject, public EngineSideChainCompressor {
  Q_OBJECT
  public:

    enum TalkoverDuckSetting {
        OFF = 0,
        AUTO,
        MANUAL,
    };

    EngineTalkoverDucking(UserSettingsPointer pConfig, const char* group);
    virtual ~EngineTalkoverDucking();

    TalkoverDuckSetting getMode() const {
        return static_cast<TalkoverDuckSetting>(int(m_pTalkoverDucking->get()));
    }

    CSAMPLE getGain(int numFrames);

  public slots:
    void slotSampleRateChanged(double);
    void slotDuckStrengthChanged(double);
    void slotDuckModeChanged(double);

  private:
    UserSettingsPointer m_pConfig;
    const char* m_group;

    ControlObjectSlave* m_pMasterSampleRate;
    ControlPotmeter* m_pDuckStrength;
    ControlPushButton* m_pTalkoverDucking;
};
