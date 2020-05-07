#include "preferences/dialog/dlgprefeffects.h"

#include <QDropEvent>
#include <QMimeData>

#include "effects/effectmanifest.h"
#include "effects/effectsbackend.h"
#include "effects/effectsmanager.h"
#include "effects/visibleeffectslist.h"
#include "preferences/effectchainpresetlistmodel.h"

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

    setupChainListView(chainListView);
    setupChainListView(quickEffectListView);

    connect(chainPresetImportButton, &QPushButton::clicked, this, &DlgPrefEffects::slotImportPreset);
    connect(chainPresetExportButton, &QPushButton::clicked, this, &DlgPrefEffects::slotExportPreset);
    connect(chainPresetRenameButton, &QPushButton::clicked, this, &DlgPrefEffects::slotRenamePreset);
    connect(chainPresetDeleteButton, &QPushButton::clicked, this, &DlgPrefEffects::slotDeletePreset);

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

void DlgPrefEffects::setupChainListView(QListView* pListView) {
    EffectChainPresetListModel* pModel = new EffectChainPresetListModel();
    pListView->setModel(pModel);
    pListView->setDropIndicatorShown(true);
    pListView->setDragDropMode(QAbstractItemView::DragDrop);
    //TODO: prevent drops of duplicate items
    pListView->setDefaultDropAction(Qt::CopyAction);
    connect(pListView,
            &QAbstractItemView::clicked,
            this,
            &DlgPrefEffects::slotChainPresetSelected);
    pListView->installEventFilter(this);
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

    loadChainPresetLists();
}

void DlgPrefEffects::slotApply() {
    m_pVisibleEffectsList->setList(m_pVisibleEffectsModel->getList());
    saveChainPresetLists();
}

void DlgPrefEffects::saveChainPresetLists() {
    auto pModel = dynamic_cast<EffectChainPresetListModel*>(chainListView->model());
    m_pChainPresetManager->setPresetOrder(pModel->stringList());

    pModel = dynamic_cast<EffectChainPresetListModel*>(quickEffectListView->model());
    m_pChainPresetManager->setQuickEffectPresetOrder(pModel->stringList());
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

void DlgPrefEffects::loadChainPresetLists() {
    QStringList chainPresetNames;
    for (const auto& pChainPreset : m_pChainPresetManager->getPresetsSorted()) {
        chainPresetNames << pChainPreset->name();
    }
    auto pModel = dynamic_cast<EffectChainPresetListModel*>(chainListView->model());
    pModel->setStringList(chainPresetNames);

    QStringList quickEffectChainPresetNames;
    for (const auto& pChainPreset : m_pChainPresetManager->getQuickEffectPresetsSorted()) {
        quickEffectChainPresetNames << pChainPreset->name();
    }
    pModel = dynamic_cast<EffectChainPresetListModel*>(quickEffectListView->model());
    pModel->setStringList(quickEffectChainPresetNames);
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

void DlgPrefEffects::slotChainPresetSelected(const QModelIndex& selected) {
    VERIFY_OR_DEBUG_ASSERT(m_pFocusedChainList) {
        return;
    }
    QString chainPresetName = selected.model()->data(selected).toString();
    EffectChainPresetPointer pChainPreset = m_pChainPresetManager->getPreset(chainPresetName);
    if (pChainPreset == nullptr || pChainPreset->isEmpty()) {
        return;
    }

    for (int i = 0; i < m_effectsLabels.size(); ++i) {
        if (i < pChainPreset->effectPresets().size()) {
            EffectPresetPointer pEffectPreset = pChainPreset->effectPresets().at(i);
            if (!pEffectPreset->isEmpty()) {
                QString displayName =
                        m_pBackendManager->getDisplayNameForEffectPreset(
                                pEffectPreset);
                // Code uses 0-indexed numbers; users see 1 indexed numbers
                m_effectsLabels[i]->setText(QString::number(i + 1) + ": " + displayName);
            } else {
                m_effectsLabels[i]->setText(QString::number(i + 1) + ": " + tr("None"));
            }
        } else {
            m_effectsLabels[i]->setText(QString::number(i + 1) + ": " + tr("None"));
        }
    }

    chainPresetExportButton->setEnabled(true);
    chainPresetRenameButton->setEnabled(true);
    chainPresetDeleteButton->setEnabled(true);
}

void DlgPrefEffects::slotImportPreset() {
    m_pChainPresetManager->importPreset();
    loadChainPresetLists();
}

void DlgPrefEffects::slotExportPreset() {
    VERIFY_OR_DEBUG_ASSERT(m_pFocusedChainList) {
        return;
    }
    QModelIndex index = m_pFocusedChainList->selectionModel()->selectedIndexes().at(0);
    const QString& selectedPresetName = m_pFocusedChainList->model()->data(index).toString();
    m_pChainPresetManager->exportPreset(selectedPresetName);
}

void DlgPrefEffects::slotRenamePreset() {
    VERIFY_OR_DEBUG_ASSERT(m_pFocusedChainList) {
        return;
    }
    saveChainPresetLists();
    QModelIndex index = m_pFocusedChainList->selectionModel()->selectedIndexes().at(0);
    const QString& selectedPresetName = m_pFocusedChainList->model()->data(index).toString();
    m_pChainPresetManager->renamePreset(selectedPresetName);
    loadChainPresetLists();
}

void DlgPrefEffects::slotDeletePreset() {
    VERIFY_OR_DEBUG_ASSERT(m_pFocusedChainList) {
        return;
    }
    // If the preset is in the focused list and the unfocused list, only remove
    // it from the focused list, but do not actually delete it so it remains in
    // the unfocused list. Only delete it if it is in one list but not the other.
    QModelIndex index =
            m_pFocusedChainList->selectionModel()->selectedIndexes().at(0);
    const QString& selectedPresetName =
            m_pFocusedChainList->model()->data(index).toString();
    QListView* pUnfocusedChainList = unfocusedChainList();
    VERIFY_OR_DEBUG_ASSERT(pUnfocusedChainList) {
        return;
    }
    auto pUnfocusedModel = dynamic_cast<EffectChainPresetListModel*>(
            pUnfocusedChainList->model());
    auto unfocusedChainStringList = pUnfocusedModel->stringList();
    if (unfocusedChainStringList.contains(selectedPresetName)) {
        auto pFocusedModel = dynamic_cast<EffectChainPresetListModel*>(
                m_pFocusedChainList->model());
        QStringList focusedChainStringList = pFocusedModel->stringList();
        focusedChainStringList.removeAll(selectedPresetName);
        pFocusedModel->setStringList(focusedChainStringList);
    } else {
        m_pChainPresetManager->deletePreset(selectedPresetName);
        loadChainPresetLists();
    }
}

bool DlgPrefEffects::eventFilter(QObject* pChainList, QEvent* event) {
    if (event->type() == QEvent::FocusIn) {
        auto pListView = dynamic_cast<QListView*>(pChainList);
        if (!pListView) {
            return false;
        }
        m_pFocusedChainList = pListView;
    }
    return false;
}

QListView* DlgPrefEffects::unfocusedChainList() {
    if (m_pFocusedChainList == chainListView) {
        return quickEffectListView;
    } else {
        return chainListView;
    }
}
