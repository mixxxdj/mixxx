#pragma once

#include <QCheckBox>
#include <QWidget>

#include "effects/lv2/lv2backend.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgpreflv2dlg.h"
#include "preferences/usersettings.h"

class EffectsManager;

class DlgPrefLV2 : public DlgPreferencePage, public Ui::DlgPrefLV2Dlg  {
    Q_OBJECT
  public:
    DlgPrefLV2(QWidget* parent,
            LV2Backend* lv2Backend,
            UserSettingsPointer pConfig,
            std::shared_ptr<EffectsManager> pEffectsManager);
    virtual ~DlgPrefLV2();

  public slots:
    /// Called when the preference dialog (not this page) is shown to the user.
    void slotUpdate() override;
    /// Called when the user clicks the global "Apply" button.
    void slotApply() override;
    /// Called when the user clicks the global "Reset to Defaults" button.
    void slotResetToDefaults() override;

  private slots:
    void slotDisplayParameters();
    void slotUpdateOnParameterCheck(int state);

  private:
    LV2Backend* m_pLV2Backend;
    QString m_currentEffectId;
    QList<QCheckBox*> m_pluginParameters;
    int m_iCheckedParameters;
    std::shared_ptr<EffectsManager> m_pEffectsManager;
};
