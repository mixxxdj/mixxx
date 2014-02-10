#include "engine/enginesidechaincompressor.h"
#include "controlpotmeter.h"
#include "controlpushbutton.h"

class ConfigValue;
class ControlObjectSlave;

class EngineMicDucking : public QObject, public EngineSideChainCompressor {
  Q_OBJECT
  public:

    enum MicDuckSetting {
        OFF = 0,
        AUTO,
        MANUAL,
    };

    EngineMicDucking(ConfigObject<ConfigValue>* pConfig, const char* group);
    virtual ~EngineMicDucking();

    MicDuckSetting getMode() const { return static_cast<MicDuckSetting>(m_pMicDucking->get()); }

    CSAMPLE getGain(int numFrames);

  public slots:
    void slotSampleRateChanged(double);
    void slotDuckStrengthChanged(double);
    void slotDuckModeChanged(double);

  private:
    ConfigObject<ConfigValue>* m_pConfig;
    const char* m_group;

    ControlObjectSlave* m_pMasterSampleRate;
    ControlPotmeter* m_pDuckStrength;
    ControlPushButton* m_pMicDucking;
};
