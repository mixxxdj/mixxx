#include <QByteArrayData>
#include <QObject>
#include <QString>

#include "control/controlpotmeter.h"
#include "control/controlpushbutton.h"
#include "engine/enginesidechaincompressor.h"
#include "preferences/usersettings.h"
#include "util/types.h"

class ConfigValue;
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
    const QString m_group;

    ControlProxy* m_pMasterSampleRate;
    ControlPotmeter* m_pDuckStrength;
    ControlPushButton* m_pTalkoverDucking;
};
