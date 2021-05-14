// modplug settings dialog

#pragma once

#include <QDialog>

#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/usersettings.h"

namespace Ui {
class DlgPrefModplug;
}

class DlgPrefModplug : public DlgPreferencePage {
    Q_OBJECT

  public:
    explicit DlgPrefModplug(QWidget* parent, UserSettingsPointer _config);
    virtual ~DlgPrefModplug();

  public slots:
    void slotApply() override;
    void slotUpdate() override;
    void slotResetToDefaults() override;

    void loadSettings();
    void saveSettings();
    void applySettings();

  private:
    Ui::DlgPrefModplug* m_pUi;
    UserSettingsPointer m_pConfig;
};
