// libopenmpt module decoder settings dialog

#pragma once

#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/usersettings.h"

namespace Ui {
class DlgPrefOpenMPT;
} // namespace Ui

class DlgPrefOpenMPT : public DlgPreferencePage {
    Q_OBJECT

  public:
    explicit DlgPrefOpenMPT(QWidget* parent, UserSettingsPointer pConfig);
    ~DlgPrefOpenMPT() override;

  public slots:
    void slotApply() override;
    void slotUpdate() override;
    void slotResetToDefaults() override;

    void loadSettings();
    void saveSettings();
    void applySettings();

  private:
    Ui::DlgPrefOpenMPT* m_pUi;
    UserSettingsPointer m_pConfig;
};
