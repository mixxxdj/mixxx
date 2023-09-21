#include "preferences/dialog/dlgprefmixer.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QPainterPath>
#include <QStandardItemModel>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "defs_urls.h"
#include "effects/backends/builtin/biquadfullkilleqeffect.h"
#include "effects/backends/builtin/filtereffect.h"
#include "effects/chains/equalizereffectchain.h"
#include "effects/chains/quickeffectchain.h"
#include "effects/effectslot.h"
#include "engine/enginexfader.h"
#include "mixer/playermanager.h"
#include "moc_dlgprefmixer.cpp"
#include "util/math.h"
#include "util/rescaler.h"

namespace {
const QString kEffectForGroupPrefix = QStringLiteral("EffectForGroup_");
const QString kEffectGroupForMaster = QStringLiteral("EffectForGroup_[Master]");
const QString kMainEQParameterKey = QStringLiteral("EffectForGroup_[Master]_parameter");
const ConfigKey kEnableEqsKey = ConfigKey(kMixerProfile, QStringLiteral("EnableEQs"));
const ConfigKey kEqsOnlyKey = ConfigKey(kMixerProfile, QStringLiteral("EQsOnly"));
const ConfigKey kSingleEqKey = ConfigKey(kMixerProfile, QStringLiteral("SingleEQEffect"));
const ConfigKey kEqAutoResetKey = ConfigKey(kMixerProfile, QStringLiteral("EqAutoReset"));
const ConfigKey kGainAutoResetKey = ConfigKey(kMixerProfile, QStringLiteral("GainAutoReset"));
const QString kDefaultEqId = BiquadFullKillEQEffect::getId() + " " +
        EffectsBackend::backendTypeToString(EffectBackendType::BuiltIn);
const QString kDefaultQuickEffectChainName = FilterEffect::getManifest()->name();
const QString kDefaultMainEqId = QString();

const ConfigKey kHighEqFreqKey = ConfigKey(kMixerProfile, kHighEqFrequency);
const ConfigKey kHighEqFreqPreciseKey =
        ConfigKey(kMixerProfile, QStringLiteral("HiEQFrequencyPrecise"));
const ConfigKey kLowEqFreqKey = ConfigKey(kMixerProfile, kLowEqFrequency);
const ConfigKey kLowEqFreqPreciseKey =
        ConfigKey(kMixerProfile, QStringLiteral("LoEQFrequencyPrecise"));

const ConfigKey kXfaderModeKey = ConfigKey(EngineXfader::kXfaderConfigKey,
        QStringLiteral("xFaderMode"));
const ConfigKey kXfaderCurveKey = ConfigKey(EngineXfader::kXfaderConfigKey,
        QStringLiteral("xFaderCurve"));
const ConfigKey kXfaderCalibrationKey = ConfigKey(EngineXfader::kXfaderConfigKey,
        QStringLiteral("xFaderCalibration"));
const ConfigKey kXfaderReverseKey = ConfigKey(EngineXfader::kXfaderConfigKey,
        QStringLiteral("xFaderReverse"));

constexpr int kFrequencyUpperLimit = 20050;
constexpr int kFrequencyLowerLimit = 16;

constexpr int kXfaderGridHLines = 3;
constexpr int kXfaderGridVLines = 5;

bool isMixingEQ(EffectManifest* pManifest) {
    return pManifest->isMixingEQ();
}

bool isMainEQ(EffectManifest* pManifest) {
    return pManifest->isMainEQ();
}
} // anonymous namespace

DlgPrefMixer::DlgPrefMixer(
        QWidget* pParent,
        std::shared_ptr<EffectsManager> pEffectsManager,
        UserSettingsPointer pConfig)
        : DlgPreferencePage(pParent),
          m_pConfig(pConfig),
          m_xFaderMode(MIXXX_XFADER_ADDITIVE),
          m_transform(EngineXfader::kTransformDefault),
          m_cal(0.0),
          m_mode(kXfaderModeKey),
          m_curve(kXfaderCurveKey),
          m_calibration(kXfaderCalibrationKey),
          m_reverse(kXfaderReverseKey),
          m_crossfader("[Master]", "crossfader"),
          m_xFaderReverse(false),
          m_COLoFreq(kLowEqFreqKey),
          m_COHiFreq(kHighEqFreqKey),
          m_lowEqFreq(0.0),
          m_highEqFreq(0.0),
          m_pChainPresetManager(pEffectsManager->getChainPresetManager()),
          m_pEffectsManager(pEffectsManager),
          m_pBackendManager(pEffectsManager->getBackendManager()),
          m_pNumDecks(make_parented<ControlProxy>(QStringLiteral("[App]"),
                  QStringLiteral("num_decks"),
                  this)),
          m_ignoreEqQuickEffectBoxSignals(false),
          m_singleEq(true),
          m_eqEffectsOnly(true),
          m_eqAutoReset(false),
          m_gainAutoReset(false),
          m_eqBypass(false),
          m_initializing(true) {
    setupUi(this);

    // Update the crossfader curve graph and other settings when the
    // crossfader mode is changed or the slider is moved.
    connect(SliderXFader,
            QOverload<int>::of(&QSlider::valueChanged),
            this,
            &DlgPrefMixer::slotUpdateXFader);
    connect(SliderXFader, &QSlider::sliderMoved, this, &DlgPrefMixer::slotUpdateXFader);
    connect(SliderXFader, &QSlider::sliderReleased, this, &DlgPrefMixer::slotUpdateXFader);
    connect(radioButtonAdditive, &QRadioButton::clicked, this, &DlgPrefMixer::slotUpdateXFader);
    connect(radioButtonConstantPower,
            &QRadioButton::clicked,
            this,
            &DlgPrefMixer::slotUpdateXFader);

    // Don't allow the xfader graph getting keyboard focus
    graphicsViewXfader->setFocusPolicy(Qt::NoFocus);

    // EQ shelf sliders
    connect(SliderHiEQ, &QSlider::valueChanged, this, &DlgPrefMixer::slotHiEqSliderChanged);
    connect(SliderHiEQ, &QSlider::sliderMoved, this, &DlgPrefMixer::slotHiEqSliderChanged);
    connect(SliderHiEQ, &QSlider::sliderReleased, this, &DlgPrefMixer::slotHiEqSliderChanged);

    connect(SliderLoEQ, &QSlider::valueChanged, this, &DlgPrefMixer::slotLoEqSliderChanged);
    connect(SliderLoEQ, &QSlider::sliderMoved, this, &DlgPrefMixer::slotLoEqSliderChanged);
    connect(SliderLoEQ, &QSlider::sliderReleased, this, &DlgPrefMixer::slotLoEqSliderChanged);

    connect(CheckBoxEqAutoReset,
            &QCheckBox::toggled,
            this,
            &DlgPrefMixer::slotEqAutoResetToggled);
    connect(CheckBoxGainAutoReset,
            &QCheckBox::toggled,
            this,
            &DlgPrefMixer::slotGainAutoResetToggled);
    connect(CheckBoxBypass,
            &QCheckBox::toggled,
            this,
            &DlgPrefMixer::slotBypassEqToggled);

    connect(CheckBoxEqOnly,
            &QCheckBox::toggled,
            this,
            &DlgPrefMixer::slotEqOnlyToggled);

    connect(CheckBoxSingleEqEffect,
            &QCheckBox::toggled,
            this,
            &DlgPrefMixer::slotSingleEqToggled);

    // Update the QuickEffect selectors when the effect list was changed
    // in Effects preferences
    connect(m_pChainPresetManager.data(),
            &EffectChainPresetManager::quickEffectChainPresetListUpdated,
            this,
            &DlgPrefMixer::slotPopulateQuickEffectSelectors);

    setUpMainEQ();

    // Update only after all settings are loaded, except EQs and QuickEffecs.
    // slotNumDecksChanged() needs the correct state of 'Same EQ for all decks".
    slotUpdate();

    // Add drop down lists for current decks and connect to num_decks control
    // so new lists are added if new decks are added.
    m_pNumDecks->connectValueChanged(this, &DlgPrefMixer::slotNumDecksChanged);
    slotNumDecksChanged(m_pNumDecks->get());

    setScrollSafeGuard(SliderXFader);
    setScrollSafeGuard(SliderHiEQ);
    setScrollSafeGuard(SliderLoEQ);
    setScrollSafeGuard(comboBoxMainEq);

    // This applies the Main EQ and saves default values of previously missing
    // ConfigKeys. EQ/QuickEffects are already applied by slotNumDecksChanged().
    slotApply();

    m_initializing = false;
}

// Create EQ & QuickEffect selectors and deck label for each added deck
void DlgPrefMixer::slotNumDecksChanged(double numDecks) {
    while (m_deckEqEffectSelectors.size() < static_cast<int>(numDecks)) {
        int deckNo = m_deckEqEffectSelectors.size() + 1;
        QString deckGroup = PlayerManager::groupForDeck(deckNo - 1);

        auto pLabel = make_parented<QLabel>(QObject::tr("Deck %1").arg(deckNo), this);

        // Create the EQ selector //////////////////////////////////////////////
        auto pEqComboBox = make_parented<QComboBox>(this);
        setScrollSafeGuard(pEqComboBox);
        m_deckEqEffectSelectors.append(pEqComboBox);

        // Load EQ from from mixxx.cfg, add and select it in the combobox(es).
        // If the ConfigKey exists the EQ is loaded. If the value is empty no
        // EQ was selected ('---').
        // If the key doesn't exist the default EQ is loaded.
        QString configuredEffect = m_pConfig->getValue<QString>(
                ConfigKey(kMixerProfile, kEffectForGroupPrefix + deckGroup),
                kDefaultEqId);
        const EffectManifestPointer pEQManifest =
                m_pBackendManager->getManifestFromUniqueId(configuredEffect);
        // We just add the one required item to the box so applyDeckEQs() can load it.
        if (pEQManifest) {
            pEqComboBox->addItem(pEQManifest->name(), QVariant(pEQManifest->uniqueId()));
        } else {
            // the previous EQ was 'none' or its uid can't be found
            pEqComboBox->addItem(kNoEffectString);
        }
        pEqComboBox->setCurrentIndex(0);

        connect(pEqComboBox,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                this,
                &DlgPrefMixer::slotEQEffectSelectionChanged);

        // Create the QuickEffect selector /////////////////////////////////////
        auto pQuickEffectComboBox = make_parented<QComboBox>(this);
        setScrollSafeGuard(pQuickEffectComboBox);
        m_deckQuickEffectSelectors.append(pQuickEffectComboBox);
        connect(pQuickEffectComboBox,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                this,
                &DlgPrefMixer::slotQuickEffectSelectionChanged);
        // Change the QuickEffect when it was changed in WEffectChainPresetSelector
        // or with controllers
        EffectChainPointer pChain = m_pEffectsManager->getQuickEffectChain(deckGroup);
        DEBUG_ASSERT(pChain);
        // TODO(xxx) Connecting the signal to a lambda that capture the parented_ptr
        // pQuickEffectComboBox and sets the combobox index causes a crash in
        // applyQuickEffects() even though the signal hasn_t been emitted, yet.
        // Hence we just capture the deck group and the new preset's name and set
        // the index in a separate slot for now.
        connect(pChain.data(),
                &EffectChain::chainPresetChanged,
                this,
                [this, deckGroup](const QString& name) {
                    slotQuickEffectChangedOnDeck(deckGroup, name);
                });

        // Add the new widgets
        gridLayout_3->addWidget(pLabel, deckNo, 0);
        gridLayout_3->addWidget(pEqComboBox, deckNo, 1);
        gridLayout_3->addWidget(pQuickEffectComboBox, deckNo, 2);
        gridLayout_3->addItem(
                new QSpacerItem(
                        40, 1, QSizePolicy::Expanding, QSizePolicy::Minimum),
                deckNo,
                3,
                1,
                1);
    }
    applyDeckEQs();

    // This also selects all currently loaded EQs and QuickEffects
    slotPopulateDeckEqSelectors();
    slotPopulateQuickEffectSelectors();

    // Ensure all newly created but unneeded widgets are hidden
    slotSingleEqToggled(m_singleEq);
}

void DlgPrefMixer::slotPopulateDeckEqSelectors() {
    m_ignoreEqQuickEffectBoxSignals = true; // prevents a recursive call

    const QList<EffectManifestPointer> pManifestList = getDeckEqManifests();
    for (int deck = 0; deck < m_deckEqEffectSelectors.size(); deck++) {
        auto* pBox = m_deckEqEffectSelectors[deck];
        // Populate comboboxes with all available effects
        // Get currently loaded EQ effect
        auto pChainSlot = m_pEffectsManager->getEqualizerEffectChain(
                PlayerManager::groupForDeck(deck));
        DEBUG_ASSERT(pChainSlot);
        auto pEffectSlot = pChainSlot->getEffectSlot(0);
        DEBUG_ASSERT(pEffectSlot);
        const EffectManifestPointer pLoadedManifest =
                pEffectSlot->getManifest();

        pBox->clear();
        // Add empty item at the top (no effect)
        pBox->addItem(kNoEffectString);
        int currentIndex = 0; // store it as default selection
        for (const auto& pManifest : pManifestList) {
            pBox->addItem(pManifest->name(), QVariant(pManifest->uniqueId()));
            int i = pBox->count() - 1;
            // <b> makes the effect name bold. Also, like <span> it serves as hack
            // to get Qt to treat the string as rich text so it automatically wraps long lines.
            pBox->setItemData(i,
                    QVariant(QStringLiteral("<b>%1</b><br/>%2")
                                     .arg(pManifest->name(),
                                             pManifest->description())),
                    Qt::ToolTipRole);
            if (pLoadedManifest &&
                    pLoadedManifest.data() == pManifest.data()) {
                currentIndex = i;
            }
        }
        if (pLoadedManifest && currentIndex == 0) {
            // Current selection is not part of the new list so we need to add it
            pBox->addItem(pLoadedManifest->displayName(),
                    QVariant(pLoadedManifest->uniqueId()));
            currentIndex = pBox->count() - 1;
            pBox->setItemData(currentIndex,
                    QVariant(QStringLiteral("<b>%1</b><br/>%2")
                                     .arg(pLoadedManifest->name(),
                                             pLoadedManifest->description())),
                    Qt::ToolTipRole);
            // Deactivate item to hopefully clarify the item is not an EQ
            const QStandardItemModel* pModel =
                    qobject_cast<QStandardItemModel*>(pBox->model());
            DEBUG_ASSERT(pModel);
            auto* pItem = pModel->item(currentIndex);
            DEBUG_ASSERT(pItem);
            pItem->setEnabled(false);
        }
        pBox->setCurrentIndex(currentIndex);
    }
    m_ignoreEqQuickEffectBoxSignals = false;
}

void DlgPrefMixer::slotPopulateQuickEffectSelectors() {
    m_ignoreEqQuickEffectBoxSignals = true;

    QList<EffectChainPresetPointer> presetList =
            m_pChainPresetManager->getQuickEffectPresetsSorted();

    for (int deck = 0; deck < m_deckQuickEffectSelectors.size(); deck++) {
        auto* pBox = m_deckQuickEffectSelectors[deck];
        pBox->clear();
        int currentIndex = 0; // preselect empty item '---' as default

        EffectChainPointer pChain = m_pEffectsManager->getQuickEffectChain(
                PlayerManager::groupForDeck(deck));
        DEBUG_ASSERT(pChain);
        for (const auto& pChainPreset : presetList) {
            pBox->addItem(pChainPreset->name());
            if (pChain->presetName() == pChainPreset->name()) {
                currentIndex = pBox->count() - 1;
            }
        }
        pBox->setCurrentIndex(currentIndex);
    }
    m_ignoreEqQuickEffectBoxSignals = false;
}

void DlgPrefMixer::slotEqOnlyToggled(bool checked) {
    m_eqEffectsOnly = checked;
    slotPopulateDeckEqSelectors();
    slotSingleEqToggled(m_singleEq);
}

void DlgPrefMixer::slotSingleEqToggled(bool checked) {
    m_singleEq = checked;
    if (m_deckEqEffectSelectors.isEmpty()) {
        return;
    }

    // If single EQ is checked copy the EQ and QuickEffect of deck 1 to the other
    // selectors. In case deck 1 has a non-EQ effect and 'EQs only' is checked we
    // need to add it. Then disable EQ selectors except deck 1.
    // Else enable all selectors and select currently loaded effects.
    if (m_singleEq) {
        m_ignoreEqQuickEffectBoxSignals = true;
        const QString deck1EqId = m_deckEqEffectSelectors[0]->currentData().toString();
        const EffectManifestPointer pManifest =
                m_pBackendManager->getManifestFromUniqueId(deck1EqId);
        int deck1QuickIndex = m_deckQuickEffectSelectors[0]->currentIndex();

        for (int deck = 1; deck < m_deckEqEffectSelectors.size(); ++deck) {
            // EQ //////////////////////////////////////////////////////////////
            int newIndex = 0;
            auto* eqBox = m_deckEqEffectSelectors[deck];
            int foundIndex = eqBox->findData(deck1EqId);
            if (foundIndex != -1) {
                newIndex = foundIndex;
            } else if (pManifest) {
                // Current selection is not part of the new list so we need to add it
                eqBox->addItem(pManifest->displayName(),
                        QVariant(pManifest->uniqueId()));
                newIndex = eqBox->count() - 1;
                eqBox->setItemData(newIndex,
                        QVariant(QStringLiteral("<b>%1</b><br/>%2")
                                         .arg(pManifest->name(),
                                                 pManifest->description())),
                        Qt::ToolTipRole);
                // Deactivate item to hopefully clarify the item is not an EQ
                const QStandardItemModel* pModel =
                        qobject_cast<QStandardItemModel*>(eqBox->model());
                DEBUG_ASSERT(pModel);
                auto* pItem = pModel->item(newIndex);
                DEBUG_ASSERT(pItem);
                pItem->setEnabled(false);
            }
            eqBox->setCurrentIndex(newIndex);
            eqBox->setDisabled(true);

            // QUickEffect /////////////////////////////////////////////////////
            auto* quickBox = m_deckQuickEffectSelectors[deck];
            quickBox->setCurrentIndex(deck1QuickIndex);
            quickBox->setDisabled(true);
        }
        m_ignoreEqQuickEffectBoxSignals = false;
    } else {
        for (int deck = 1; deck < m_deckEqEffectSelectors.size(); ++deck) {
            auto* eqBox = m_deckEqEffectSelectors[deck];
            eqBox->setEnabled(!m_eqBypass);
            slotPopulateDeckEqSelectors();

            auto* quickBox = m_deckQuickEffectSelectors[deck];
            quickBox->setEnabled(true);
            slotPopulateQuickEffectSelectors();
        }
    }
}

QUrl DlgPrefMixer::helpUrl() const {
    return QUrl(MIXXX_MANUAL_EQ_URL);
}

void DlgPrefMixer::setDefaultShelves() {
    SliderHiEQ->setValue(
            getSliderPosition(2500,
                    SliderHiEQ->minimum(),
                    SliderHiEQ->maximum()));
    SliderLoEQ->setValue(
            getSliderPosition(250,
                    SliderLoEQ->minimum(),
                    SliderLoEQ->maximum()));
}

void DlgPrefMixer::slotResetToDefaults() {
    double sliderVal = RescalerUtils::oneByXToLinear(
            EngineXfader::kTransformDefault - EngineXfader::kTransformMin + 1,
            EngineXfader::kTransformMax - EngineXfader::kTransformMin + 1,
            SliderXFader->minimum(),
            SliderXFader->maximum());
    SliderXFader->setValue(static_cast<int>(sliderVal));

    m_xFaderMode = MIXXX_XFADER_ADDITIVE;
    radioButtonAdditive->setChecked(true);
    checkBoxReverse->setChecked(false);

    // EQ //////////////////////////////////
    for (const auto& pBox : std::as_const(m_deckEqEffectSelectors)) {
        pBox->setCurrentIndex(
                pBox->findData(kDefaultEqId));
    }
    for (const auto& pBox : std::as_const(m_deckQuickEffectSelectors)) {
        pBox->setCurrentIndex(
                pBox->findText(kDefaultQuickEffectChainName));
    }
    CheckBoxBypass->setChecked(false);
    CheckBoxEqOnly->setChecked(true);
    CheckBoxSingleEqEffect->setChecked(true);
    CheckBoxEqAutoReset->setChecked(false);
    CheckBoxGainAutoReset->setChecked(false);

    setDefaultShelves();
    comboBoxMainEq->setCurrentIndex(0); // '---' no EQ
    slotApply();
}

void DlgPrefMixer::slotEQEffectSelectionChanged(int effectIndex) {
    QComboBox* c = qobject_cast<QComboBox*>(sender());
    // Check if qobject_cast was successful
    if (!c || m_ignoreEqQuickEffectBoxSignals) {
        return;
    }

    // If we are in single-effect mode and the first effect was changed,
    // change the others as well.
    // TODO Fictional use case: when user changes EQ effect on a deck other than
    // deck1 via direct chain controls, we may uncheck single EQ.
    if (m_singleEq) {
        slotSingleEqToggled(true);
    }
}

void DlgPrefMixer::slotQuickEffectSelectionChanged(int effectIndex) {
    QComboBox* c = qobject_cast<QComboBox*>(sender());
    // Check if qobject_cast was successful
    if (!c || m_ignoreEqQuickEffectBoxSignals) {
        return;
    }

    // If we are in single-effect mode and the first effect was changed,
    // change the others as well.
    if (m_singleEq) {
        slotSingleEqToggled(true);
    }
}

/// The Quick Effect was changed via the GUI or controls, update the combobox
void DlgPrefMixer::slotQuickEffectChangedOnDeck(const QString& deckGroup,
        const QString& presetName) {
    int deck;
    if (PlayerManager::isDeckGroup(deckGroup, &deck)) {
        deck -= 1; // decks indices are 0-based
        auto* pBox = m_deckQuickEffectSelectors[deck];
        VERIFY_OR_DEBUG_ASSERT(pBox) {
            return;
        }
        pBox->blockSignals(true);
        pBox->setCurrentIndex(pBox->findText(presetName));
        pBox->blockSignals(false);
    }
}

void DlgPrefMixer::applyDeckEQs() {
    m_ignoreEqQuickEffectBoxSignals = true;

    for (int deck = 0; deck < m_deckEqEffectSelectors.size(); deck++) {
        auto* pBox = m_deckEqEffectSelectors[deck];
        int effectIndex = pBox->currentIndex();

        bool needLoad = true;
        bool startingUp = m_eqIndiciesOnUpdate.size() < (deck + 1);
        if (!startingUp) {
            needLoad = effectIndex != m_eqIndiciesOnUpdate[deck];
        }

        QString deckGroup = PlayerManager::groupForDeck(deck);
        auto pChainSlot = m_pEffectsManager->getEqualizerEffectChain(deckGroup);
        DEBUG_ASSERT(pChainSlot);
        auto pEffectSlot = pChainSlot->getEffectSlot(0);
        DEBUG_ASSERT(pEffectSlot);
        pEffectSlot->setEnabled(!m_eqBypass);

        const EffectManifestPointer pManifest =
                m_pBackendManager->getManifestFromUniqueId(
                        pBox->itemData(effectIndex).toString());
        if (needLoad) {
            pEffectSlot->loadEffectWithDefaults(pManifest);
            if (pManifest && pManifest->isMixingEQ() && !m_eqBypass) {
                pChainSlot->setFilterWaveform(true);
            } else {
                pChainSlot->setFilterWaveform(false);
            }
        }

        if (startingUp) {
            m_eqIndiciesOnUpdate.append(effectIndex);
        } else {
            m_eqIndiciesOnUpdate[deck] = effectIndex;
        }

        QString effectId;
        if (pManifest) {
            effectId = pManifest->uniqueId();
        }
        // TODO store this in effects.xml
        m_pConfig->set(ConfigKey(kMixerProfile, kEffectForGroupPrefix + deckGroup),
                effectId);
    }
    m_ignoreEqQuickEffectBoxSignals = false;
}

void DlgPrefMixer::applyQuickEffects() {
    m_ignoreEqQuickEffectBoxSignals = true;

    for (int deck = 0; deck < m_deckQuickEffectSelectors.size(); deck++) {
        auto* pBox = m_deckQuickEffectSelectors[deck];
        int effectIndex = pBox->currentIndex();

        bool needLoad = true;
        bool startingUp = m_quickEffectIndiciesOnUpdate.size() < (deck + 1);
        if (!startingUp) {
            needLoad = effectIndex != m_quickEffectIndiciesOnUpdate[deck];
        }

        if (needLoad) {
            EffectChainPointer pChain = m_pEffectsManager->getQuickEffectChain(
                    PlayerManager::groupForDeck(deck));
            DEBUG_ASSERT(pChain);
            const QList<EffectChainPresetPointer> presetList =
                    m_pChainPresetManager->getQuickEffectPresetsSorted();
            if (effectIndex >= 0 && effectIndex < presetList.size()) {
                pChain->loadChainPreset(presetList[effectIndex]);
            }
        }

        if (startingUp) {
            m_quickEffectIndiciesOnUpdate.append(effectIndex);
        } else {
            m_quickEffectIndiciesOnUpdate[deck] = effectIndex;
        }
    }
    m_ignoreEqQuickEffectBoxSignals = false;
}

void DlgPrefMixer::slotHiEqSliderChanged() {
    if (SliderHiEQ->value() < SliderLoEQ->value()) {
        SliderHiEQ->setValue(SliderLoEQ->value());
    }
    m_highEqFreq = getEqFreq(SliderHiEQ->value(),
            SliderHiEQ->minimum(),
            SliderHiEQ->maximum());
    validateEQShelves();
    if (m_highEqFreq < 1000) {
        TextHiEQ->setText(QString("%1 Hz").arg(std::round(m_highEqFreq)));
    } else {
        TextHiEQ->setText(QString("%1 kHz").arg(std::round(m_highEqFreq) / 1000.));
    }

    m_COHiFreq.set(m_highEqFreq);
}

void DlgPrefMixer::slotLoEqSliderChanged() {
    if (SliderLoEQ->value() > SliderHiEQ->value()) {
        SliderLoEQ->setValue(SliderHiEQ->value());
    }
    m_lowEqFreq = getEqFreq(SliderLoEQ->value(),
            SliderLoEQ->minimum(),
            SliderLoEQ->maximum());
    validateEQShelves();
    if (m_lowEqFreq < 1000) {
        TextLoEQ->setText(QString("%1 Hz").arg(std::round(m_lowEqFreq)));
    } else {
        TextLoEQ->setText(QString("%1 kHz").arg(std::round(m_lowEqFreq) / 1000.));
    }

    m_COLoFreq.set(m_lowEqFreq);
}

void DlgPrefMixer::slotMainEQParameterSliderChanged(int value) {
    // Apply parameter, but don't write to config, yet, so Cancel will restore
    // the saved state.
    EffectSlotPointer pEffectSlot(m_pEffectMainEQ);
    if (pEffectSlot.isNull()) {
        return;
    }

    QSlider* pSlider = qobject_cast<QSlider*>(sender());
    VERIFY_OR_DEBUG_ASSERT(pSlider) { // no slider, called from elsewhere
        return;
    }

    // Update slider label
    int index = m_mainEQSliders.indexOf(pSlider);
    QLabel* pValueLabel = m_mainEQValues[index];
    VERIFY_OR_DEBUG_ASSERT(pValueLabel) {
        return;
    }
    // hide decimals for large ranges
    if (pSlider->property("roundToInt").toBool()) {
        pValueLabel->setText(QString::number(std::round(value / 100.0)));
    } else {
        pValueLabel->setText(QString::number(value / 100.0));
    }

    int paramIndex = pSlider->property("index").toInt();
    auto pParameterSlot = pEffectSlot->getEffectParameterSlot(
            EffectManifestParameter::ParameterType::Knob, paramIndex);
    VERIFY_OR_DEBUG_ASSERT(pParameterSlot && pParameterSlot->isLoaded()) {
        return;
    }
    // Calculate parameter value from relative slider position
    int sValue = pSlider->value();
    double paramValue = static_cast<double>(sValue - pSlider->minimum()) /
            static_cast<double>(pSlider->maximum() - pSlider->minimum());
    // Set the parameter
    pParameterSlot->setParameter(paramValue);
}

int DlgPrefMixer::getSliderPosition(double eqFreq, int minValue, int maxValue) {
    if (eqFreq >= kFrequencyUpperLimit) {
        return maxValue;
    } else if (eqFreq <= kFrequencyLowerLimit) {
        return minValue;
    }
    double dsliderPos = (eqFreq - kFrequencyLowerLimit) /
            (kFrequencyUpperLimit - kFrequencyLowerLimit);
    dsliderPos = pow(dsliderPos, 1.0 / 4.0) * (maxValue - minValue) + minValue;
    return static_cast<int>(std::round(dsliderPos));
}

void DlgPrefMixer::slotApply() {
    // xfader //////////////////////////////////////////////////////////////////
    m_mode.set(m_xFaderMode);
    m_curve.set(m_transform);
    m_calibration.set(m_cal);
    if (checkBoxReverse->isChecked() != m_xFaderReverse) {
        m_reverse.set(checkBoxReverse->isChecked());
        double position = m_crossfader.get();
        m_crossfader.set(0.0 - position);
        m_xFaderReverse = checkBoxReverse->isChecked();
    }
    m_pConfig->set(kXfaderModeKey, ConfigValue(m_xFaderMode));
    m_pConfig->set(kXfaderCurveKey, ConfigValue(QString::number(m_transform)));
    m_pConfig->set(kXfaderReverseKey, ConfigValue(checkBoxReverse->isChecked() ? 1 : 0));

    // EQ & QuickEffect settings ///////////////////////////////////////////////
    m_pConfig->set(kEnableEqsKey, ConfigValue(m_eqBypass ? 0 : 1));
    m_pConfig->set(kSingleEqKey, ConfigValue(m_singleEq ? 1 : 0));
    m_pConfig->set(kEqsOnlyKey, ConfigValue(m_eqEffectsOnly ? 1 : 0));
    m_pConfig->set(kEqAutoResetKey, ConfigValue(m_eqAutoReset ? 1 : 0));
    m_pConfig->set(kGainAutoResetKey, ConfigValue(m_gainAutoReset ? 1 : 0));
    applyDeckEQs();
    applyQuickEffects();

    storeEqShelves();

    storeMainEQ();
}

void DlgPrefMixer::storeEqShelves() {
    if (m_initializing) {
        return;
    }

    m_pConfig->set(kHighEqFreqPreciseKey, ConfigValue(QString::number(m_highEqFreq, 'f')));
    m_pConfig->set(kLowEqFreqPreciseKey, ConfigValue(QString::number(m_lowEqFreq, 'f')));
}

// Update the widgets with values from config / EffectsManager
void DlgPrefMixer::slotUpdate() {
    // xfader //////////////////////////////////////////////////////////////////
    m_transform = m_pConfig->getValue(kXfaderCurveKey, EngineXfader::kTransformDefault);

    // Range SliderXFader 0 .. 100
    double sliderVal = RescalerUtils::oneByXToLinear(
            m_transform - EngineXfader::kTransformMin + 1,
            EngineXfader::kTransformMax - EngineXfader::kTransformMin + 1,
            SliderXFader->minimum(),
            SliderXFader->maximum());
    SliderXFader->setValue(static_cast<int>(sliderVal + 0.5));

    m_xFaderMode = m_pConfig->getValueString(kXfaderModeKey).toInt();
    if (m_xFaderMode == MIXXX_XFADER_CONSTPWR) {
        radioButtonConstantPower->setChecked(true);
    } else {
        radioButtonAdditive->setChecked(true);
    }

    m_xFaderReverse = m_pConfig->getValueString(kXfaderReverseKey).toInt() == 1;
    checkBoxReverse->setChecked(m_xFaderReverse);

    slotUpdateXFader();

    // EQs & QuickEffects //////////////////////////////////////////////////////
    QString eqsOnly = m_pConfig->getValueString(kEqsOnlyKey);
    m_eqEffectsOnly = eqsOnly != "no" && eqsOnly != "0"; // default true
    CheckBoxEqOnly->setChecked(m_eqEffectsOnly);

    QString singleEqCfg = m_pConfig->getValueString(kSingleEqKey);
    m_singleEq = singleEqCfg != "no" && singleEqCfg != "0"; // default true
    if (!m_initializing) {
        slotPopulateDeckEqSelectors();
        slotPopulateQuickEffectSelectors();
    }
    if (m_initializing || CheckBoxSingleEqEffect->isChecked() != m_singleEq) {
        CheckBoxSingleEqEffect->setChecked(m_singleEq);
        slotSingleEqToggled(m_singleEq);
    }

    m_eqAutoReset = m_pConfig->getValue<bool>(kEqAutoResetKey, false);
    CheckBoxEqAutoReset->setChecked(m_eqAutoReset);

    m_gainAutoReset = m_pConfig->getValue<bool>(kGainAutoResetKey, false);
    CheckBoxGainAutoReset->setChecked(m_gainAutoReset);

    QString eqBaypassCfg = m_pConfig->getValueString(kEnableEqsKey);
    m_eqBypass = !(eqBaypassCfg == "yes" || eqBaypassCfg == "1"); // default false
    CheckBoxBypass->setChecked(m_eqBypass);
    // Deactivate EQ comboboxes when Bypass is enabled
    slotBypassEqToggled(CheckBoxBypass->isChecked());

    // EQ shelves //////////////////////////////////////////////////////////////
    QString highEqCoarse = m_pConfig->getValueString(kHighEqFreqKey);
    QString highEqPrecise = m_pConfig->getValueString(kHighEqFreqPreciseKey);
    QString lowEqCoarse = m_pConfig->getValueString(kLowEqFreqKey);
    QString lowEqPrecise = m_pConfig->getValueString(kLowEqFreqPreciseKey);
    double lowEqFreq = 0.0;
    double highEqFreq = 0.0;

    // Precise takes precedence over coarse.
    lowEqFreq = lowEqCoarse.isEmpty() ? lowEqFreq : lowEqCoarse.toDouble();
    lowEqFreq = lowEqPrecise.isEmpty() ? lowEqFreq : lowEqPrecise.toDouble();
    highEqFreq = highEqCoarse.isEmpty() ? highEqFreq : highEqCoarse.toDouble();
    highEqFreq = highEqPrecise.isEmpty() ? highEqFreq : highEqPrecise.toDouble();

    if (lowEqFreq == 0.0 || highEqFreq == 0.0 || lowEqFreq == highEqFreq) {
        setDefaultShelves();
    } else {
        SliderHiEQ->setValue(
                getSliderPosition(highEqFreq,
                        SliderHiEQ->minimum(),
                        SliderHiEQ->maximum()));

        SliderLoEQ->setValue(
                getSliderPosition(lowEqFreq,
                        SliderLoEQ->minimum(),
                        SliderLoEQ->maximum()));
    }

    updateMainEQ();
}

// Draw the crossfader curve graph. Only needs to get drawn when a change
// has been made.
void DlgPrefMixer::drawXfaderDisplay() {
    // Initialize or clear scene
    if (m_pxfScene) {
        m_pxfScene->clear();
    } else {
        m_pxfScene = make_parented<QGraphicsScene>(this);
        // The size of the QGraphicsView doesn't change so we need to do this only once
        graphicsViewXfader->setLineWidth(1); // frame width
        int sizeX = graphicsViewXfader->width() - 2;
        int sizeY = graphicsViewXfader->height() - 2;
        m_pxfScene->setSceneRect(0, 0, sizeX, sizeY);
        m_pxfScene->setBackgroundBrush(Qt::black);
        graphicsViewXfader->setRenderHints(QPainter::Antialiasing);
        graphicsViewXfader->setScene(m_pxfScene);
    }

    // Initialize QPens
    QPen gridPen(Qt::darkGray);
    QPen gainPen(Qt::white);
    QPen totalPen(Qt::red);
    // In conjunction with anti-aliasing this gives smooth, solid lines.
    totalPen.setWidth(2);
    gainPen.setWidth(2);
    // For some reason grid lines also appear 2px wide, with the nice side effect
    // that gain curves now intersect 'exactly'at the grid center line.

    const int sceneW = static_cast<int>(m_pxfScene->width());
    const int sceneH = static_cast<int>(m_pxfScene->height());

    // Draw grid.
    // Height is (grid segments * n)+1, so subtract 1 in order to get int coordinates.
    const double kGridHDist = ((sceneH - 1) / (kXfaderGridHLines + 1));
    const double kGridVDist = ((sceneW - 1) / (kXfaderGridVLines + 1));
    for (int i = 1; i <= kXfaderGridHLines; i++) {
        // Shift by .5 to trick anti-aliasing and get sharp lines (1px instead 2px)
        const double y = (i * kGridHDist) + .5;
        m_pxfScene->addLine(QLineF(0, y, sceneW, y), gridPen);
    }
    for (int i = 1; i <= kXfaderGridVLines; i++) {
        const double x = (i * kGridVDist) + .5;
        m_pxfScene->addLine(QLineF(x, 0, x, sceneH), gridPen);
    }

    // Draw gain curves
    // Required to make the curves fit in the view, i.e. not drawn on the left/right edge
    const int pointCount = sceneW - 2;
    const int vOffset = 1;
    const double xfadeStep = 2. / pointCount;
    // Align the curves with first (top) horizontal gridline.
    const double scaleFactorToAlignWithGrid = static_cast<double>(
                                                      kXfaderGridHLines) /
            (kXfaderGridHLines + 1)
            // Compensate for the added v-offset required to draw curves inside the view
            // (not on the edge), especially the thicker, anti-aliased curves.
            * (static_cast<double>(sceneH - vOffset) / (sceneH));
    QPolygonF polylineTotal;
    QPolygonF polylineL;
    QPolygonF polylineR;
    for (int x = 1; x <= pointCount + 1; x++) {
        CSAMPLE_GAIN gainL, gainR;
        EngineXfader::getXfadeGains((-1. + (xfadeStep * (x - 1))),
                m_transform,
                m_cal,
                m_xFaderMode,
                checkBoxReverse->isChecked(),
                &gainL,
                &gainR);

        const double gainTotal = sqrt(gainL * gainL + gainR * gainR) * scaleFactorToAlignWithGrid;
        const double gainLScaled = gainL * scaleFactorToAlignWithGrid;
        const double gainRScaled = gainR * scaleFactorToAlignWithGrid;

        polylineTotal.append(QPointF(x, (1. - gainTotal) * sceneH - 1));
        polylineL.append(QPointF(x, (1. - gainLScaled) * sceneH - vOffset));
        polylineR.append(QPointF(x, (1. - gainRScaled) * sceneH - vOffset));
    }

    QPainterPath pathTotal;
    QPainterPath pathL;
    QPainterPath pathR;
    pathTotal.addPolygon(polylineTotal);
    pathL.addPolygon(polylineL);
    pathR.addPolygon(polylineR);
    m_pxfScene->addPath(pathTotal, totalPen);
    m_pxfScene->addPath(pathL, gainPen);
    m_pxfScene->addPath(pathR, gainPen);

    graphicsViewXfader->show();
    graphicsViewXfader->repaint();
}

void DlgPrefMixer::slotUpdateXFader() {
    if (radioButtonAdditive->isChecked()) {
        m_xFaderMode = MIXXX_XFADER_ADDITIVE;
    } else {
        m_xFaderMode = MIXXX_XFADER_CONSTPWR;
    }

    // m_transform is in the range of 1 to 1000 while 50 % slider results
    // to ~2, which represents a medium rounded fader curve.
    m_transform = RescalerUtils::linearToOneByX(
                          SliderXFader->value(),
                          SliderXFader->minimum(),
                          SliderXFader->maximum(),
                          EngineXfader::kTransformMax) -
            1 + EngineXfader::kTransformMin;
    m_cal = EngineXfader::getPowerCalibration(m_transform);

    drawXfaderDisplay();
}

void DlgPrefMixer::slotEqAutoResetToggled(bool checked) {
    m_eqAutoReset = checked;
}

void DlgPrefMixer::slotGainAutoResetToggled(bool checked) {
    m_gainAutoReset = checked;
}

void DlgPrefMixer::slotBypassEqToggled(bool checked) {
    m_eqBypass = checked;

    // De/activate deck EQ comboboxes
    for (int deck = 0; deck < m_deckEqEffectSelectors.size(); deck++) {
        auto* pBox = m_deckEqEffectSelectors[deck];
        if (deck == 0) {
            pBox->setEnabled(!m_eqBypass);
        } else {
            pBox->setEnabled(!m_eqBypass && !m_singleEq);
        }
    }
}

void DlgPrefMixer::setUpMainEQ() {
    auto pChainSlot = m_pEffectsManager->getOutputEffectChain();
    DEBUG_ASSERT(pChainSlot);
    auto pEffectSlot = pChainSlot->getEffectSlot(0);
    DEBUG_ASSERT(pEffectSlot);
    m_pEffectMainEQ = pEffectSlot;

    connect(pbResetMainEq, &QPushButton::clicked, this, &DlgPrefMixer::slotMainEQToDefault);

    connect(comboBoxMainEq,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefMixer::slotMainEqEffectChanged);

    const QList<EffectManifestPointer> availableMainEQEffects = getMainEqManifests();

    // Add empty '---' item at the top
    comboBoxMainEq->addItem(kNoEffectString);
    for (const auto& pManifest : availableMainEQEffects) {
        comboBoxMainEq->addItem(pManifest->name(), QVariant(pManifest->uniqueId()));
        // <b> makes the effect name bold. Also, like <span> it serves as hack
        // to get Qt to treat the string as rich text so it automatically wraps long lines.
        comboBoxMainEq->setItemData(comboBoxMainEq->count() - 1,
                QVariant(QStringLiteral("<b>%1</b><br/>%2")
                                 .arg(pManifest->name(),
                                         pManifest->description())),
                Qt::ToolTipRole);
    }
    comboBoxMainEq->setCurrentIndex(0);
}

void DlgPrefMixer::updateMainEQ() {
    const QString configuredEffectId =
            m_pConfig->getValue(ConfigKey(kMixerProfile, kEffectGroupForMaster),
                    kDefaultMainEqId);
    const EffectManifestPointer configuredEffectManifest =
            m_pBackendManager->getManifestFromUniqueId(configuredEffectId);
    int mainEqIndex = 0; // selects '---' by default
    if (configuredEffectManifest) {
        mainEqIndex = comboBoxMainEq->findData(configuredEffectManifest->uniqueId());
    }
    // Set index and create required sliders and labels
    comboBoxMainEq->setCurrentIndex(mainEqIndex);

    // Load parameters from preferences and set sliders
    for (QSlider* pSlider : std::as_const(m_mainEQSliders)) {
        int paramIndex = pSlider->property("index").toInt();
        QString strValue = m_pConfig->getValueString(ConfigKey(kMixerProfile,
                kMainEQParameterKey + QString::number(paramIndex + 1)));

        bool ok;
        double paramValue = strValue.toDouble(&ok);
        if (!ok) {
            continue;
        }
        pSlider->setValue(static_cast<int>(paramValue * 100));
        // Thelabel is updated in slotMainEQParameterSliderChanged()
    }
}

void DlgPrefMixer::slotMainEqEffectChanged(int effectIndex) {
    EffectSlotPointer pEffectSlot(m_pEffectMainEQ);
    VERIFY_OR_DEBUG_ASSERT(!pEffectSlot.isNull()) {
        return;
    }

    // Clear parameters view first
    qDeleteAll(m_mainEQSliders);
    m_mainEQSliders.clear();
    qDeleteAll(m_mainEQValues);
    m_mainEQValues.clear();
    qDeleteAll(m_mainEQLabels);
    m_mainEQLabels.clear();

    const EffectManifestPointer pManifest =
            m_pBackendManager->getManifestFromUniqueId(
                    comboBoxMainEq->itemData(effectIndex).toString());

    pEffectSlot->loadEffectWithDefaults(pManifest);

    if (pManifest) {
        pbResetMainEq->show();
    } else {
        pbResetMainEq->hide();
        // no effect, no sliders, nothing to do
        return;
    }

    // Create and set up Main EQ's sliders
    const auto params = pManifest->parameters();
    for (const auto& param : params) {
        if (param->parameterType() != EffectManifestParameter::ParameterType::Knob) {
            continue;
        }
        int i = param->index();

        auto pCenterFreqLabel = make_parented<QLabel>(this);
        QString labelText = param->name();
        pCenterFreqLabel->setText(labelText);
        m_mainEQLabels.append(pCenterFreqLabel);
        slidersGridLayout->addWidget(pCenterFreqLabel, 0, i + 1, Qt::AlignCenter);

        auto pSlider = make_parented<QSlider>(this);
        setScrollSafeGuard(pSlider);
        pSlider->setProperty("index", QVariant(i));
        pSlider->setMinimumHeight(90);
        pSlider->setMinimum(static_cast<int>(param->getMinimum() * 100));
        pSlider->setMaximum(static_cast<int>(param->getMaximum() * 100));
        // Make steps depend on range
        int step = 1;
        bool roundToInt = false;
        const double range = param->getMaximum() - param->getMinimum();
        if (range > 1000) {
            step = 100;
            // Used to round value text to integer
            roundToInt = true;
        } else if (range > 100) {
            step = 10;
        }
        pSlider->setProperty("roundToInt", roundToInt);
        pSlider->setSingleStep(step);
        pSlider->setPageStep(step * 10);
        pSlider->setValue(static_cast<int>(param->getDefault() * 100));
        // Store the index as a property because we need it in
        // slotMainEQParameterSliderChanged() and storeMainEQ()
        slidersGridLayout->addWidget(pSlider, 1, i + 1, Qt::AlignCenter);
        m_mainEQSliders.append(pSlider);
        // catch drag event
        connect(pSlider,
                &QSlider::sliderMoved,
                this,
                &DlgPrefMixer::slotMainEQParameterSliderChanged);
        // catch scroll event and slider->setValue()
        connect(pSlider,
                &QSlider::valueChanged,
                this,
                &DlgPrefMixer::slotMainEQParameterSliderChanged);

        auto pValueLabel = make_parented<QLabel>(this);
        QString valueText = QString::number((double)pSlider->value() / 100);
        pValueLabel->setText(valueText);

        // Use max required label width so column width doesn't change when the
        // slider is moved. Considers decimals only where we actually show them.
        QFontMetrics metrics(font());
        QString maxValueString = QString::number(
                roundToInt ? pSlider->maximum() : pSlider->maximum() - 0.01);
        QString minValueString = QString::number(
                pSlider->maximum() ? pSlider->minimum() : pSlider->minimum() + 0.01);
        int optWidth = math_max(
                metrics.size(0, maxValueString).width(),
                metrics.size(0, minValueString).width());
        pValueLabel->setMinimumWidth(optWidth);
        pValueLabel->setAlignment(Qt::AlignCenter);

        m_mainEQValues.append(pValueLabel);
        slidersGridLayout->addWidget(pValueLabel, 2, i + 1, Qt::AlignCenter);
    }
}

double DlgPrefMixer::getEqFreq(int sliderVal, int minValue, int maxValue) {
    // We're mapping f(x) = x^4 onto the range kFrequencyLowerLimit,
    // kFrequencyUpperLimit with x [minValue, maxValue]. First translate x into
    // [0.0, 1.0], raise it to the 4th power, and then scale the result from
    // [0.0, 1.0] to [kFrequencyLowerLimit, kFrequencyUpperLimit].
    double normValue = static_cast<double>(sliderVal - minValue) /
            (maxValue - minValue);
    // Use a non-linear mapping between slider and frequency.
    normValue = normValue * normValue * normValue * normValue;
    double result = normValue * (kFrequencyUpperLimit - kFrequencyLowerLimit) +
            kFrequencyLowerLimit;
    return result;
}

void DlgPrefMixer::validateEQShelves() {
    m_highEqFreq = math_clamp<double>(m_highEqFreq, kFrequencyLowerLimit, kFrequencyUpperLimit);
    m_lowEqFreq = math_clamp<double>(m_lowEqFreq, kFrequencyLowerLimit, kFrequencyUpperLimit);
    if (m_lowEqFreq == m_highEqFreq) {
        if (m_lowEqFreq == kFrequencyLowerLimit) {
            ++m_highEqFreq;
        } else if (m_highEqFreq == kFrequencyUpperLimit) {
            --m_lowEqFreq;
        } else {
            ++m_highEqFreq;
        }
    }
}

void DlgPrefMixer::slotMainEQToDefault() {
    const EffectManifestPointer pManifest =
            m_pBackendManager->getManifestFromUniqueId(
                    comboBoxMainEq->currentData().toString());
    if (!pManifest) {
        return;
    }

    for (QSlider* pSlider : std::as_const(m_mainEQSliders)) {
        int paramIndex = pSlider->property("index").toInt();
        auto pParam = pManifest->parameter(paramIndex);
        pSlider->setValue(static_cast<int>(pParam->getDefault() * 100));
    }
}

void DlgPrefMixer::storeMainEQ() {
    // store effect
    const EffectManifestPointer pManifest =
            m_pBackendManager->getManifestFromUniqueId(
                    comboBoxMainEq->currentData().toString());

    m_pConfig->set(ConfigKey(kMixerProfile, kEffectGroupForMaster),
            ConfigValue(pManifest ? pManifest->uniqueId() : ""));

    // clear all previous parameter values so we have no residues
    const QList<ConfigKey> mixerKeys = m_pConfig->getKeysWithGroup(kMixerProfile);
    for (const auto& key : mixerKeys) {
        if (key.item.contains(kMainEQParameterKey)) {
            m_pConfig->remove(key);
        }
    }

    // store current parameters
    for (QSlider* pSlider : std::as_const(m_mainEQSliders)) {
        int paramIndex = pSlider->property("index").toInt();
        double dispValue = static_cast<double>(pSlider->value()) / 100;
        // TODO store this in effects.xml
        m_pConfig->set(ConfigKey(kMixerProfile,
                               kMainEQParameterKey + QString::number(paramIndex + 1)),
                ConfigValue(QString::number(dispValue)));
    }
}

const QList<EffectManifestPointer> DlgPrefMixer::getDeckEqManifests() const {
    const QList<EffectManifestPointer> allManifests =
            m_pBackendManager->getManifests();
    QList<EffectManifestPointer> eqs;
    QList<EffectManifestPointer> nonEqs;
    // Add EQs first, then append non-EQs
    for (const auto& pManifest : allManifests) {
        if (isMixingEQ(pManifest.data())) {
            eqs.append(pManifest);
        } else if (!m_eqEffectsOnly) {
            nonEqs.append(pManifest);
        }
    }
    // Append non-EQs. No-op if 'EQs only' is unchecked.
    eqs.append(nonEqs);
    return eqs;
}

const QList<EffectManifestPointer> DlgPrefMixer::getMainEqManifests() const {
    const QList<EffectManifestPointer> allManifests =
            m_pBackendManager->getManifests();
    QList<EffectManifestPointer> eqs;
    for (const auto& pManifest : allManifests) {
        if (isMainEQ(pManifest.data())) {
            eqs.append(pManifest);
        }
    }
    return eqs;
}
