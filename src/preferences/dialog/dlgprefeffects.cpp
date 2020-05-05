#include "preferences/dialog/dlgprefeffects.h"

#include "effects/effectmanifest.h"
#include "effects/effectsbackend.h"
#include "effects/effectsmanager.h"
#include "effects/visibleeffectslist.h"

DlgPrefEffects::DlgPrefEffects(QWidget* pParent,
        UserSettingsPointer pConfig,
        EffectsManager* pEffectsManager)
        : DlgPreferencePage(pParent),
          m_pConfig(pConfig),
          m_pVisibleEffectsList(pEffectsManager->getVisibleEffectsList()),
          m_pChainPresetManager(pEffectsManager->getChainPresetManager()),
          m_pBackendManager(pEffectsManager->getBackendManager()) {
    setupUi(this);

    m_pVisibleEffectsModel = new EffectManifestTableModel(m_pBackendManager);
    visibleEffectsTableView->setModel(m_pVisibleEffectsModel);
    setupManifestTableView(visibleEffectsTableView);

    m_pHiddenEffectsModel = new EffectManifestTableModel(m_pBackendManager);
    hiddenEffectsTableView->setModel(m_pHiddenEffectsModel);
    setupManifestTableView(hiddenEffectsTableView);

    chainListWidget->setDragEnabled(true);
    chainListWidget->viewport()->setAcceptDrops(true);
    chainListWidget->setDropIndicatorShown(true);
    chainListWidget->setDragDropMode(QAbstractItemView::InternalMove);

    connect(chainListWidget, &QListWidget::currentTextChanged, this, &DlgPrefEffects::slotChainPresetSelected);

    connect(chainPresetImportButton, &QPushButton::clicked, this, &DlgPrefEffects::slotImportPreset);
    connect(chainPresetExportButton, &QPushButton::clicked, this, &DlgPrefEffects::slotExportPreset);
    connect(chainPresetRenameButton, &QPushButton::clicked, this, &DlgPrefEffects::slotRenamePreset);
    connect(chainPresetDeleteButton, &QPushButton::clicked, this, &DlgPrefEffects::slotDeletePreset);
    connect(chainPresetMoveUpButton, &QPushButton::clicked, this, &DlgPrefEffects::slotPresetMoveUp);
    connect(chainPresetMoveDownButton, &QPushButton::clicked, this, &DlgPrefEffects::slotPresetMoveDown);

    m_effectsLabels.append(effect1Name);
    m_effectsLabels.append(effect2Name);
    m_effectsLabels.append(effect3Name);
}

DlgPrefEffects::~DlgPrefEffects() {
}

void DlgPrefEffects::setupManifestTableView(QTableView* pTableView) {
    pTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    pTableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    pTableView->setDragDropMode(QAbstractItemView::DragDrop);
    // QTableView won't remove dragged items without this??
    pTableView->setDragDropOverwriteMode(false);
    pTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    pTableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(pTableView->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &DlgPrefEffects::effectsTableItemSelected);
}

void DlgPrefEffects::slotUpdate() {
    clear();

    const QList<EffectManifestPointer> visibleEffects = m_pVisibleEffectsList->getList();
    m_pVisibleEffectsModel->setList(visibleEffects);

    QList<EffectManifestPointer> hiddenEffects = m_pBackendManager->getManifests();
    for (const auto& pManifest : visibleEffects) {
        hiddenEffects.removeAll(pManifest);
    }
    m_pHiddenEffectsModel->setList(hiddenEffects);

    // No chain preset is selected when the preferences are opened
    for (int i = 0; i < m_effectsLabels.size(); ++i) {
        m_effectsLabels[i]->setText(QString::number(i + 1) + ": ");
    }

    chainPresetExportButton->setEnabled(false);
    chainPresetRenameButton->setEnabled(false);
    chainPresetDeleteButton->setEnabled(false);
    chainPresetMoveUpButton->setEnabled(false);
    chainPresetMoveDownButton->setEnabled(false);

    loadChainPresetList();
}

void DlgPrefEffects::slotApply() {
    m_pVisibleEffectsList->setList(m_pVisibleEffectsModel->getList());

    QStringList chainList;
    for (int i = 0; i < chainListWidget->count(); ++i) {
        chainList << chainListWidget->item(i)->text();
    }
    m_pChainPresetManager->setPresetOrder(chainList);
}

void DlgPrefEffects::slotResetToDefaults() {
    slotUpdate();
}

void DlgPrefEffects::clear() {
    effectName->clear();
    effectAuthor->clear();
    effectDescription->clear();
    effectVersion->clear();
    effectType->clear();
}

void DlgPrefEffects::loadChainPresetList() {
    chainListWidget->clear();
    for (const auto& pChainPreset : m_pChainPresetManager->getPresetsSorted()) {
        chainListWidget->addItem(pChainPreset->name());
    }
}

void DlgPrefEffects::effectsTableItemSelected(const QModelIndex& selected) {
    auto pModel = static_cast<const EffectManifestTableModel*>(selected.model());
    VERIFY_OR_DEBUG_ASSERT(pModel) {
        return;
    }
    EffectManifestPointer pManifest = pModel->getList().at(selected.row());
    VERIFY_OR_DEBUG_ASSERT(pManifest) {
        return;
    }

    effectName->setText(pManifest->name());
    effectAuthor->setText(pManifest->author());
    effectDescription->setText(pManifest->description());
    effectVersion->setText(pManifest->version());
    effectType->setText(pManifest->translatedBackendName());
}

void DlgPrefEffects::slotChainPresetSelected(const QString& chainPresetName) {
    EffectChainPresetPointer pChainPreset = m_pChainPresetManager->getPreset(chainPresetName);
    if (pChainPreset == nullptr || pChainPreset->isEmpty()) {
        return;
    }

    for (int i = 0; i < pChainPreset->effectPresets().size() - 1; ++i) {
        EffectPresetPointer pEffectPreset = pChainPreset->effectPresets().at(i);
        if (!pEffectPreset->isEmpty()) {
            QString displayName = m_pBackendManager->getDisplayNameForEffectPreset(pEffectPreset);
            // Code uses 0-indexed numbers; users see 1 indexed numbers
            m_effectsLabels[i]->setText(QString::number(i + 1) + ": " + displayName);
        } else {
            m_effectsLabels[i]->setText(QString::number(i + 1) + ": " + tr("None"));
        }
    }

    chainPresetExportButton->setEnabled(true);
    chainPresetRenameButton->setEnabled(true);
    chainPresetDeleteButton->setEnabled(true);
    if (chainListWidget->currentRow() > 0) {
        chainPresetMoveUpButton->setEnabled(true);
    } else {
        chainPresetMoveUpButton->setEnabled(false);
    }
    if (chainListWidget->currentRow() < chainListWidget->count() - 1) {
        chainPresetMoveDownButton->setEnabled(true);
    } else {
        chainPresetMoveDownButton->setEnabled(false);
    }
}

void DlgPrefEffects::slotImportPreset() {
    m_pChainPresetManager->importPreset();
    loadChainPresetList();
}

void DlgPrefEffects::slotExportPreset() {
    const QString& selectedPresetName = chainListWidget->currentItem()->text();
    m_pChainPresetManager->exportPreset(selectedPresetName);
}

void DlgPrefEffects::slotRenamePreset() {
    const QString& selectedPresetName = chainListWidget->currentItem()->text();
    m_pChainPresetManager->renamePreset(selectedPresetName);
    loadChainPresetList();
}

void DlgPrefEffects::slotDeletePreset() {
    const QString& selectedPresetName = chainListWidget->currentItem()->text();
    m_pChainPresetManager->deletePreset(selectedPresetName);
    loadChainPresetList();
}

void DlgPrefEffects::slotPresetMoveUp() {
    QListWidgetItem* item = chainListWidget->currentItem();
    int oldRow = chainListWidget->currentRow();
    if (oldRow == 0) {
        return;
    }
    chainListWidget->takeItem(oldRow);
    chainListWidget->insertItem(oldRow - 1, item);
    chainListWidget->setCurrentRow(oldRow - 1);
}

void DlgPrefEffects::slotPresetMoveDown() {
    QListWidgetItem* item = chainListWidget->currentItem();
    int oldRow = chainListWidget->currentRow();
    if (oldRow == chainListWidget->count()) {
        return;
    }
    chainListWidget->takeItem(oldRow);
    chainListWidget->insertItem(oldRow + 1, item);
    chainListWidget->setCurrentRow(oldRow + 1);
}
