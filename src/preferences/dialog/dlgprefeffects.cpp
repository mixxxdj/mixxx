#include "preferences/dialog/dlgprefeffects.h"

#include "effects/effectsmanager.h"
#include "effects/effectmanifest.h"
#include "effects/effectsbackend.h"

DlgPrefEffects::DlgPrefEffects(QWidget* pParent,
        UserSettingsPointer pConfig,
        EffectsManager* pEffectsManager)
        : DlgPreferencePage(pParent),
          m_pConfig(pConfig),
          m_pEffectsManager(pEffectsManager),
          m_pChainPresetManager(pEffectsManager->getChainPresetManager()),
          m_pBackendManager(pEffectsManager->getBackendManager()) {
    setupUi(this);

    m_availableEffectsModel.resetFromEffectManager(pEffectsManager);
    for (auto& profile : m_availableEffectsModel.profiles()) {
        EffectManifestPointer pManifest = profile->pManifest;

        // Users are likely to have lots of external plugins installed and
        // many of them are useless for DJing. To avoid cluttering the list
        // shown in WEffectSelector, blacklist external plugins by default.
        bool defaultValue = (pManifest->backendType() == EffectBackendType::BuiltIn);
        bool visible = m_pConfig->getValue<bool>(ConfigKey("[Visible " + pManifest->backendName() + " Effects]",
                                                         pManifest->id()),
                defaultValue);
        profile->bIsVisible = visible;
        m_pEffectsManager->setEffectVisibility(pManifest, visible);
    }
    availableEffectsList->setModel(&m_availableEffectsModel);

    connect(availableEffectsList->selectionModel(),
            SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)),
            this,
            SLOT(availableEffectsListItemSelected(const QModelIndex&)));

    // Highlight first row
    availableEffectsList->selectRow(0);

    availableEffectsList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    availableEffectsList->setColumnWidth(1, 200);
    availableEffectsList->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

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

void DlgPrefEffects::slotUpdate() {
    clear();
    m_availableEffectsModel.resetFromEffectManager(m_pEffectsManager);

    // No chain preset is selected when the preferences are opened
    for (int i = 0; i < m_effectsLabels.size(); ++i) {
        m_effectsLabels[i]->setText(QString::number(i + 1) + ": ");
    }

    chainPresetExportButton->setEnabled(false);
    chainPresetRenameButton->setEnabled(false);
    chainPresetDeleteButton->setEnabled(false);
    chainPresetMoveUpButton->setEnabled(false);
    chainPresetMoveDownButton->setEnabled(false);

    if (!m_availableEffectsModel.isEmpty()) {
        availableEffectsList->selectRow(0);
    }
    loadChainPresetList();
}

void DlgPrefEffects::slotApply() {
    for (EffectProfilePtr profile : m_availableEffectsModel.profiles()) {
        EffectManifestPointer pManifest = profile->pManifest;
        m_pEffectsManager->setEffectVisibility(pManifest, profile->bIsVisible);

        // Effects from different backends can have same Effect IDs.
        // Add backend name to group to uniquely identify those effects.
        // Use untranslated value to keep the group language independent.
        m_pConfig->set(ConfigKey("[Visible " + pManifest->backendName() + " Effects]", pManifest->id()),
                ConfigValue(profile->bIsVisible));
    }

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

void DlgPrefEffects::availableEffectsListItemSelected(const QModelIndex& selected) {
    QString effectId = m_availableEffectsModel.data(selected, Qt::UserRole).toString();

    if (effectId == QVariant().toString())
        return;

    EffectManifestPointer pManifest = m_pBackendManager->getManifestFromUniqueId(effectId);

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
