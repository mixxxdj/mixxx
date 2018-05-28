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

    m_pAvailableEffectsModel = new EffectSettingsModel();

    m_pAvailableEffectsModel->resetFromEffectManager(pEffectsManager);
    for (auto profile : m_pAvailableEffectsModel->profiles()) {
        EffectManifestPointer pManifest = profile->pManifest;

        // Users are likely to have lots of external plugins installed and 
        // many of them are useless for DJing. To avoid cluttering the list 
        // shown in WEffectSelector, blacklist external plugins by default.
        bool defaultValue = (pManifest->backendType() == EffectBackendType::BuiltIn);
        bool visible = m_pConfig->getValue<bool>(ConfigKey("[Visible Effects]", 
                                                 pManifest->id()), defaultValue);
        profile->bIsVisible = visible;
        m_pEffectsManager->setEffectVisibility(pManifest, visible);
    }
    availableEffectsList->setModel(m_pAvailableEffectsModel);

    connect(availableEffectsList->selectionModel(),
            SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)),
            this,
            SLOT(availableEffectsListItemSelected(const QModelIndex&)));

    // Highlight first row
    availableEffectsList->selectRow(0);

  #if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    availableEffectsList->horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);
    availableEffectsList->setColumnWidth(1, 200);
    availableEffectsList->horizontalHeader()->setResizeMode(2, QHeaderView::ResizeToContents);
  #else
    availableEffectsList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    availableEffectsList->setColumnWidth(1, 200);
    availableEffectsList->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
  #endif // QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
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
        EffectManifestPointer pManifest = profile->pManifest;
        m_pEffectsManager->setEffectVisibility(pManifest, profile->bIsVisible);
        m_pConfig->set(ConfigKey("[Visible Effects]", pManifest->id()), ConfigValue(profile->bIsVisible));
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
