#pragma once

#include <QButtonGroup>

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
            std::shared_ptr<EffectsManager> pEffectsManager);
    virtual ~DlgPrefEffects();

    void slotUpdate() override;
    void slotApply() override;
    void slotResetToDefaults() override;

  private slots:
    void effectsTableItemSelected(const QModelIndex& selected);
    void slotChainPresetSelected(const QModelIndex& selected);
    void slotImportPreset();
    void slotExportPreset();
    void slotRenamePreset();
    void slotDeletePreset();

  private:
    void setupManifestTableView(QTableView* pTableView);
    void setupChainListView(QListView* pListView);

    void clear();
    void loadChainPresetLists();
    void saveChainPresetLists();

    bool eventFilter(QObject* pChainList, QEvent* event) override;
    QListView* m_pFocusedChainList;
    QListView* unfocusedChainList();

    QList<QLabel*> m_effectsLabels;

    UserSettingsPointer m_pConfig;
    VisibleEffectsListPointer m_pVisibleEffectsList;
    EffectChainPresetManagerPointer m_pChainPresetManager;
    EffectsBackendManagerPointer m_pBackendManager;
    EffectManifestTableModel* m_pVisibleEffectsModel;
    EffectManifestTableModel* m_pHiddenEffectsModel;
};
