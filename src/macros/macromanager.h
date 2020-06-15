#pragma once

#include <QObject>

#include "engine/enginemaster.h"
#include "preferences/usersettings.h"

class MacroManager : public QObject {
    Q_OBJECT
  public:
    MacroManager(UserSettingsPointer pConfig);

  private:
    UserSettingsPointer m_pConfig;
};
