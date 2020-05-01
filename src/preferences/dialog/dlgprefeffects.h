#ifndef DLGPREFEFFECTS_H
#define DLGPREFEFFECTS_H

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
                   EffectsManager* pEffectsManager);
    virtual ~DlgPrefEffects();

    void slotUpdate();
    void slotApply();
    void slotResetToDefaults();

  private slots:
    void availableEffectsListItemSelected(const QModelIndex& selected);
    void slotChainPresetSelected(const QString& chainPresetName);
    void slotImportPreset();
    void slotExportPreset();
    void slotRenamePreset();
    void slotDeletePreset();
    void slotPresetMoveUp();
    void slotPresetMoveDown();

  private:
    void clear();
    void loadChainPresetList();

    QList<QLabel*> m_effectsLabels;

    EffectSettingsModel m_availableEffectsModel;
    UserSettingsPointer m_pConfig;
    EffectsManager* m_pEffectsManager;
    EffectChainPresetManagerPointer m_pChainPresetManager;
    EffectSettingsModel* m_pAvailableEffectsModel;
};

#endif /* DLGPREFEFFECTS_H */
