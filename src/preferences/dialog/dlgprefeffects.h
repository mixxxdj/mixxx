#pragma once

#include "preferences/dialog/ui_dlgprefeffectsdlg.h"
#include "preferences/dlgpreferencepage.h"
#include "preferences/effectmanifesttablemodel.h"
#include "preferences/usersettings.h"

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
    void effectsTableItemSelected(const QModelIndex& selected);
    void slotChainPresetSelected(const QString& chainPresetName);
    void slotImportPreset();
    void slotExportPreset();
    void slotRenamePreset();
    void slotDeletePreset();
    void slotPresetMoveUp();
    void slotPresetMoveDown();

  private:
    void setupManifestTableView(QTableView* pTableView);

    void clear();
    void loadChainPresetList();

    QList<QLabel*> m_effectsLabels;

    UserSettingsPointer m_pConfig;
    VisibleEffectsListPointer m_pVisibleEffectsList;
    EffectChainPresetManagerPointer m_pChainPresetManager;
    EffectsBackendManagerPointer m_pBackendManager;
    EffectManifestTableModel* m_pVisibleEffectsModel;
    EffectManifestTableModel* m_pHiddenEffectsModel;
};
