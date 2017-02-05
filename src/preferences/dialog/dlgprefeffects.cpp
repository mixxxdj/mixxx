#include "preferences/dialog/dlgprefeffects.h"

#include "effects/effectsmanager.h"

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
    const QList<EffectManifest> availableEffectManifests = m_pEffectsManager->getAvailableEffectManifests();

    for (const auto& manifest : availableEffectManifests) {
        QListWidgetItem* pItem = new QListWidgetItem();
        pItem->setData(Qt::UserRole, manifest.id());
        pItem->setText(manifest.displayName());
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
    QPair<EffectManifest, EffectsBackend*> manifestAndBackend =
            m_pEffectsManager->getEffectManifestAndBackend(effectId);

    const EffectManifest& manifest = manifestAndBackend.first;
    effectName->setText(manifest.name());
    effectAuthor->setText(manifest.author());
    effectDescription->setText(manifest.description());
    effectVersion->setText(manifest.version());
    if (manifestAndBackend.second != NULL) {
        effectType->setText(manifestAndBackend.second->getName());
    } else {
        effectType->clear();
    }
}
