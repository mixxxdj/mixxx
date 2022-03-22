#include "preferences/dialog/dlgprefeffects.h"

#include <QDropEvent>
#include <QMimeData>

#include "effects/backends/effectmanifest.h"
#include "effects/backends/effectsbackend.h"
#include "effects/effectsmanager.h"
#include "effects/visibleeffectslist.h"
#include "moc_dlgprefeffects.cpp"
#include "preferences/effectchainpresetlistmodel.h"

DlgPrefEffects::DlgPrefEffects(QWidget* pParent,
        UserSettingsPointer pConfig,
        std::shared_ptr<EffectsManager> pEffectsManager)
        : DlgPreferencePage(pParent),
          m_pFocusedChainList(nullptr),
          m_pConfig(pConfig),
          m_pVisibleEffectsList(pEffectsManager->getVisibleEffectsList()),
          m_pChainPresetManager(pEffectsManager->getChainPresetManager()),
          m_pBackendManager(pEffectsManager->getBackendManager()) {
    setupUi(this);

    m_pVisibleEffectsModel = make_parented<EffectManifestTableModel>(
            visibleEffectsTableView, m_pBackendManager);
    visibleEffectsTableView->setModel(m_pVisibleEffectsModel);
    setupManifestTableView(visibleEffectsTableView);

    m_pHiddenEffectsModel = make_parented<EffectManifestTableModel>(
            hiddenEffectsTableView, m_pBackendManager);
    hiddenEffectsTableView->setModel(m_pHiddenEffectsModel);
    setupManifestTableView(hiddenEffectsTableView);

    // Allow selection only in either of the effects lists at a time to clarify
    // which effect/chain the info below refers to.
    // Upon selection change, deselect items in the adjacent list (reset doesn't
    // emit dataChanged() signal.
    connect(visibleEffectsTableView->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            hiddenEffectsTableView->selectionModel(),
            &QItemSelectionModel::reset);
    connect(hiddenEffectsTableView->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            visibleEffectsTableView->selectionModel(),
            &QItemSelectionModel::reset);

    setupChainListView(chainListView);
    setupChainListView(quickEffectListView);

    connect(chainPresetImportButton,
            &QPushButton::clicked,
            this,
            &DlgPrefEffects::slotImportPreset);
    connect(chainPresetExportButton,
            &QPushButton::clicked,
            this,
            &DlgPrefEffects::slotExportPreset);
    connect(chainPresetRenameButton,
            &QPushButton::clicked,
            this,
            &DlgPrefEffects::slotRenamePreset);
    connect(chainPresetDeleteButton,
            &QPushButton::clicked,
            this,
            &DlgPrefEffects::slotDeletePreset);

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
    pTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(pTableView->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &DlgPrefEffects::effectsTableItemSelected);
}

void DlgPrefEffects::setupChainListView(QListView* pListView) {
    auto pModel = make_parented<EffectChainPresetListModel>(pListView, m_pChainPresetManager);
    pListView->setModel(pModel);
    pListView->setDropIndicatorShown(true);
    pListView->setDragDropMode(QAbstractItemView::DragDrop);
    pListView->setSelectionMode(QAbstractItemView::SingleSelection);
    //TODO: prevent drops of duplicate items
    pListView->setDefaultDropAction(Qt::CopyAction);
    connect(pListView->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &DlgPrefEffects::slotChainPresetSelected);
    pListView->installEventFilter(this);
}

void DlgPrefEffects::slotUpdate() {
    clear();

    // Prevent emission of dataChanged() when clearing the effects lists to not
    // call effectsTableItemSelected() with a selection that has no model.
    visibleEffectsTableView->selectionModel()->reset();
    hiddenEffectsTableView->selectionModel()->reset();

    const QList<EffectManifestPointer> visibleEffects = m_pVisibleEffectsList->getList();
    m_pVisibleEffectsModel->setList(visibleEffects);

    QList<EffectManifestPointer> hiddenEffects = m_pBackendManager->getManifests();
    for (const auto& pManifest : std::as_const(visibleEffects)) {
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

    bool effectAdoptMetaknobValue = m_pConfig->getValue(
            ConfigKey("[Effects]", "AdoptMetaknobValue"), true);
    radioButtonKeepMetaknobPosition->setChecked(effectAdoptMetaknobValue);
    radioButtonMetaknobLoadDefault->setChecked(!effectAdoptMetaknobValue);
}

void DlgPrefEffects::slotApply() {
    m_pVisibleEffectsList->setList(m_pVisibleEffectsModel->getList());
    saveChainPresetLists();

    m_pConfig->set(ConfigKey("[Effects]", "AdoptMetaknobValue"),
            ConfigValue(radioButtonKeepMetaknobPosition->isChecked()));
}

void DlgPrefEffects::saveChainPresetLists() {
    auto pModel = dynamic_cast<EffectChainPresetListModel*>(chainListView->model());
    m_pChainPresetManager->setPresetOrder(pModel->stringList());

    pModel = dynamic_cast<EffectChainPresetListModel*>(quickEffectListView->model());
    m_pChainPresetManager->setQuickEffectPresetOrder(pModel->stringList());
}

void DlgPrefEffects::slotResetToDefaults() {
    radioButtonKeepMetaknobPosition->setChecked(true);
    m_pChainPresetManager->resetToDefaults();

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
    effectType->setText(EffectsBackend::translatedBackendName(pManifest->backendType()));
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
                m_effectsLabels[i]->setText(QString::number(i + 1) + ": " + kNoEffectString);
            }
        } else {
            m_effectsLabels[i]->setText(QString::number(i + 1) + ": " + kNoEffectString);
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
    const auto& selectedIndices = m_pFocusedChainList->selectionModel()->selectedIndexes();
    for (const auto& index : selectedIndices) {
        const QString& selectedPresetName = m_pFocusedChainList->model()->data(index).toString();
        m_pChainPresetManager->exportPreset(selectedPresetName);
    }
}

void DlgPrefEffects::slotRenamePreset() {
    VERIFY_OR_DEBUG_ASSERT(m_pFocusedChainList) {
        return;
    }
    saveChainPresetLists();
    const auto& selectedIndices = m_pFocusedChainList->selectionModel()->selectedIndexes();
    for (const auto& index : selectedIndices) {
        const QString& selectedPresetName = m_pFocusedChainList->model()->data(index).toString();
        m_pChainPresetManager->renamePreset(selectedPresetName);
    }
    loadChainPresetLists();
}

void DlgPrefEffects::slotDeletePreset() {
    // If the preset is in the focused list and the unfocused list, only remove
    // it from the focused list, but do not actually delete it so it remains in
    // the unfocused list. Only delete it if it is in one list but not the other.
    VERIFY_OR_DEBUG_ASSERT(m_pFocusedChainList) {
        return;
    }
    auto pFocusedModel = dynamic_cast<EffectChainPresetListModel*>(
            m_pFocusedChainList->model());
    QStringList focusedChainStringList = pFocusedModel->stringList();

    QListView* pUnfocusedChainList = unfocusedChainList();
    VERIFY_OR_DEBUG_ASSERT(pUnfocusedChainList) {
        return;
    }
    auto pUnfocusedModel = dynamic_cast<EffectChainPresetListModel*>(
            pUnfocusedChainList->model());
    auto unfocusedChainStringList = pUnfocusedModel->stringList();

    const auto& selectedIndices = m_pFocusedChainList->selectionModel()->selectedIndexes();
    for (const auto& index : selectedIndices) {
        QString selectedPresetName =
                m_pFocusedChainList->model()->data(index).toString();
        if (!unfocusedChainStringList.contains(selectedPresetName)) {
            if (m_pChainPresetManager->deletePreset(selectedPresetName)) {
                focusedChainStringList.removeAll(selectedPresetName);
            }
        } else {
            focusedChainStringList.removeAll(selectedPresetName);
        }
    }

    pFocusedModel->setStringList(focusedChainStringList);
    saveChainPresetLists();
}

bool DlgPrefEffects::eventFilter(QObject* pChainList, QEvent* event) {
    if (event->type() == QEvent::FocusIn) {
        auto pListView = dynamic_cast<QListView*>(pChainList);
        if (!pListView) {
            return false;
        }
        m_pFocusedChainList = pListView;
        unfocusedChainList()->selectionModel()->reset();
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
