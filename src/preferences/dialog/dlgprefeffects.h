#pragma once

#include <QButtonGroup>

#include "preferences/dialog/ui_dlgprefeffectsdlg.h"
#include "preferences/dlgpreferencepage.h"
#include "preferences/effectsettingsmodel.h"
#include "preferences/usersettings.h"

class EffectsManager;

class DlgPrefEffects : public DlgPreferencePage, public Ui::DlgPrefEffectsDlg {
    Q_OBJECT
  public:
    DlgPrefEffects(QWidget* pParent,
                   UserSettingsPointer pConfig,
                   EffectsManager* pEffectsManager);
    virtual ~DlgPrefEffects();

    void slotUpdate() override;
    void slotApply() override;
    void slotResetToDefaults() override;

  private slots:
    void availableEffectsListItemSelected(const QModelIndex& selected);

  private:
    void clear();

    EffectSettingsModel m_availableEffectsModel;
    UserSettingsPointer m_pConfig;
    EffectsManager* m_pEffectsManager;
};
