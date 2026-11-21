#pragma once

#include <QDialog>

#include "dialog/ui_dlgaboutdlg.h"
#include "preferences/usersettings.h"

namespace mixxx {
namespace skin {
class SkinLoader;
} // namespace skin
} // namespace mixxx

class DlgAbout : public QDialog, public Ui::DlgAboutDlg {
    Q_OBJECT
  public:
    DlgAbout(UserSettingsPointer pConfig, mixxx::skin::SkinLoader* pSkinLoader);

  private:
    UserSettingsPointer m_pConfig;
    mixxx::skin::SkinLoader* m_pSkinLoader;
};
