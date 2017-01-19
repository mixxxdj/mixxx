#include <QtDebug>

#include "widget/weffectselector.h"

#include "effects/effectsmanager.h"
#include "widget/effectwidgetutils.h"

namespace {
    const QString nullEffectId = "NULL";
}

WEffectSelector::WEffectSelector(QWidget* pParent, EffectsManager* pEffectsManager)
        : QComboBox(pParent),
          WBaseWidget(pParent),
          m_pEffectsManager(pEffectsManager) {

    setInsertPolicy(QComboBox::InsertAlphabetically);
    // TODO(xxx): filter out blacklisted effects
    // https://bugs.launchpad.net/mixxx/+bug/1653140
    QList<QPair<QString,QString> > idNamePairs = m_pEffectsManager->getEffectShortNamesFiltered(nullptr);
    for (const auto& effectPair : idNamePairs) {
        addItem(effectPair.second, QVariant(effectPair.first));
    }
    //: Displayed when no effect is loaded
    addItem(tr("None"), QVariant(nullEffectId));
}

void WEffectSelector::setup(const QDomNode& node, const SkinContext& context) {
    // EffectWidgetUtils propagates NULLs so this is all safe.
    m_pRack = EffectWidgetUtils::getEffectRackFromNode(
            node, context, m_pEffectsManager);
    m_pChainSlot = EffectWidgetUtils::getEffectChainSlotFromNode(
            node, context, m_pRack);
    m_pEffectSlot = EffectWidgetUtils::getEffectSlotFromNode(
            node, context, m_pChainSlot);

    if (m_pEffectSlot != nullptr) {
        connect(m_pEffectSlot.data(), SIGNAL(updated()),
                this, SLOT(slotEffectUpdated()));
        connect(this, SIGNAL(currentIndexChanged(int)),
                this, SLOT(slotEffectSelected(int)));
        slotEffectUpdated();
    } else {
        SKIN_WARNING(node, context)
                << "EffectSelector node could not attach to effect slot.";
    }
}

void WEffectSelector::slotEffectSelected(int newIndex) {
    const QString id = itemData(newIndex).toString();
    EffectPointer pEffect;

    bool loadNew = false;
    if (m_pEffectSlot == nullptr || m_pEffectSlot->getEffect() == nullptr) {
        loadNew = true;
    } else if (id != m_pEffectSlot->getEffect()->getManifest().id()) {
        loadNew = true;
    }

    if (loadNew) {
        EffectChainPointer pChain = m_pChainSlot->getEffectChain();
        if (id == nullEffectId) {
            pEffect = EffectPointer();
        } else {
            pEffect = m_pEffectsManager->instantiateEffect(id);
        }
        pChain->replaceEffect(m_pEffectSlot->getEffectSlotNumber(), pEffect);
    }
}

void WEffectSelector::slotEffectUpdated() {
    QString description = tr("No effect loaded.");
    int newIndex;

    if (m_pEffectSlot != nullptr) {
        EffectPointer pEffect = m_pEffectSlot->getEffect();
        if (pEffect != nullptr) {
            const EffectManifest& manifest = pEffect->getManifest();
            newIndex = findData(QVariant(manifest.id()));

            //: %1 = effect name; %2 = effect description
            description = tr("%1: %2").arg(manifest.name(), manifest.description());
        } else {
            newIndex = findData(QVariant(nullEffectId));
        }
    } else {
        newIndex = findData(QVariant(nullEffectId));
    }

    setBaseTooltip(description);
    if (newIndex != -1 && newIndex != currentIndex()) {
        setCurrentIndex(newIndex);
    }
}
