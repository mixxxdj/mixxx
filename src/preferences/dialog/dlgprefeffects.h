#pragma once

#include "effects/defs.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefeffectsdlg.h"
#include "preferences/usersettings.h"
#include "util/parented_ptr.h"

class EffectsManager;
class EffectManifestTableModel;

class DlgPrefEffects : public DlgPreferencePage, public Ui::DlgPrefEffectsDlg {
    Q_OBJECT
  public:
    DlgPrefEffects(QWidget* pParent,
            UserSettingsPointer pConfig,
            std::shared_ptr<EffectsManager> pEffectsManager);
    virtual ~DlgPrefEffects();

    void slotUpdate() override;
    void slotApply() override;
    void slotResetToDefaults() override;

  private slots:
    void effectsTableItemSelected(const QModelIndex& selected);
    void slotChainPresetSelectionChanged(const QItemSelection& selected);
    void slotImportPreset();
    void slotExportPreset();
    void slotRenamePreset();
    void slotDeletePreset();

  private:
    void setupManifestTableView(QTableView* pTableView);
    void setupChainListView(QListView* pListView);

    void clearEffectInfo();
    void clearChainInfoDisableButtons();
    void loadChainPresetLists();
    void saveChainPresetLists();

    bool eventFilter(QObject* pChainList, QEvent* event) override;
    QListView* m_pFocusedChainList;
    QListView* unfocusedChainList();
    QTableView* m_pFocusedEffectList;
    QTableView* unfocusedEffectList();

    QList<QLabel*> m_effectsLabels;

    UserSettingsPointer m_pConfig;
    VisibleEffectsListPointer m_pVisibleEffectsList;
    EffectChainPresetManagerPointer m_pChainPresetManager;
    EffectsBackendManagerPointer m_pBackendManager;
    parented_ptr<EffectManifestTableModel> m_pVisibleEffectsModel;
    parented_ptr<EffectManifestTableModel> m_pHiddenEffectsModel;
};
