#include "widget/weffectchainpresetbutton.h"

#include "widget/effectwidgetutils.h"

WEffectChainPresetButton::WEffectChainPresetButton(QWidget* parent, EffectsManager* pEffectsManager)
        : QPushButton(parent),
          WBaseWidget(this),
          m_pEffectsManager(pEffectsManager),
          m_pMenu(make_parented<QMenu>(new QMenu(this))) {
    setMenu(m_pMenu.get());
    connect(this,
            &QPushButton::pressed,
            this,
            &WEffectChainPresetButton::populateMenu);
}

void WEffectChainPresetButton::setup(const QDomNode& node, const SkinContext& context) {
    m_iChainNumber = EffectWidgetUtils::getEffectUnitNumberFromNode(node, context);
    // TODO: set icon
}

void WEffectChainPresetButton::populateMenu() {
    m_pMenu->clear();
    for (const auto pChainPreset : m_pEffectsManager->getAvailableChainPresets()) {
        m_pMenu->addAction(pChainPreset->name(), [=]() {
            m_pEffectsManager->loadPresetToStandardChain(m_iChainNumber, pChainPreset);
        });
    }
    m_pMenu->addSeparator();
    m_pMenu->addAction(tr("Save preset"), this, &WEffectChainPresetButton::saveChainPreset);
    showMenu();
}

void WEffectChainPresetButton::saveChainPreset() {
    m_pEffectsManager->savePresetFromStandardEffectChain(m_iChainNumber);
}
