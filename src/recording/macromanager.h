#pragma once

#include <QObject>

#include "engine/enginemaster.h"
#include "mixer/playermanager.h"
#include "preferences/usersettings.h"

class MacroManager : public QObject {
    Q_OBJECT
  public:
    MacroManager(
            UserSettingsPointer pConfig,
            EngineMaster* pMaster,
            PlayerManager* pPlayerManager);
    void startRecording();
    void stopRecording();
    bool isRecordingActive() const;

  private slots:
    void slotHotcueActivate(double v);
    void slotSetRecording(bool recording);
    void slotToggleRecording(double value);

  private:
    ControlProxy* m_hotcueActivate;
    UserSettingsPointer m_pConfig;

    ControlProxy* m_recStatus;
    ControlObject* m_recStatusCO;
    ControlPushButton* m_pToggleRecording;
};
