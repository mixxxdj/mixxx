#include "preferences/dialog/dlgprefeffects.h"

#include "effects/effectsmanager.h"
#include "effects/effectmanifest.h"
#include "effects/effectsbackend.h"
// #include "preferences/effectsettingsmodel.h"

DlgPrefEffects::DlgPrefEffects(QWidget* pParent,
                               UserSettingsPointer pConfig,
                               EffectsManager* pEffectsManager)
        : DlgPreferencePage(pParent),
          m_pConfig(pConfig),
          m_pEffectsManager(pEffectsManager) {
    setupUi(this);

    m_pAvailableEffectsModel = new EffectSettingsModel();

    m_pAvailableEffectsModel->resetFromEffectManager(pEffectsManager);
    availableEffectsList->setModel(m_pAvailableEffectsModel);

    connect(availableEffectsList->selectionModel(),
            SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)),
            this,
            SLOT(availableEffectsListItemSelected(const QModelIndex&)));

    // Highlight first row
    availableEffectsList->selectRow(0);

    availableEffectsList->setColumnWidth(0, 50);
}

DlgPrefEffects::~DlgPrefEffects() {
    delete m_pAvailableEffectsModel;
}

void DlgPrefEffects::slotUpdate() {
    clear();
    m_pAvailableEffectsModel->resetFromEffectManager(m_pEffectsManager);

    if (!m_pAvailableEffectsModel->isEmpty()) {
        availableEffectsList->selectRow(0);
    }
}

void DlgPrefEffects::slotApply() {
    for (EffectProfilePtr profile : m_pAvailableEffectsModel->profiles()) {
        EffectManifest* pManifest = profile->getManifest();
        pManifest->setVisibility(profile->isVisible());
        m_pConfig->set(ConfigKey("[Visible Effects]", pManifest->id()), ConfigValue(profile->isVisible()));
    }
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

void DlgPrefEffects::availableEffectsListItemSelected(const QModelIndex& selected) {
    QString effectId = m_pAvailableEffectsModel->data(selected, Qt::UserRole).toString();

    if (effectId == QVariant().toString())
        return;

    EffectManifestPointer pManifest;
    EffectsBackend* pBackend;
    m_pEffectsManager->getEffectManifestAndBackend(effectId, &pManifest, &pBackend);

    effectName->setText(pManifest->name());
    effectAuthor->setText(pManifest->author());
    effectDescription->setText(pManifest->description());
    effectVersion->setText(pManifest->version());
    if (pBackend != NULL) {
        effectType->setText(pBackend->getName());
    } else {
        effectType->clear();
    }
}
