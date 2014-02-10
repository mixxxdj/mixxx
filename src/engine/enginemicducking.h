#include "engine/enginesidechaincompressor.h"
#include "controlpotmeter.h"
#include "controlpushbutton.h"

class ConfigValue;
class ControlObjectSlave;

class EngineMicDucking : public QObject, public EngineSideChainCompressor {
  Q_OBJECT
  public:
    #define MIC_DUCK_THRESHOLD 0.1

    enum MicDuckSetting {
        OFF = 0,
        AUTO,
        MANUAL,
    };

    EngineMicDucking(ConfigObject<ConfigValue>* pConfig, const char* group);
    virtual ~EngineMicDucking();

    bool getMode() const { return static_cast<int>(m_pMicDucking->get()); }
    CSAMPLE getMaxStrength() const { return m_pDuckStrength->get(); }

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
