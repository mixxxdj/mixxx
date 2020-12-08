#pragma once

#include <QWidget>

#include "preferences/dialog/ui_dlgprefnovinyldlg.h"
#include "preferences/usersettings.h"
#include "preferences/dlgpreferencepage.h"

class SoundManager;

class DlgPrefNoVinyl : public DlgPreferencePage, Ui::DlgPrefNoVinylDlg  {
    Q_OBJECT
  public:
    DlgPrefNoVinyl(QWidget *parent, SoundManager* soundman, UserSettingsPointer _config);
    virtual ~DlgPrefNoVinyl();
};
