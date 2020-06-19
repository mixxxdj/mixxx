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

  private slots:
    void slotHotcueActivate(double v);

  private:
    UserSettingsPointer m_pConfig;
};
