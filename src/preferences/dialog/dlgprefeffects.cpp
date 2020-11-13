#include "preferences/dialog/dlgprefeffects.h"

#include "effects/effectsmanager.h"
#include "effects/effectmanifest.h"
#include "effects/effectsbackend.h"

DlgPrefEffects::DlgPrefEffects(QWidget* pParent,
                               UserSettingsPointer pConfig,
                               EffectsManager* pEffectsManager)
        : DlgPreferencePage(pParent),
          m_pConfig(pConfig),
          m_pEffectsManager(pEffectsManager) {
    setupUi(this);

    m_availableEffectsModel.resetFromEffectManager(pEffectsManager);
    const QList<EffectProfilePtr> effectProfiles = m_availableEffectsModel.profiles();
    for (auto& profile : effectProfiles) {
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
    availableEffectsList->setTabKeyNavigation(false);

    connect(availableEffectsList->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &DlgPrefEffects::availableEffectsListItemSelected);

    // Highlight first row
    availableEffectsList->selectRow(0);

    availableEffectsList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    availableEffectsList->setColumnWidth(1, 200);
    availableEffectsList->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
}

DlgPrefEffects::~DlgPrefEffects() {
}

void DlgPrefEffects::slotUpdate() {
    clear();
    m_availableEffectsModel.resetFromEffectManager(m_pEffectsManager);

    if (!m_availableEffectsModel.isEmpty()) {
        availableEffectsList->selectRow(0);
    }

    bool effectAdoptMetaknobValue = m_pConfig->getValue(
            ConfigKey("[Effects]", "AdoptMetaknobValue"), true);
    radioButtonKeepMetaknobPosition->setChecked(effectAdoptMetaknobValue);
    radioButtonMetaknobLoadDefault->setChecked(!effectAdoptMetaknobValue);
}

void DlgPrefEffects::slotApply() {
    const QList<EffectProfilePtr> effectProfiles = m_availableEffectsModel.profiles();
    for (const EffectProfilePtr& profile : effectProfiles) {
        EffectManifestPointer pManifest = profile->pManifest;
        m_pEffectsManager->setEffectVisibility(pManifest, profile->bIsVisible);

        // Effects from different backends can have same Effect IDs.
        // Add backend name to group to uniquely identify those effects.
        // Use untranslated value to keep the group language independent.
        m_pConfig->set(ConfigKey("[Visible " + pManifest->backendName() + " Effects]", pManifest->id()),
                ConfigValue(profile->bIsVisible));
    }

    m_pConfig->set(ConfigKey("[Effects]", "AdoptMetaknobValue"),
            ConfigValue(radioButtonKeepMetaknobPosition->isChecked()));
}

void DlgPrefEffects::slotResetToDefaults() {
    radioButtonKeepMetaknobPosition->setChecked(true);

    slotUpdate();
}

void DlgPrefEffects::clear() {
    effectName->clear();
    effectAuthor->clear();
    effectDescription->clear();
    effectVersion->clear();
    effectType->clear();
}

void DlgPrefEffects::availableEffectsListItemSelected(const QModelIndex& selected) {
    QString effectId = m_availableEffectsModel.data(selected, Qt::UserRole).toString();

    EffectManifestPointer pManifest = m_pEffectsManager->getEffectManifest(effectId);
    if (!pManifest) {
        return;
    }

    effectName->setText(pManifest->name());
    effectAuthor->setText(pManifest->author());
    effectDescription->setText(pManifest->description());
    effectVersion->setText(pManifest->version());
    effectType->setText(pManifest->translatedBackendName());
}
