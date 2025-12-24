#include "widget/weffectchainpresetbutton.h"

#include <QWidgetAction>

#include "effects/effectparameter.h"
#include "effects/effectparameterslotbase.h"
#include "effects/presets/effectchainpreset.h"
#include "effects/presets/effectpreset.h"
#include "effects/presets/effectpresetmanager.h"
#include "moc_weffectchainpresetbutton.cpp"
#include "util/parented_ptr.h"
#include "widget/effectwidgetutils.h"
#include "widget/wmenucheckbox.h"

WEffectChainPresetButton::WEffectChainPresetButton(QWidget* parent, EffectsManager* pEffectsManager)
        : QPushButton(parent),
          WBaseWidget(this),
          m_pEffectsManager(pEffectsManager),
          m_pChainPresetManager(pEffectsManager->getChainPresetManager()),
          m_pMenu(make_parented<QMenu>(new QMenu(this))) {
    setMenu(m_pMenu.get());
    connect(m_pChainPresetManager.data(),
            &EffectChainPresetManager::effectChainPresetListUpdated,
            this,
            &WEffectChainPresetButton::populateMenu);
    setFocusPolicy(Qt::NoFocus);
}

void WEffectChainPresetButton::setup(const QDomNode& node, const SkinContext& context) {
    m_pChain = EffectWidgetUtils::getEffectChainFromNode(
            node, context, m_pEffectsManager);
    VERIFY_OR_DEBUG_ASSERT(m_pChain) {
        SKIN_WARNING(node,
                context,
                QStringLiteral("EffectChainPresetButton node could not attach "
                               "to effect chain"));
        return;
    }
    connect(m_pChain.get(),
            &EffectChain::chainPresetChanged,
            this,
            &WEffectChainPresetButton::populateMenu);
    for (const auto& pEffectSlot : m_pChain->getEffectSlots()) {
        connect(pEffectSlot.data(),
                &EffectSlot::effectChanged,
                this,
                &WEffectChainPresetButton::populateMenu);
        connect(pEffectSlot.data(),
                &EffectSlot::parametersChanged,
                this,
                &WEffectChainPresetButton::populateMenu);
    }
    m_pMenu->setToolTipsVisible(true);
    populateMenu();
}

void WEffectChainPresetButton::populateMenu() {
    m_pMenu->clear();

    // Chain preset items
    const EffectsBackendManagerPointer pBackendManager = m_pEffectsManager->getBackendManager();
    bool presetIsReadOnly = true;
    QStringList effectNames;
    for (const auto& pChainPreset : m_pChainPresetManager->getPresetsSorted()) {
        QString title = pChainPreset->name();
        if (title == m_pChain->presetName()) {
            title = QChar(0x2713) + // CHECK MARK
                    QChar(' ') + title;
            presetIsReadOnly = pChainPreset->isReadOnly();
        }
        QString tooltip =
                QStringLiteral("<b>") + pChainPreset->name() + QStringLiteral("</b>");
        for (const auto& pEffectPreset : pChainPreset->effectPresets()) {
            if (!pEffectPreset->isEmpty()) {
                EffectManifestPointer pManifest = pBackendManager->getManifest(pEffectPreset);
                if (pManifest) {
                    effectNames.append(pManifest->name());
                }
            }
        }
        if (effectNames.size() > 1) {
            tooltip.append("<br/>");
            tooltip.append(effectNames.join("<br/>"));
        }
        effectNames.clear();
        parented_ptr<QAction> pAction = make_parented<QAction>(title, this);
        connect(pAction,
                &QAction::triggered,
                this,
                [this, pChainPreset]() {
                    m_pChain->loadChainPreset(pChainPreset);
                });
        pAction->setToolTip(tooltip);
        m_pMenu->addAction(pAction);
    }
    m_pMenu->addSeparator();
    // This prevents showing the Update button for the empty '---' preset, in case
    // WEffectChainPresetButton and effect slot controls of a QuickEffect chain are
    // exposed in a custom skin.
    if (!presetIsReadOnly) {
        m_pMenu->addAction(tr("Update Preset"), this, [this]() {
            m_pChainPresetManager->updatePreset(m_pChain);
        });
    }
    if (!presetIsReadOnly && !m_pChain->presetName().isEmpty()) {
        m_pMenu->addAction(tr("Rename Preset"), this, [this]() {
            m_pChainPresetManager->renamePreset(m_pChain->presetName());
        });
    }
    m_pMenu->addAction(tr("Save As New Preset..."), this, [this]() {
        m_pChainPresetManager->savePresetAndReload(m_pChain);
    });

    m_pMenu->addSeparator();

    // Effect parameter hiding/showing and saving snapshots
    // TODO: get number of currently visible effect slots from skin
    for (int effectSlotIndex = 0; effectSlotIndex < 3; effectSlotIndex++) {
        auto pEffectSlot = m_pChain->getEffectSlots().at(effectSlotIndex);

        const QString effectSlotNumPrefix(QString::number(effectSlotIndex + 1) + ": ");
        auto pManifest = pEffectSlot->getManifest();
        if (pManifest == nullptr) {
            m_pMenu->addAction(effectSlotNumPrefix + kNoEffectString);
            continue;
        }

        auto pEffectMenu = make_parented<QMenu>(
                effectSlotNumPrefix + pEffectSlot->getManifest()->displayName(),
                m_pMenu);

        int numTypes = static_cast<int>(EffectManifestParameter::ParameterType::NumTypes);
        for (int parameterTypeId = 0; parameterTypeId < numTypes; ++parameterTypeId) {
            const EffectManifestParameter::ParameterType parameterType =
                    static_cast<EffectManifestParameter::ParameterType>(parameterTypeId);

            const auto& loadedParameters = pEffectSlot->getLoadedParameters().value(parameterType);
            const auto& hiddenParameters = pEffectSlot->getHiddenParameters().value(parameterType);
            for (const auto& parameters : {loadedParameters, hiddenParameters}) {
                for (const auto& pParameter : parameters) {
                    auto pCheckbox = make_parented<WMenuCheckBox>(pEffectMenu);
                    pCheckbox->setChecked(true);
                    pCheckbox->setText(pParameter->manifest()->name());
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
                    auto handler = [pEffectSlot, pParameter](Qt::CheckState state) {
                        if (state == Qt::Checked) {
#else
                    auto handler = [pEffectSlot, pParameter](int state) {
                        if (static_cast<bool>(state)) {
#endif
                            pEffectSlot->showParameter(pParameter);
                        } else {
                            pEffectSlot->hideParameter(pParameter);
                        }
                    };
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
                    connect(pCheckbox.get(), &QCheckBox::checkStateChanged, this, handler);
#else
                    connect(pCheckbox.get(), &QCheckBox::stateChanged, this, handler);
#endif

                    auto pAction = make_parented<QWidgetAction>(pEffectMenu);
                    pAction->setDefaultWidget(pCheckbox.get());

                    pEffectMenu->addAction(pAction.get());
                }
            }
        }
        pEffectMenu->addSeparator();

        pEffectMenu->addAction(tr("Save snapshot"), this, [this, pEffectSlot] {
            m_pEffectsManager->getEffectPresetManager()->saveDefaultForEffect(pEffectSlot);
        });
        m_pMenu->addMenu(pEffectMenu);
    }
}

bool WEffectChainPresetButton::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }

    return QPushButton::event(pEvent);
}
