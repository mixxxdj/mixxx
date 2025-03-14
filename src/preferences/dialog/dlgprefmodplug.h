// modplug settings dialog

#pragma once

#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/usersettings.h"

namespace Ui {
class DlgPrefModplug;
} // namespace Ui

class DlgPrefModplug : public DlgPreferencePage {
    Q_OBJECT

  public:
    explicit DlgPrefModplug(QWidget* parent, UserSettingsPointer _config);
    virtual ~DlgPrefModplug();

    void updateColoredLinkTexts() override;

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
