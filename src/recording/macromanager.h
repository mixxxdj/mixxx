#pragma once

#include <QObject>

#include "engine/enginemaster.h"
#include "mixer/playermanager.h"
#include "preferences/usersettings.h"

class MacroManager : public RecordingManagerBase {
    Q_OBJECT
  public:
    MacroManager(
            UserSettingsPointer pConfig,
            EngineMaster* pMaster,
            PlayerManager* pPlayerManager);
    ~MacroManager() override;

    void startRecording();
    void stopRecording();

  private slots:
    void slotHotcueActivate(double v);

  private:
    ControlProxy* m_hotcueActivate;
    UserSettingsPointer m_pConfig;

    ControlProxy* m_recStatus;
    ControlObject* m_recStatusCO;
    ControlPushButton* m_pToggleRecording;
};
