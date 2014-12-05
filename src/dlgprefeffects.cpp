#include "dlgprefeffects.h"

#include "effects/effectsmanager.h"

DlgPrefEffects::DlgPrefEffects(QWidget* pParent,
                               ConfigObject<ConfigValue>* pConfig,
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
    QStringList effectIds = m_pEffectsManager->getAvailableEffects();

    foreach (QString id, effectIds) {
        addEffectToList(id);
    }

    if (!effectIds.isEmpty()) {
        availableEffectsList->setCurrentRow(0);
    }
}

void DlgPrefEffects::addEffectToList(const QString& effectId) {
    QPair<EffectManifest, EffectsBackend*> manifestAndBackend =
            m_pEffectsManager->getEffectManifestAndBackend(effectId);

    QListWidgetItem* pItem = new QListWidgetItem();
    pItem->setText(manifestAndBackend.first.name());
    pItem->setData(Qt::UserRole, effectId);
    availableEffectsList->addItem(pItem);
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
