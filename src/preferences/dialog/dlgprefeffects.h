#ifndef DLGPREFEFFECTS_H
#define DLGPREFEFFECTS_H

#include "effects/effectsmanager.h"
#include "preferences/usersettings.h"
#include "preferences/dialog/ui_dlgprefeffectsdlg.h"
#include "preferences/dlgpreferencepage.h"
#include "preferences/effectsettingsmodel.h"

class EffectsManager;

class DlgPrefEffects : public DlgPreferencePage, public Ui::DlgPrefEffectsDlg {
    Q_OBJECT
  public:
    DlgPrefEffects(QWidget* pParent,
                   UserSettingsPointer pConfig,
                   std::shared_ptr<EffectsManager> pEffectsManager);
    virtual ~DlgPrefEffects();

    void slotUpdate();
    void slotApply();
    void slotResetToDefaults();

  private slots:
    void availableEffectsListItemSelected(const QModelIndex& selected);

  private:
    void clear();

    EffectSettingsModel m_availableEffectsModel;
    UserSettingsPointer m_pConfig;
    std::shared_ptr<EffectsManager> m_pEffectsManager;
    EffectSettingsModel* m_pAvailableEffectsModel;
};

#endif /* DLGPREFEFFECTS_H */
