#include "widget/weffectchainpresetbutton.h"

#include <QCheckBox>
#include <QWidgetAction>

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
    for (const auto pChainPreset : m_pEffectsManager->getChainPresetManager()->getPresetsSorted()) {
        m_pMenu->addAction(pChainPreset->name(), [=]() {
            m_pEffectsManager->loadPresetToStandardChain(m_iChainNumber, pChainPreset);
        });
    }
    m_pMenu->addSeparator();
    m_pMenu->addAction(tr("Save preset"), this, &WEffectChainPresetButton::saveChainPreset);

    m_pMenu->addSeparator();
    for (int i = 0; i < 3; ++i) {
        const ParameterMap loadedParameters = m_pEffectsManager->getLoadedParameters(m_iChainNumber, i);
        const ParameterMap hiddenParameters = m_pEffectsManager->getHiddenParameters(m_iChainNumber, i);

        auto pEffectMenu = make_parented<QMenu>(m_pMenu);
        pEffectMenu->setTitle(tr("Effect") + " " + QString::number(i + 1));

        int numTypes = static_cast<int>(EffectManifestParameter::ParameterType::NUM_TYPES);
        for (int parameterTypeId = 0; parameterTypeId < numTypes; ++parameterTypeId) {
            const EffectManifestParameter::ParameterType parameterType =
                    static_cast<EffectManifestParameter::ParameterType>(parameterTypeId);
            for (const auto pParameter : loadedParameters.value(parameterType)) {
                auto pCheckbox = make_parented<QCheckBox>(pEffectMenu);
                pCheckbox->setChecked(true);
                pCheckbox->setText(pParameter->manifest()->name());
                auto handler = [this, pCheckbox{pCheckbox.get()}, i, pParameter] {
                    if (pCheckbox->isChecked()) {
                        m_pEffectsManager->showParameter(m_iChainNumber, i, pParameter);
                    } else {
                        m_pEffectsManager->hideParameter(m_iChainNumber, i, pParameter);
                    }
                };
                connect(pCheckbox.get(), &QCheckBox::stateChanged, this, handler);

                auto pAction = make_parented<QWidgetAction>(pEffectMenu);
                pAction->setDefaultWidget(pCheckbox.get());
                connect(pAction.get(), &QAction::triggered, this, handler);

                pEffectMenu->addAction(pAction.get());
            }

            for (const auto pParameter : hiddenParameters.value(parameterType)) {
                auto pCheckbox = make_parented<QCheckBox>(pEffectMenu);
                pCheckbox->setChecked(false);
                pCheckbox->setText(pParameter->manifest()->name());
                auto handler = [this, pCheckbox{pCheckbox.get()}, i, pParameter] {
                    if (pCheckbox->isChecked()) {
                        m_pEffectsManager->showParameter(m_iChainNumber, i, pParameter);
                    } else {
                        m_pEffectsManager->hideParameter(m_iChainNumber, i, pParameter);
                    }
                };
                connect(pCheckbox.get(), &QCheckBox::stateChanged, this, handler);

                auto pAction = make_parented<QWidgetAction>(pEffectMenu);
                pAction->setDefaultWidget(pCheckbox.get());
                connect(pAction.get(), &QAction::triggered, this, handler);

                pEffectMenu->addAction(pAction.get());
            }
            pEffectMenu->addSeparator();
        }
        pEffectMenu->addAction(tr("Save snapshot"), [=] {
            m_pEffectsManager->saveDefaultForEffect(m_iChainNumber, i);
        });
        m_pMenu->addMenu(pEffectMenu);
    }

    showMenu();
}

void WEffectChainPresetButton::saveChainPreset() {
    m_pEffectsManager->savePresetFromStandardEffectChain(m_iChainNumber);
}
