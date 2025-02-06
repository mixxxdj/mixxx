#include "preferences/dialog/dlgprefeffects.h"

#include <QFocusEvent>
#include <QMimeData>

#include "effects/backends/effectmanifest.h"
#include "effects/backends/effectsbackend.h"
#include "effects/effectsmanager.h"
#include "effects/presets/effectchainpreset.h"
#include "effects/presets/effectpreset.h"
#include "effects/visibleeffectslist.h"
#include "moc_dlgprefeffects.cpp"
#include "preferences/effectchainpresetlistmodel.h"
#include "preferences/effectmanifesttablemodel.h"

class EffectsManager;

DlgPrefEffects::DlgPrefEffects(QWidget* pParent,
        UserSettingsPointer pConfig,
        std::shared_ptr<EffectsManager> pEffectsManager)
        : DlgPreferencePage(pParent),
          m_pFocusedChainList(nullptr),
          m_pFocusedEffectList(nullptr),
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

    updateHideUnhideButtons();
    // TODO Use only one button, set text/icon depending on focused list view?
    connect(hideButton,
            &QPushButton::clicked,
            this,
            &DlgPrefEffects::slotHideUnhideEffect);
    connect(unhideButton,
            &QPushButton::clicked,
            this,
            &DlgPrefEffects::slotHideUnhideEffect);

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
    pTableView->verticalHeader()->hide();
    pTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    pTableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    pTableView->setDragDropMode(QAbstractItemView::DragDrop);
    // QTableView won't remove dragged items without this??
    pTableView->setDragDropOverwriteMode(false);
    pTableView->setTabKeyNavigation(false);
    pTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    pTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(pTableView->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &DlgPrefEffects::effectsTableItemSelected);
    pTableView->installEventFilter(this);
}

void DlgPrefEffects::setupChainListView(QListView* pListView) {
    auto pModel = make_parented<EffectChainPresetListModel>(pListView, m_pChainPresetManager);
    pListView->setModel(pModel);
    pListView->setDropIndicatorShown(true);
    pListView->setDragDropMode(QAbstractItemView::DragDrop);
    pListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    //TODO: prevent drops of duplicate items
    pListView->setDefaultDropAction(Qt::CopyAction);
    connect(pListView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &DlgPrefEffects::slotChainPresetSelectionChanged);
    pListView->installEventFilter(this);
}

void DlgPrefEffects::slotUpdate() {
    clearEffectInfo();

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
    updateHideUnhideButtons();

    // No chain preset is selected when the preferences are opened
    clearChainInfo();
    updateChainPresetButtons(0);

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
    auto* pModel = dynamic_cast<EffectChainPresetListModel*>(chainListView->model());
    m_pChainPresetManager->setPresetOrder(pModel->stringList());

    pModel = dynamic_cast<EffectChainPresetListModel*>(quickEffectListView->model());
    m_pChainPresetManager->setQuickEffectPresetOrder(pModel->stringList());
}

void DlgPrefEffects::slotResetToDefaults() {
    radioButtonKeepMetaknobPosition->setChecked(true);
    m_pChainPresetManager->resetToDefaults();

    slotUpdate();
}

void DlgPrefEffects::clearEffectInfo() {
    effectName->clear();
    effectAuthor->clear();
    effectDescription->clear();
    effectVersion->clear();
    effectType->clear();
}

void DlgPrefEffects::clearChainInfo() {
    for (int i = 0; i < m_effectsLabels.size(); ++i) {
        m_effectsLabels[i]->setText(QString::number(i + 1) + ": ");
    }
}

void DlgPrefEffects::updateChainPresetButtons(int selectedIndices) {
    // Allow Delete and Export of multiple presets
    chainPresetDeleteButton->setEnabled(selectedIndices > 0);
    chainPresetExportButton->setEnabled(selectedIndices > 0);
    // Enable Rename only for one preset
    chainPresetRenameButton->setEnabled(selectedIndices == 1);
}

void DlgPrefEffects::updateHideUnhideButtons(const QModelIndex& selected) {
    if (!selected.isValid() || m_pFocusedEffectList == nullptr) {
        hideButton->setEnabled(false);
        unhideButton->setEnabled(false);
        return;
    }
    bool enableHide = m_pFocusedEffectList == visibleEffectsTableView;
    hideButton->setEnabled(enableHide);
    unhideButton->setEnabled(!enableHide);
}

void DlgPrefEffects::loadChainPresetLists() {
    QStringList chainPresetNames;
    for (const auto& pChainPreset : m_pChainPresetManager->getPresetsSorted()) {
        // Don't show the empty '---' preset.
        // After pushing the changed preferences list back to the preset manager
        // it is re-added to the base list.
        if (pChainPreset->name() == kNoEffectString) {
            continue;
        }
        chainPresetNames << pChainPreset->name();
    }
    auto* pModel = dynamic_cast<EffectChainPresetListModel*>(chainListView->model());
    pModel->setStringList(chainPresetNames);

    QStringList quickEffectChainPresetNames;
    for (const auto& pChainPreset : m_pChainPresetManager->getQuickEffectPresetsSorted()) {
        // Same here, don't show the empty '---' preset.
        if (pChainPreset->name() == kNoEffectString) {
            continue;
        }
        quickEffectChainPresetNames << pChainPreset->name();
    }
    pModel = dynamic_cast<EffectChainPresetListModel*>(quickEffectListView->model());
    pModel->setStringList(quickEffectChainPresetNames);
}

void DlgPrefEffects::effectsTableItemSelected(const QModelIndex& selected) {
    // Clear the info box and return if the index is invalid, e.g. after clearCurrentIndex()
    // in eventFilter()
    if (!selected.isValid()) {
        clearEffectInfo();
        updateHideUnhideButtons();
        return;
    }
    const auto* pModel = static_cast<const EffectManifestTableModel*>(selected.model());
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
    updateHideUnhideButtons(selected);
}

void DlgPrefEffects::slotHideUnhideEffect() {
    auto* pSourceList = m_pFocusedEffectList;
    auto* pTargetList = unfocusedEffectList();
    VERIFY_OR_DEBUG_ASSERT(pSourceList && pTargetList) {
        return;
    }
    auto* pSelectionModel = pSourceList->selectionModel();
    if (!pSelectionModel || pSelectionModel->selectedRows().size() != 1) {
        return;
    }
    auto* pSourceModel = static_cast<EffectManifestTableModel*>(pSourceList->model());
    VERIFY_OR_DEBUG_ASSERT(pSourceModel) {
        return;
    }
    auto* pTargetModel = static_cast<EffectManifestTableModel*>(pTargetList->model());
    VERIFY_OR_DEBUG_ASSERT(pTargetModel) {
        return;
    }
    QModelIndex selIdx = pSelectionModel->selectedRows().first();
    EffectManifestPointer pManifest = pSourceModel->getList().at(selIdx.row());
    VERIFY_OR_DEBUG_ASSERT(pManifest) {
        return;
    }

    QMimeData* mimeData = new QMimeData;
    mimeData->setText(pManifest->uniqueId());
    // Append the selected effect to the target list
    if (!pTargetModel->dropMimeData(mimeData)) {
        return;
    }
    // Note the added item so we can remove it if necessary
    QModelIndex pMovedEffect = pTargetModel->index(pTargetModel->rowCount() - 1, 0);
    DEBUG_ASSERT(pMovedEffect.isValid());

    if (!pSourceModel->removeRows(selIdx.row(), 1, selIdx.parent())) {
        // If removing failed, undo add to target list
        pTargetModel->removeRows(pMovedEffect.row(), 1, pMovedEffect.parent());
    }
}

void DlgPrefEffects::slotChainPresetSelectionChanged(const QItemSelection& selected) {
    Q_UNUSED(selected);
    VERIFY_OR_DEBUG_ASSERT(m_pFocusedChainList) {
        return;
    }
    auto* pSelModel = m_pFocusedChainList->selectionModel();
    auto selIndices = pSelModel->selectedIndexes();

    updateChainPresetButtons(selIndices.count());

    // Clear the info box and return if the index is invalid, e.g. after clearCurrentIndex()
    // in eventFilter()
    if (selIndices.count() != 1) {
        clearChainInfo();
        return;
    }

    QString chainPresetName = selIndices.first().data().toString();
    EffectChainPresetPointer pChainPreset = m_pChainPresetManager->getPreset(chainPresetName);
    if (pChainPreset == nullptr || pChainPreset->isEmpty()) {
        return;
    }

    for (int i = 0; i < m_effectsLabels.size(); ++i) {
        EffectManifestPointer pManifest;
        if (i < pChainPreset->effectPresets().size()) {
            EffectPresetPointer pEffectPreset = pChainPreset->effectPresets().at(i);
            if (!pEffectPreset->isEmpty()) {
                pManifest = m_pBackendManager->getManifest(pEffectPreset);
            }
        }
        if (pManifest) {
            QString displayName = pManifest->name();
            // Code uses 0-indexed numbers; users see 1 indexed numbers
            m_effectsLabels[i]->setText(QString::number(i + 1) + ": " + displayName);
        } else {
            m_effectsLabels[i]->setText(QString::number(i + 1) + ": " + kNoEffectString);
        }
    }
}

void DlgPrefEffects::slotImportPreset() {
    if (m_pChainPresetManager->importPreset()) {
        loadChainPresetLists();
    }
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
    bool presetsRenamed = false;
    const auto& selectedIndices = m_pFocusedChainList->selectionModel()->selectedIndexes();
    for (const auto& index : selectedIndices) {
        const QString& selectedPresetName = m_pFocusedChainList->model()->data(index).toString();
        if (m_pChainPresetManager->renamePreset(selectedPresetName)) {
            presetsRenamed = true;
        }
    }
    if (presetsRenamed) {
        loadChainPresetLists();
    }
}

void DlgPrefEffects::slotDeletePreset() {
    // If the preset is in the focused list and the unfocused list, only remove
    // it from the focused list, but do not actually delete it so it remains in
    // the unfocused list. Only delete it if it is in one list but not the other.
    VERIFY_OR_DEBUG_ASSERT(m_pFocusedChainList) {
        return;
    }
    auto* pFocusedModel = dynamic_cast<EffectChainPresetListModel*>(
            m_pFocusedChainList->model());
    QStringList focusedChainStringList = pFocusedModel->stringList();

    QListView* pUnfocusedChainList = unfocusedChainList();
    VERIFY_OR_DEBUG_ASSERT(pUnfocusedChainList) {
        return;
    }
    auto* pUnfocusedModel = dynamic_cast<EffectChainPresetListModel*>(
            pUnfocusedChainList->model());
    auto unfocusedChainStringList = pUnfocusedModel->stringList();

    bool updateAndSavePresetLists = false;
    const auto& selectedIndices = m_pFocusedChainList->selectionModel()->selectedIndexes();
    for (const auto& index : selectedIndices) {
        QString selectedPresetName =
                m_pFocusedChainList->model()->data(index).toString();
        if (!unfocusedChainStringList.contains(selectedPresetName)) {
            if (m_pChainPresetManager->deletePreset(selectedPresetName)) {
                focusedChainStringList.removeAll(selectedPresetName);
                updateAndSavePresetLists = true;
            }
        } else {
            focusedChainStringList.removeAll(selectedPresetName);
            updateAndSavePresetLists = true;
        }
    }

    if (updateAndSavePresetLists) {
        pFocusedModel->setStringList(focusedChainStringList);
        saveChainPresetLists();
    }
}

bool DlgPrefEffects::eventFilter(QObject* object, QEvent* event) {
    if (event->type() == QEvent::FocusIn) {
        // Allow selection only in either of the effects/chains lists at a time
        // to clarify which effect/chain the info below refers to.
        // The method to update the info box for the new selection is the same
        // for both the Chains and Effects tab:
        // * clear selection in adjacent view
        // * restore previous selection (select first item if none was selected)
        //   which updates the info box via 'currentRowChanged' signals
        auto* pChainList = qobject_cast<QListView*>(object);
        auto* pEffectList = qobject_cast<QTableView*>(object);
        // Restore previous selection only if focus was changed with keyboard.
        // For mouse clicks, that procedure would select the wrong index.
        QFocusEvent* focEv = static_cast<QFocusEvent*>(event);
        bool keyboardFocusIn = false;
        if (focEv->reason() == Qt::TabFocusReason ||
                focEv->reason() == Qt::BacktabFocusReason) {
            keyboardFocusIn = true;
        }

        if (pChainList) {
            m_pFocusedChainList = pChainList;
            unfocusedChainList()->selectionModel()->clearSelection();
            if (keyboardFocusIn) {
                QModelIndex currIndex = m_pFocusedChainList->selectionModel()->currentIndex();
                if (!currIndex.isValid()) {
                    currIndex = m_pFocusedChainList->model()->index(0, 0);
                }
                m_pFocusedChainList->selectionModel()->clearCurrentIndex();
                m_pFocusedChainList->selectionModel()->setCurrentIndex(
                        currIndex,
                        QItemSelectionModel::ClearAndSelect);
            }
        } else if (pEffectList) {
            m_pFocusedEffectList = pEffectList;
            unfocusedEffectList()->selectionModel()->clearSelection();
            if (keyboardFocusIn) {
                QModelIndex currIndex = m_pFocusedEffectList->currentIndex();
                if (!currIndex.isValid()) {
                    currIndex = pEffectList->model()->index(0, 0);
                }
                pEffectList->selectionModel()->clearCurrentIndex();
                pEffectList->selectRow(currIndex.row());
            }
        }
    }
    return DlgPreferencePage::eventFilter(object, event);
}

QListView* DlgPrefEffects::unfocusedChainList() {
    if (m_pFocusedChainList == chainListView) {
        return quickEffectListView;
    } else {
        return chainListView;
    }
}

QTableView* DlgPrefEffects::unfocusedEffectList() {
    if (m_pFocusedEffectList == visibleEffectsTableView) {
        return hiddenEffectsTableView;
    } else {
        return visibleEffectsTableView;
    }
}
