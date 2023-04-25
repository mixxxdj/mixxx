#include "preferences/dialog/dlgprefmixer.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QString>
#include <QWidget>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "defs_urls.h"
#include "effects/backends/builtin/biquadfullkilleqeffect.h"
#include "effects/chains/equalizereffectchain.h"
#include "effects/chains/quickeffectchain.h"
#include "effects/effectslot.h"
#include "engine/enginexfader.h"
#include "mixer/playermanager.h"
#include "moc_dlgprefmixer.cpp"
#include "util/math.h"
#include "util/rescaler.h"

namespace {
const QString kConfigGroup = QStringLiteral("[Mixer Profile]");
const QString kEffectForGroupPrefix = QStringLiteral("EffectForGroup_");
const QString kEnableEqs = QStringLiteral("EnableEQs");
const QString kEqsOnly = QStringLiteral("EQsOnly");
const QString kSingleEq = QStringLiteral("SingleEQEffect");
const QString kMainEQParameterKey = QStringLiteral("EffectForGroup_[Master]_parameter");
const QString kDefaultEqId = BiquadFullKillEQEffect::getId() + " " +
        EffectsBackend::backendTypeToString(EffectBackendType::BuiltIn);
const QString kDefaultQuickEffectChainName = QStringLiteral("Filter");
const QString kDefaultMainEqId = QString();

constexpr int kFrequencyUpperLimit = 20050;
constexpr int kFrequencyLowerLimit = 16;

bool isMixingEQ(EffectManifest* pManifest) {
    return pManifest->isMixingEQ();
}

bool isMainEQ(EffectManifest* pManifest) {
    return pManifest->isMasterEQ();
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
          m_mode(EngineXfader::kXfaderConfigKey, "xFaderMode"),
          m_curve(EngineXfader::kXfaderConfigKey, "xFaderCurve"),
          m_calibration(EngineXfader::kXfaderConfigKey, "xFaderCalibration"),
          m_reverse(EngineXfader::kXfaderConfigKey, "xFaderReverse"),
          m_crossfader("[Master]", "crossfader"),
          m_xFaderReverse(false),
          m_COLoFreq(kConfigGroup, QStringLiteral("LoEQFrequency")),
          m_COHiFreq(kConfigGroup, QStringLiteral("HiEQFrequency")),
          m_lowEqFreq(0.0),
          m_highEqFreq(0.0),
          m_pChainPresetManager(pEffectsManager->getChainPresetManager()),
          m_pEffectsManager(pEffectsManager),
          m_pBackendManager(pEffectsManager->getBackendManager()),
          m_firstSelectorLabel(nullptr),
          m_pNumDecks(nullptr),
          m_inSlotPopulateDeckEffectSelectors(false),
          m_bEqAutoReset(false),
          m_bGainAutoReset(false) {
    setupUi(this);

    loadSettings();

    connect(SliderXFader,
            QOverload<int>::of(&QSlider::valueChanged),
            this,
            &DlgPrefMixer::slotUpdateXFader);
    connect(SliderXFader, &QSlider::sliderMoved, this, &DlgPrefMixer::slotUpdateXFader);
    connect(SliderXFader, &QSlider::sliderReleased, this, &DlgPrefMixer::slotUpdateXFader);

    // Update the crossfader curve graph and other settings when the
    // crossfader mode is changed.
    connect(radioButtonAdditive, &QRadioButton::clicked, this, &DlgPrefMixer::slotUpdateXFader);
    connect(radioButtonConstantPower,
            &QRadioButton::clicked,
            this,
            &DlgPrefMixer::slotUpdateXFader);

    // Don't allow the xfader graph getting keyboard focus
    graphicsViewXfader->setFocusPolicy(Qt::NoFocus);

    // Connection
    connect(SliderHiEQ, &QSlider::valueChanged, this, &DlgPrefMixer::slotUpdateHiEQ);
    connect(SliderHiEQ, &QSlider::sliderMoved, this, &DlgPrefMixer::slotUpdateHiEQ);
    connect(SliderHiEQ, &QSlider::sliderReleased, this, &DlgPrefMixer::slotUpdateHiEQ);

    connect(SliderLoEQ, &QSlider::valueChanged, this, &DlgPrefMixer::slotUpdateLoEQ);
    connect(SliderLoEQ, &QSlider::sliderMoved, this, &DlgPrefMixer::slotUpdateLoEQ);
    connect(SliderLoEQ, &QSlider::sliderReleased, this, &DlgPrefMixer::slotUpdateLoEQ);

    connect(CheckBoxEqAutoReset,
            &QCheckBox::stateChanged,
            this,
            &DlgPrefMixer::slotUpdateEqAutoReset);
    connect(CheckBoxGainAutoReset,
            &QCheckBox::stateChanged,
            this,
            &DlgPrefMixer::slotUpdateGainAutoReset);
    connect(CheckBoxBypass,
            &QCheckBox::stateChanged,
            this,
            &DlgPrefMixer::slotBypassEqChanged);

    connect(CheckBoxEqOnly,
            &QCheckBox::stateChanged,
            this,
            &DlgPrefMixer::slotPopulateDeckEffectSelectors);

    connect(CheckBoxSingleEqEffect,
            &QCheckBox::stateChanged,
            this,
            &DlgPrefMixer::slotSingleEqCheckboxChanged);
    // Quick hack to update the checkbox "Use the same EQ filter for all decks"
    // to not use the default state (checked) when slotNumDecksChanged() calls
    // slotSingleEqCheckboxChanged(state) here in constructor, because that would
    // be written to config immediateley and thus reset the previous unchecked state.
    // TODO(ronso0) Write only in slotApply(), read from config only in slotUpdate().
    // Currently config is read in both slotUpdate() and loadSettings().
    CheckBoxSingleEqEffect->setChecked(
            m_pConfig->getValue(ConfigKey(kConfigGroup, kSingleEq), "yes") == "yes");
    slotSingleEqCheckboxChanged(CheckBoxSingleEqEffect->isChecked());

    // Add drop down lists for current decks and connect num_decks control
    // to slotNumDecksChanged
    m_pNumDecks = new ControlProxy("[Master]", "num_decks", this);
    m_pNumDecks->connectValueChanged(this, &DlgPrefMixer::slotNumDecksChanged);
    slotNumDecksChanged(m_pNumDecks->get());

    connect(m_pChainPresetManager.data(),
            &EffectChainPresetManager::quickEffectChainPresetListUpdated,
            this,
            &DlgPrefMixer::slotPopulateDeckEffectSelectors);

    setUpMainEQ();

    setScrollSafeGuard(SliderXFader);
    setScrollSafeGuard(SliderHiEQ);
    setScrollSafeGuard(SliderLoEQ);
    setScrollSafeGuard(comboBoxMainEq);

    slotUpdate();
    slotApply();
}

DlgPrefMixer::~DlgPrefMixer() {
    qDeleteAll(m_deckEqEffectSelectors);
    m_deckEqEffectSelectors.clear();

    qDeleteAll(m_deckQuickEffectSelectors);
    m_deckQuickEffectSelectors.clear();
}

void DlgPrefMixer::slotNumDecksChanged(double numDecks) {
    int oldDecks = m_deckEqEffectSelectors.size();
    while (m_deckEqEffectSelectors.size() < static_cast<int>(numDecks)) {
        int deckNo = m_deckEqEffectSelectors.size() + 1;

        QLabel* label = new QLabel(QObject::tr("Deck %1").arg(deckNo), this);

        // Create the drop down list for deck EQs
        QComboBox* pEqComboBox = new QComboBox(this);
        m_deckEqEffectSelectors.append(pEqComboBox);
        connect(pEqComboBox,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                this,
                &DlgPrefMixer::slotEffectChangedOnDeck);
        setScrollSafeGuard(pEqComboBox);

        // Create the drop down list for Quick Effects
        QComboBox* pQuickEffectComboBox = new QComboBox(this);
        m_deckQuickEffectSelectors.append(pQuickEffectComboBox);
        connect(pQuickEffectComboBox,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                this,
                &DlgPrefMixer::slotQuickEffectChangedOnDeck);
        setScrollSafeGuard(pQuickEffectComboBox);

        QString deckGroupName = PlayerManager::groupForDeck(deckNo - 1);
        QString unitGroup = QuickEffectChain::formatEffectChainGroup(deckGroupName);
        EffectChainPointer pChain = m_pEffectsManager->getEffectChain(unitGroup);
        connect(pChain.data(),
                &EffectChain::chainPresetChanged,
                this,
                [this, pQuickEffectComboBox](const QString& name) {
                    m_inSlotPopulateDeckEffectSelectors = true;
                    pQuickEffectComboBox->setCurrentIndex(pQuickEffectComboBox->findText(name));
                    m_inSlotPopulateDeckEffectSelectors = false;
                });

        if (deckNo == 1) {
            m_firstSelectorLabel = label;
            if (CheckBoxSingleEqEffect->isChecked()) {
                m_firstSelectorLabel->clear();
            }
        }

        // Setup the GUI
        gridLayout_3->addWidget(label, deckNo, 0);
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
    slotPopulateDeckEffectSelectors();
    for (int i = oldDecks; i < static_cast<int>(numDecks); ++i) {
        // Set the configured effect for box and simpleBox or default
        // if none is configured
        QString group = PlayerManager::groupForDeck(i);
        QString configuredEffect = m_pConfig->getValue(
                ConfigKey(kConfigGroup, kEffectForGroupPrefix + group),
                kDefaultEqId);

        const EffectManifestPointer pEQManifest =
                m_pBackendManager->getManifestFromUniqueId(configuredEffect);

        int selectedEQEffectIndex = 0; // placeholder for no effect
        if (pEQManifest) {
            selectedEQEffectIndex = m_deckEqEffectSelectors[i]->findData(
                    QVariant(pEQManifest->uniqueId()));
        }
        m_deckEqEffectSelectors[i]->setCurrentIndex(selectedEQEffectIndex);
    }
    applySelections();
    slotSingleEqCheckboxChanged(CheckBoxSingleEqEffect->isChecked());
}

void DlgPrefMixer::slotPopulateDeckEffectSelectors() {
    m_inSlotPopulateDeckEffectSelectors = true; // prevents a recursive call

    EffectManifestFilterFnc filterEQ;
    if (CheckBoxEqOnly->isChecked()) {
        m_pConfig->set(ConfigKey(kConfigGroup, kEqsOnly), QString("yes"));
        filterEQ = isMixingEQ;
    } else {
        m_pConfig->set(ConfigKey(kConfigGroup, kEqsOnly), QString("no"));
        filterEQ = nullptr; // take all
    }

    populateDeckEqBoxList(m_deckEqEffectSelectors, filterEQ);
    populateDeckQuickEffectBoxList(m_deckQuickEffectSelectors);

    m_inSlotPopulateDeckEffectSelectors = false;
}

void DlgPrefMixer::populateDeckEqBoxList(
        const QList<QComboBox*>& boxList,
        EffectManifestFilterFnc filterFunc) {
    const QList<EffectManifestPointer> pManifestList = getFilteredManifests(filterFunc);
    for (QComboBox* box : boxList) {
        // Populate comboboxes with all available effects
        // Save current selection
        const EffectManifestPointer pCurrentlySelectedManifest =
                m_pBackendManager->getManifestFromUniqueId(
                        box->itemData(box->currentIndex()).toString());

        box->clear();
        int currentIndex = -1; // Nothing selected

        // Add empty item at the top: no effect
        box->addItem(kNoEffectString);
        int i = 1;
        for (const auto& pManifest : pManifestList) {
            box->addItem(pManifest->name(), QVariant(pManifest->uniqueId()));
            if (pCurrentlySelectedManifest &&
                    pCurrentlySelectedManifest.data() == pManifest.data()) {
                currentIndex = i;
            }
            ++i;
        }
        if (pCurrentlySelectedManifest == nullptr) {
            // Configured effect has no id, clear selection
            currentIndex = 0;
        } else if (currentIndex < 0) {
            // current selection is not part of the new list
            // So we need to add it
            box->addItem(pCurrentlySelectedManifest->shortName(),
                    QVariant(pCurrentlySelectedManifest->uniqueId()));
            currentIndex = i + 1;
        }
        box->setCurrentIndex(currentIndex);
    }
}

void DlgPrefMixer::populateDeckQuickEffectBoxList(
        const QList<QComboBox*>& boxList) {
    QList<EffectChainPresetPointer> presetList =
            m_pChainPresetManager->getQuickEffectPresetsSorted();

    int deck = 0;
    for (QComboBox* box : boxList) {
        box->clear();
        int currentIndex = -1; // Nothing selected

        QString deckGroupName = PlayerManager::groupForDeck(deck);
        QString unitGroup = QuickEffectChain::formatEffectChainGroup(deckGroupName);
        EffectChainPointer pChain = m_pEffectsManager->getEffectChain(unitGroup);
        int i = 0;
        for (const auto& pChainPreset : presetList) {
            box->addItem(pChainPreset->name());
            if (pChain->presetName() == pChainPreset->name()) {
                currentIndex = i;
            }
            ++i;
        }
        box->setCurrentIndex(currentIndex);
        ++deck;
    }
}

void DlgPrefMixer::slotSingleEqCheckboxChanged(int checked) {
    bool do_hide = static_cast<bool>(checked);
    m_pConfig->set(ConfigKey(kConfigGroup, kSingleEq),
            do_hide ? QString("yes") : QString("no"));
    if (m_deckEqEffectSelectors.size()) {
        int deck1EQIndex = m_deckEqEffectSelectors.at(0)->currentIndex();
        for (int i = 2; i < m_deckEqEffectSelectors.size() + 1; ++i) {
            if (do_hide) {
                m_deckEqEffectSelectors.at(i - 1)->setCurrentIndex(deck1EQIndex);
                gridLayout_3->itemAtPosition(i, 0)->widget()->hide();
                gridLayout_3->itemAtPosition(i, 1)->widget()->hide();
                gridLayout_3->itemAtPosition(i, 2)->widget()->hide();
            } else {
                gridLayout_3->itemAtPosition(i, 0)->widget()->show();
                gridLayout_3->itemAtPosition(i, 1)->widget()->show();
                gridLayout_3->itemAtPosition(i, 2)->widget()->show();
            }
        }
    }

    if (m_firstSelectorLabel != nullptr) {
        if (do_hide) {
            m_firstSelectorLabel->clear();
        } else {
            m_firstSelectorLabel->setText(QObject::tr("Deck 1"));
        }
    }

    applySelections();
}

QUrl DlgPrefMixer::helpUrl() const {
    return QUrl(MIXXX_MANUAL_EQ_URL);
}

void DlgPrefMixer::loadSettings() {
    // Range xFaderCurve EngineXfader::kTransformMin .. EngineXfader::kTransformMax
    m_transform = m_pConfig->getValue(
            ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderCurve"),
            EngineXfader::kTransformDefault);

    // Range SliderXFader 0 .. 100
    double sliderVal = RescalerUtils::oneByXToLinear(
            m_transform - EngineXfader::kTransformMin + 1,
            EngineXfader::kTransformMax - EngineXfader::kTransformMin + 1,
            SliderXFader->minimum(),
            SliderXFader->maximum());
    SliderXFader->setValue(static_cast<int>(sliderVal + 0.5));

    m_xFaderMode = m_pConfig->getValueString(
                                    ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderMode"))
                           .toInt();

    if (m_xFaderMode == MIXXX_XFADER_CONSTPWR) {
        radioButtonConstantPower->setChecked(true);
    } else {
        radioButtonAdditive->setChecked(true);
    }

    m_xFaderReverse = m_pConfig->getValueString(
                                       ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderReverse"))
                              .toInt() == 1;
    checkBoxReverse->setChecked(m_xFaderReverse);

    slotUpdateXFader();
    slotApply();

    // EQ ////////////////////////////////////////////////
    QString highEqCourse = m_pConfig->getValueString(ConfigKey(kConfigGroup, "HiEQFrequency"));
    QString highEqPrecise = m_pConfig->getValueString(
            ConfigKey(kConfigGroup, "HiEQFrequencyPrecise"));
    QString lowEqCourse = m_pConfig->getValueString(ConfigKey(kConfigGroup, "LoEQFrequency"));
    QString lowEqPrecise = m_pConfig->getValueString(
            ConfigKey(kConfigGroup, "LoEQFrequencyPrecise"));
    m_bEqAutoReset = static_cast<bool>(
            m_pConfig->getValueString(ConfigKey(kConfigGroup, "EqAutoReset"))
                    .toInt());
    CheckBoxEqAutoReset->setChecked(m_bEqAutoReset);
    m_bGainAutoReset = static_cast<bool>(
            m_pConfig->getValueString(ConfigKey(kConfigGroup, "GainAutoReset"))
                    .toInt());
    CheckBoxGainAutoReset->setChecked(m_bGainAutoReset);
    CheckBoxBypass->setChecked(
            m_pConfig->getValue(
                    ConfigKey(kConfigGroup, kEnableEqs), QString("yes")) == "no");
    CheckBoxEqOnly->setChecked(
            m_pConfig->getValue(
                    ConfigKey(kConfigGroup, kEqsOnly), "yes") == "yes");
    CheckBoxSingleEqEffect->setChecked(
            m_pConfig->getValue(
                    ConfigKey(kConfigGroup, kSingleEq), "yes") == "yes");

    double lowEqFreq = 0.0;
    double highEqFreq = 0.0;

    // Precise takes precedence over course.
    lowEqFreq = lowEqCourse.isEmpty() ? lowEqFreq : lowEqCourse.toDouble();
    lowEqFreq = lowEqPrecise.isEmpty() ? lowEqFreq : lowEqPrecise.toDouble();
    highEqFreq = highEqCourse.isEmpty() ? highEqFreq : highEqCourse.toDouble();
    highEqFreq = highEqPrecise.isEmpty() ? highEqFreq : highEqPrecise.toDouble();

    if (lowEqFreq == 0.0 || highEqFreq == 0.0 || lowEqFreq == highEqFreq) {
        setDefaultShelves();
        lowEqFreq = m_pConfig
                            ->getValueString(ConfigKey(
                                    kConfigGroup, "LoEQFrequencyPrecise"))
                            .toDouble();
        highEqFreq = m_pConfig
                             ->getValueString(ConfigKey(
                                     kConfigGroup, "HiEQFrequencyPrecise"))
                             .toDouble();
    }

    SliderHiEQ->setValue(
            getSliderPosition(highEqFreq,
                    SliderHiEQ->minimum(),
                    SliderHiEQ->maximum()));
    SliderLoEQ->setValue(
            getSliderPosition(lowEqFreq,
                    SliderLoEQ->minimum(),
                    SliderLoEQ->maximum()));

    if (m_pConfig->getValue(
                ConfigKey(kConfigGroup, kEnableEqs), "yes") == "yes") {
        CheckBoxBypass->setChecked(false);
    }
}

void DlgPrefMixer::setDefaultShelves() {
    m_pConfig->set(ConfigKey(kConfigGroup, "HiEQFrequency"), ConfigValue(2500));
    m_pConfig->set(ConfigKey(kConfigGroup, "LoEQFrequency"), ConfigValue(250));
    m_pConfig->set(ConfigKey(kConfigGroup, "HiEQFrequencyPrecise"), ConfigValue(2500.0));
    m_pConfig->set(ConfigKey(kConfigGroup, "LoEQFrequencyPrecise"), ConfigValue(250.0));
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
    slotMainEQToDefault();
    setDefaultShelves();
    for (const auto& pCombo : std::as_const(m_deckEqEffectSelectors)) {
        pCombo->setCurrentIndex(
                pCombo->findData(kDefaultEqId));
    }
    for (const auto& pCombo : std::as_const(m_deckQuickEffectSelectors)) {
        pCombo->setCurrentIndex(
                pCombo->findText(kDefaultQuickEffectChainName));
    }
    loadSettings();
    CheckBoxBypass->setChecked(false);
    CheckBoxEqOnly->setChecked(true);
    CheckBoxSingleEqEffect->setChecked(true);
    CheckBoxEqAutoReset->setChecked(m_bEqAutoReset);
    CheckBoxGainAutoReset->setChecked(m_bGainAutoReset);
    slotUpdate();
    slotApply();
}

void DlgPrefMixer::slotEffectChangedOnDeck(int effectIndex) {
    QComboBox* c = qobject_cast<QComboBox*>(sender());
    // Check if qobject_cast was successful
    if (!c || m_inSlotPopulateDeckEffectSelectors) {
        return;
    }

    QList<QComboBox*>* pBoxList = &m_deckEqEffectSelectors;
    int deckNumber = pBoxList->indexOf(c);
    // If we are in single-effect mode and the first effect was changed,
    // change the others as well.
    if (deckNumber == 0 && CheckBoxSingleEqEffect->isChecked()) {
        for (int otherDeck = 1;
                otherDeck < static_cast<int>(m_pNumDecks->get());
                ++otherDeck) {
            QComboBox* box = (*pBoxList)[otherDeck];
            box->setCurrentIndex(effectIndex);
        }
    }

    // This is required to remove a previous selected effect that does not
    // fit to the current ShowAllEffects checkbox
    slotPopulateDeckEffectSelectors();
}

void DlgPrefMixer::slotQuickEffectChangedOnDeck(int effectIndex) {
    QComboBox* c = qobject_cast<QComboBox*>(sender());
    // Check if qobject_cast was successful
    if (!c || m_inSlotPopulateDeckEffectSelectors) {
        return;
    }

    QList<QComboBox*>* pBoxList = &m_deckQuickEffectSelectors;
    int deckNumber = pBoxList->indexOf(c);
    // If we are in single-effect mode and the first effect was changed,
    // change the others as well.
    if (deckNumber == 0 && CheckBoxSingleEqEffect->isChecked()) {
        for (int otherDeck = 1;
                otherDeck < static_cast<int>(m_pNumDecks->get());
                ++otherDeck) {
            QComboBox* box = m_deckQuickEffectSelectors[otherDeck];
            box->setCurrentIndex(effectIndex);
        }
    }

    QString deckGroupName = PlayerManager::groupForDeck(deckNumber);
    QString unitGroup = QuickEffectChain::formatEffectChainGroup(deckGroupName);
    EffectChainPointer pChain = m_pEffectsManager->getEffectChain(unitGroup);
    QList<EffectChainPresetPointer> presetList =
            m_pChainPresetManager->getQuickEffectPresetsSorted();
    if (pChain && effectIndex >= 0 && effectIndex < presetList.size()) {
        pChain->loadChainPreset(presetList[effectIndex]);
    }
}

void DlgPrefMixer::applySelections() {
    if (m_inSlotPopulateDeckEffectSelectors) {
        return;
    }

    applySelectionsToDecks();
}

void DlgPrefMixer::applySelectionsToDecks() {
    int deck = 0;
    for (QComboBox* box : std::as_const(m_deckEqEffectSelectors)) {
        const EffectManifestPointer pManifest =
                m_pBackendManager->getManifestFromUniqueId(
                        box->itemData(box->currentIndex()).toString());

        QString group = PlayerManager::groupForDeck(deck);

        bool needLoad = true;
        bool startingUp = (m_eqIndiciesOnUpdate.size() < (deck + 1));
        if (!startingUp) {
            needLoad = (box->currentIndex() != m_eqIndiciesOnUpdate[deck]);
        }
        if (needLoad) {
            auto pChainSlot = m_pEffectsManager->getEqualizerEffectChain(group);
            auto pEffectSlot = pChainSlot->getEffectSlot(0);
            pEffectSlot->loadEffectWithDefaults(pManifest);
            if (pManifest && pManifest->isMixingEQ()) {
                pChainSlot->setFilterWaveform(true);
            } else {
                pChainSlot->setFilterWaveform(false);
            }

            if (!startingUp) {
                m_eqIndiciesOnUpdate[deck] = box->currentIndex();
            }

            QString configString;
            if (pManifest) {
                configString = pManifest->uniqueId();
            }
            m_pConfig->set(ConfigKey(kConfigGroup, kEffectForGroupPrefix + group),
                    configString);

            // This is required to remove a previous selected effect that does not
            // fit to the current ShowAllEffects checkbox
            slotPopulateDeckEffectSelectors();
        }
        ++deck;
    }
}

void DlgPrefMixer::slotUpdateHiEQ() {
    if (SliderHiEQ->value() < SliderLoEQ->value()) {
        SliderHiEQ->setValue(SliderLoEQ->value());
    }
    m_highEqFreq = getEqFreq(SliderHiEQ->value(),
            SliderHiEQ->minimum(),
            SliderHiEQ->maximum());
    validate_levels();
    if (m_highEqFreq < 1000) {
        TextHiEQ->setText(QString("%1 Hz").arg((int)m_highEqFreq));
    } else {
        TextHiEQ->setText(QString("%1 kHz").arg((int)m_highEqFreq / 1000.));
    }
    m_pConfig->set(ConfigKey(kConfigGroup, "HiEQFrequency"),
            ConfigValue(QString::number(static_cast<int>(m_highEqFreq))));
    m_pConfig->set(ConfigKey(kConfigGroup, "HiEQFrequencyPrecise"),
            ConfigValue(QString::number(m_highEqFreq, 'f')));

    slotApply();
}

void DlgPrefMixer::slotUpdateLoEQ() {
    if (SliderLoEQ->value() > SliderHiEQ->value()) {
        SliderLoEQ->setValue(SliderHiEQ->value());
    }
    m_lowEqFreq = getEqFreq(SliderLoEQ->value(),
            SliderLoEQ->minimum(),
            SliderLoEQ->maximum());
    validate_levels();
    if (m_lowEqFreq < 1000) {
        TextLoEQ->setText(QString("%1 Hz").arg((int)m_lowEqFreq));
    } else {
        TextLoEQ->setText(QString("%1 kHz").arg((int)m_lowEqFreq / 1000.));
    }
    m_pConfig->set(ConfigKey(kConfigGroup, "LoEQFrequency"),
            ConfigValue(QString::number(static_cast<int>(m_lowEqFreq))));
    m_pConfig->set(ConfigKey(kConfigGroup, "LoEQFrequencyPrecise"),
            ConfigValue(QString::number(m_lowEqFreq, 'f')));

    slotApply();
}

void DlgPrefMixer::slotApplyMainEQParameter(int value) {
    EffectSlotPointer pEffectSlot(m_pEffectMainEQ);
    if (!pEffectSlot.isNull()) {
        QSlider* slider = qobject_cast<QSlider*>(sender());
        int index = slider->property("index").toInt();
        auto pParameterSlot = pEffectSlot->getEffectParameterSlot(
                EffectManifestParameter::ParameterType::Knob, index);

        if (pParameterSlot->isLoaded()) {
            // Calculate parameter value from relative slider position
            double paramValue = static_cast<double>(value - slider->minimum()) /
                    static_cast<double>(slider->maximum() - slider->minimum());
            pParameterSlot->setParameter(paramValue);
            QLabel* valueLabel = m_mainEQValues[index];
            double dValue = value / 100.0;
            QString valueText = QString::number(dValue);
            valueLabel->setText(valueText);

            m_pConfig->set(ConfigKey(kConfigGroup,
                                   kMainEQParameterKey + QString::number(index + 1)),
                    ConfigValue(valueText));
        }
    }
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
    return static_cast<int>(dsliderPos);
}

void DlgPrefMixer::slotApply() {
    m_mode.set(m_xFaderMode);
    m_curve.set(m_transform);
    m_calibration.set(m_cal);
    if (checkBoxReverse->isChecked() != m_xFaderReverse) {
        m_reverse.set(checkBoxReverse->isChecked());
        double position = m_crossfader.get();
        m_crossfader.set(0.0 - position);
        m_xFaderReverse = checkBoxReverse->isChecked();
    }
    slotUpdateXFader();

    // EQ ////////////////////////////////////////
    m_COLoFreq.set(m_lowEqFreq);
    m_COHiFreq.set(m_highEqFreq);
    m_pConfig->set(ConfigKey(kConfigGroup, "EqAutoReset"),
            ConfigValue(m_bEqAutoReset ? 1 : 0));
    m_pConfig->set(ConfigKey(kConfigGroup, "GainAutoReset"),
            ConfigValue(m_bGainAutoReset ? 1 : 0));
    applySelections();
}

// supposed to set the widgets to match internal state
void DlgPrefMixer::slotUpdate() {
    if (radioButtonAdditive->isChecked()) {
        m_xFaderMode = MIXXX_XFADER_ADDITIVE;
    }
    if (radioButtonConstantPower->isChecked()) {
        m_xFaderMode = MIXXX_XFADER_CONSTPWR;
    }

    slotUpdateXFader();

    slotUpdateLoEQ();
    slotUpdateHiEQ();
    slotPopulateDeckEffectSelectors();
    CheckBoxEqAutoReset->setChecked(m_bEqAutoReset);
    CheckBoxGainAutoReset->setChecked(m_bGainAutoReset);

    m_eqIndiciesOnUpdate.clear();
    for (const auto& box : std::as_const(m_deckEqEffectSelectors)) {
        m_eqIndiciesOnUpdate.append(box->currentIndex());
    }
}

// Draw the crossfader curve graph. Only needs to get drawn when a change
// has been made.
void DlgPrefMixer::drawXfaderDisplay() {
    constexpr int kGrindXLines = 4;
    constexpr int kGrindYLines = 6;

    int frameWidth = graphicsViewXfader->frameWidth();
    int sizeX = graphicsViewXfader->width() - 2 * frameWidth;
    int sizeY = graphicsViewXfader->height() - 2 * frameWidth;

    // Initialize Scene
    QGraphicsScene* pXfScene = new QGraphicsScene();
    pXfScene->setSceneRect(0, 0, sizeX, sizeY);
    pXfScene->setBackgroundBrush(Qt::black);

    // Initialize QPens
    QPen gridPen(Qt::green);
    QPen graphLinePen(Qt::white);

    // draw grid
    for (int i = 1; i < kGrindXLines; i++) {
        pXfScene->addLine(
                QLineF(0, i * (sizeY / kGrindXLines), sizeX, i * (sizeY / kGrindXLines)), gridPen);
    }
    for (int i = 1; i < kGrindYLines; i++) {
        pXfScene->addLine(
                QLineF(i * (sizeX / kGrindYLines), 0, i * (sizeX / kGrindYLines), sizeY), gridPen);
    }

    // Draw graph lines
    QPointF pointTotal, point1, point2;
    QPointF pointTotalPrev, point1Prev, point2Prev;
    int pointCount = sizeX - 4;
    // reduced by 2 x 1 for border + 2 x 1 for inner distance to border
    double xfadeStep = 2. / (pointCount - 1);
    for (int i = 0; i < pointCount; i++) {
        CSAMPLE_GAIN gain1, gain2;
        EngineXfader::getXfadeGains((-1. + (xfadeStep * i)),
                m_transform,
                m_cal,
                m_xFaderMode,
                checkBoxReverse->isChecked(),
                &gain1,
                &gain2);

        double gain = sqrt(gain1 * gain1 + gain2 * gain2);
        // scale for graph
        gain1 *= 0.71f;
        gain2 *= 0.71f;
        gain *= 0.71f;

        // draw it
        pointTotal = QPointF(i + 1, (1. - gain) * (sizeY)-3);
        point1 = QPointF(i + 1, (1. - gain1) * (sizeY)-3);
        point2 = QPointF(i + 1, (1. - gain2) * (sizeY)-3);

        if (i > 0) {
            pXfScene->addLine(QLineF(pointTotal, pointTotalPrev), QPen(Qt::red));
            pXfScene->addLine(QLineF(point1, point1Prev), graphLinePen);
            pXfScene->addLine(QLineF(point2, point2Prev), graphLinePen);
        }

        // Save old values
        pointTotalPrev = pointTotal;
        point1Prev = point1;
        point2Prev = point2;
    }

    graphicsViewXfader->setScene(pXfScene);
    graphicsViewXfader->show();
    graphicsViewXfader->repaint();
}

// Update and save the crossfader's parameters from the dialog's widgets.
void DlgPrefMixer::slotUpdateXFader() {
    // m_transform is in the range of 1 to 1000 while 50 % slider results
    // to ~2, which represents a medium rounded fader curve.
    m_transform = RescalerUtils::linearToOneByX(
                          SliderXFader->value(),
                          SliderXFader->minimum(),
                          SliderXFader->maximum(),
                          EngineXfader::kTransformMax) -
            1 + EngineXfader::kTransformMin;
    m_cal = EngineXfader::getPowerCalibration(m_transform);
    m_pConfig->set(ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderMode"),
            ConfigValue(m_xFaderMode));
    m_pConfig->set(ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderCurve"),
            ConfigValue(QString::number(m_transform)));
    m_pConfig->set(ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderReverse"),
            ConfigValue(checkBoxReverse->isChecked() ? 1 : 0));

    drawXfaderDisplay();
}

void DlgPrefMixer::slotUpdateEqAutoReset(int i) {
    m_bEqAutoReset = static_cast<bool>(i);
}

void DlgPrefMixer::slotUpdateGainAutoReset(int i) {
    m_bGainAutoReset = static_cast<bool>(i);
}

void DlgPrefMixer::slotBypassEqChanged(int state) {
    bool bypass = static_cast<bool>(state);

    m_pConfig->set(ConfigKey(kConfigGroup, kEnableEqs), ConfigValue(bypass ? "no" : "yes"));
    // Disable/enable EQ effect processing for all decks by setting the appropriate
    // controls to 0 or 1 ("[EqualizerRackX_EffectUnitDeck_Effect1],enable")
    int deck = 0;
    for (const auto& box : std::as_const(m_deckEqEffectSelectors)) {
        QString group = EqualizerEffectChain::formatEffectSlotGroup(
                PlayerManager::groupForDeck(deck));
        ControlObject::set(ConfigKey(group, "enabled"), bypass ? 0 : 1);
        deck++;
        box->setEnabled(!bypass);
    }
}

void DlgPrefMixer::setUpMainEQ() {
    connect(pbResetMainEq, &QPushButton::clicked, this, &DlgPrefMixer::slotMainEQToDefault);

    connect(comboBoxMainEq,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefMixer::slotMainEqEffectChanged);

    const QString configuredEffectId = m_pConfig->getValue(ConfigKey(kConfigGroup,
                                                                   "EffectForGroup_[Master]"),
            kDefaultMainEqId);
    const EffectManifestPointer configuredEffectManifest =
            m_pBackendManager->getManifestFromUniqueId(configuredEffectId);

    const QList<EffectManifestPointer> availableMainEQEffects =
            getFilteredManifests(isMainEQ);

    // Add empty item at the top: no Main EQ
    comboBoxMainEq->addItem(kNoEffectString);
    for (const auto& pManifest : availableMainEQEffects) {
        comboBoxMainEq->addItem(pManifest->name(), QVariant(pManifest->uniqueId()));
    }
    int mainEqIndex = 0; // selects "None" by default
    if (configuredEffectManifest) {
        mainEqIndex = comboBoxMainEq->findData(configuredEffectManifest->uniqueId());
    }
    comboBoxMainEq->setCurrentIndex(mainEqIndex);

    // Load parameters from preferences:
    EffectSlotPointer pEffectSlot(m_pEffectMainEQ);
    if (!pEffectSlot.isNull()) {
        int knobNum = pEffectSlot->numParameters(EffectManifestParameter::ParameterType::Knob);
        for (int i = 0; i < knobNum; i++) {
            auto pParameterSlot = pEffectSlot->getEffectParameterSlot(
                    EffectManifestParameter::ParameterType::Knob, i);

            if (pParameterSlot->isLoaded()) {
                QString strValue = m_pConfig->getValueString(ConfigKey(kConfigGroup,
                        kMainEQParameterKey + QString::number(i + 1)));
                bool ok;
                double value = strValue.toDouble(&ok);
                if (ok) {
                    setMainEQParameter(i, value);
                }
            }
        }
    }
}

void DlgPrefMixer::slotMainEqEffectChanged(int effectIndex) {
    // clear parameters view first
    qDeleteAll(m_mainEQSliders);
    m_mainEQSliders.clear();
    qDeleteAll(m_mainEQValues);
    m_mainEQValues.clear();
    qDeleteAll(m_mainEQLabels);
    m_mainEQLabels.clear();

    const EffectManifestPointer pManifest =
            m_pBackendManager->getManifestFromUniqueId(
                    comboBoxMainEq->itemData(effectIndex).toString());

    if (pManifest == nullptr) {
        pbResetMainEq->hide();
    } else {
        pbResetMainEq->show();
    }

    auto pChainSlot = m_pEffectsManager->getOutputEffectChain();
    if (pChainSlot) {
        auto pEffectSlot = pChainSlot->getEffectSlot(0);
        if (pEffectSlot) {
            pEffectSlot->loadEffectWithDefaults(pManifest);
            pEffectSlot->setEnabled(true);
            m_pEffectMainEQ = pEffectSlot;

            int knobNum = pEffectSlot->numParameters(EffectManifestParameter::ParameterType::Knob);

            // Create and set up Main EQ's sliders
            int i;
            for (i = 0; i < knobNum; i++) {
                auto pParameterSlot = pEffectSlot->getEffectParameterSlot(
                        EffectManifestParameter::ParameterType::Knob, i);

                if (pParameterSlot->isLoaded()) {
                    EffectManifestParameterPointer pManifestParameter =
                            pParameterSlot->getManifest();

                    // Setup Label
                    QLabel* centerFreqLabel = new QLabel(this);
                    QString labelText = pParameterSlot->getManifest()->name();
                    m_mainEQLabels.append(centerFreqLabel);
                    centerFreqLabel->setText(labelText);
                    slidersGridLayout->addWidget(centerFreqLabel, 0, i + 1, Qt::AlignCenter);

                    QSlider* slider = new QSlider(this);
                    slider->setMinimum(static_cast<int>(pManifestParameter->getMinimum() * 100));
                    slider->setMaximum(static_cast<int>(pManifestParameter->getMaximum() * 100));
                    slider->setSingleStep(1);
                    slider->setValue(static_cast<int>(pManifestParameter->getDefault() * 100));
                    slider->setMinimumHeight(90);
                    // Set the index as a property because we need it inside slotUpdateFilter()
                    slider->setProperty("index", QVariant(i));
                    setScrollSafeGuard(slider);
                    slidersGridLayout->addWidget(slider, 1, i + 1, Qt::AlignCenter);
                    m_mainEQSliders.append(slider);
                    // catch drag event
                    connect(slider,
                            &QSlider::sliderMoved,
                            this,
                            &DlgPrefMixer::slotApplyMainEQParameter);
                    // catch scroll event
                    connect(slider,
                            &QSlider::valueChanged,
                            this,
                            &DlgPrefMixer::slotApplyMainEQParameter);

                    QLabel* valueLabel = new QLabel(this);
                    m_mainEQValues.append(valueLabel);
                    QString valueText = QString::number((double)slider->value() / 100);
                    valueLabel->setText(valueText);
                    slidersGridLayout->addWidget(valueLabel, 2, i + 1, Qt::AlignCenter);

                    // Immediately save the new (default) parameter values in preferences.
                    // Otherwise, without pressing 'Reset parameter', the previously saved
                    // parameter values (of another EQ effect) would be loaded on next start
                    // which (unnoticed) messes up the parameters displayed now.
                    m_pConfig->set(ConfigKey(kConfigGroup,
                                           kMainEQParameterKey + QString::number(i + 1)),
                            ConfigValue(valueText));
                }
            }
        }
    }

    // Update the configured effect for the current QComboBox
    if (pManifest) {
        m_pConfig->set(ConfigKey(kConfigGroup, "EffectForGroup_[Master]"),
                ConfigValue(pManifest->uniqueId()));
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

void DlgPrefMixer::validate_levels() {
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
    EffectSlotPointer pEffectSlot(m_pEffectMainEQ);
    if (!pEffectSlot.isNull()) {
        int knobNum = pEffectSlot->numParameters(EffectManifestParameter::ParameterType::Knob);
        for (int i = 0; i < knobNum; i++) {
            auto pParameterSlot = pEffectSlot->getEffectParameterSlot(
                    EffectManifestParameter::ParameterType::Knob, i);
            if (pParameterSlot->isLoaded()) {
                double defaultValue = pParameterSlot->getManifest()->getDefault();
                setMainEQParameter(i, defaultValue);
            }
        }
    }
}

void DlgPrefMixer::setMainEQParameter(int i, double value) {
    EffectSlotPointer pEffectSlot(m_pEffectMainEQ);
    if (!pEffectSlot.isNull()) {
        auto pParameterSlot = pEffectSlot->getEffectParameterSlot(
                EffectManifestParameter::ParameterType::Knob, i);

        if (pParameterSlot->isLoaded()) {
            QSlider* slider = m_mainEQSliders[i];
            // Calculate parameter value from relative slider position
            double paramValue = static_cast<double>(value - slider->minimum()) /
                    static_cast<double>(slider->maximum() - slider->minimum());
            pParameterSlot->setParameter(paramValue);
            slider->setValue(static_cast<int>(value * 100));

            QLabel* valueLabel = m_mainEQValues[i];
            QString valueText = QString::number(value);
            valueLabel->setText(valueText);

            m_pConfig->set(ConfigKey(kConfigGroup,
                                   kMainEQParameterKey + QString::number(i + 1)),
                    ConfigValue(valueText));
        }
    }
}

const QList<EffectManifestPointer> DlgPrefMixer::getFilteredManifests(
        EffectManifestFilterFnc filterFunc) const {
    const QList<EffectManifestPointer> allManifests =
            m_pBackendManager->getManifests();
    if (filterFunc == nullptr) {
        return allManifests;
    }

    QList<EffectManifestPointer> list;
    for (const auto& pManifest : allManifests) {
        if (filterFunc(pManifest.data())) {
            list.append(pManifest);
        }
    }
    return list;
}
