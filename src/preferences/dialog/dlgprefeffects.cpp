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

    connect(availableEffectsList,
            SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
            this,
            SLOT(slotEffectSelected(QListWidgetItem*, QListWidgetItem*)));
}

void DlgPrefEffects::slotUpdate() {
    clear();
    const QList<EffectManifestPointer> availableEffectManifests =
            m_pEffectsManager->getAvailableEffectManifests();

    for (const auto& pManifest : availableEffectManifests) {
        QListWidgetItem* pItem = new QListWidgetItem();
        pItem->setData(Qt::UserRole, pManifest->id());
        pItem->setText(pManifest->displayName());
        availableEffectsList->addItem(pItem);
    }

    if (!availableEffectManifests.isEmpty()) {
        availableEffectsList->setCurrentRow(0);
    }
}

void DlgPrefEffects::slotApply() {
    // Nothing to apply.
}

void DlgPrefEffects::slotResetToDefaults() {
    // Nothing to reset.
}

void DlgPrefEffects::clear() {
    availableEffectsList->clear();
    effectName->clear();
    effectAuthor->clear();
    effectDescription->clear();
    effectVersion->clear();
    effectType->clear();
}

void DlgPrefEffects::slotEffectSelected(QListWidgetItem* pCurrent,
                                        QListWidgetItem* pPrevious) {
    Q_UNUSED(pPrevious);
    if (pCurrent == NULL) {
        return;
    }
    QString effectId = pCurrent->data(Qt::UserRole).toString();
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
