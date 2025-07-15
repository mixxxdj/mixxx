#include "preferences/dialog/dlgprefmixer.h"

#include <QButtonGroup>
#include <QPainterPath>
#include <QStandardItemModel>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "defs_urls.h"
#include "effects/chains/equalizereffectchain.h"
#include "effects/chains/quickeffectchain.h"
#include "effects/effectknobparameterslot.h"
#include "effects/effectslot.h"
#include "effects/effectsmanager.h"
#include "effects/presets/effectchainpreset.h"
#include "engine/enginexfader.h"
#include "mixer/playermanager.h"
#include "moc_dlgprefmixer.cpp"
#include "util/make_const_iterator.h"
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
#ifdef __STEM__
const ConfigKey kStemAutoResetKey = ConfigKey(kMixerProfile, QStringLiteral("stem_auto_reset"));
#endif
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

bool isMixingEQ(const EffectManifest* pManifest) {
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
          m_xFaderCurve(EngineXfader::kTransformDefault),
          m_xFaderCal(0.0),
          m_xfModeCO(make_parented<ControlProxy>(kXfaderModeKey, this)),
          m_xfCurveCO(make_parented<ControlProxy>(kXfaderCurveKey, this)),
          m_xfReverseCO(make_parented<ControlProxy>(kXfaderReverseKey, this)),
          m_xfCalibrationCO(make_parented<ControlProxy>(kXfaderCalibrationKey, this)),
          m_crossfader(QStringLiteral("[Master]"), QStringLiteral("crossfader")),
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
#ifdef __STEM__
          m_stemAutoReset(true),
#endif
          m_eqBypass(false),
          m_initializing(true),
          m_updatingMainEQ(false),
          m_applyingDeckEQs(false),
          m_applyingQuickEffects(false) {
    setupUi(this);

    // Update the crossfader curve graph and other settings when the
    // crossfader mode is changed or the slider is moved.
    connect(SliderXFader,
            QOverload<int>::of(&QSlider::valueChanged),
            this,
            &DlgPrefMixer::slotXFaderSliderChanged);
    connect(SliderXFader, &QSlider::sliderMoved, this, &DlgPrefMixer::slotXFaderSliderChanged);
    connect(SliderXFader, &QSlider::sliderReleased, this, &DlgPrefMixer::slotXFaderSliderChanged);
    connect(buttonGroupCrossfaderModes,
            &QButtonGroup::buttonClicked,
            this,
            &DlgPrefMixer::slotXFaderModeBoxToggled);
    connect(checkBoxReverse,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefMixer::slotXFaderReverseBoxToggled);

    m_xfModeCO->connectValueChanged(
            this, &DlgPrefMixer::slotXFaderModeControlChanged);
    m_xfCurveCO->connectValueChanged(
            this, &DlgPrefMixer::slotXFaderCurveControlChanged);
    m_xfCalibrationCO->connectValueChanged(
            this, &DlgPrefMixer::slotXFaderCalibrationControlChanged);
    m_xfReverseCO->connectValueChanged(
            this, &DlgPrefMixer::slotXFaderReverseControlChanged);

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
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefMixer::slotEqAutoResetToggled);
    connect(CheckBoxGainAutoReset,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefMixer::slotGainAutoResetToggled);
#ifdef __STEM__
    connect(CheckBoxStemAutoReset,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefMixer::slotStemAutoResetToggled);
#else
    CheckBoxStemAutoReset->hide();
#endif
    connect(CheckBoxBypass,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefMixer::slotBypassEqToggled);

    connect(CheckBoxEqOnly,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefMixer::slotEqOnlyToggled);

    connect(CheckBoxSingleEqEffect,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
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

void DlgPrefMixer::slotNumDecksChanged(double numDecks) {
    while (m_deckEqEffectSelectors.size() < static_cast<int>(numDecks)) {
        // 1-based for display
        int deckNo = m_deckEqEffectSelectors.size() + 1;
        // 0-based for engine
        QString deckGroup = PlayerManager::groupForDeck(deckNo - 1);
        auto pLabel = make_parented<QLabel>(QObject::tr("Deck %1").arg(deckNo), this);

        // Create the EQ selector //////////////////////////////////////////////
        auto pEqComboBox = make_parented<QComboBox>(this);
        setScrollSafeGuard(pEqComboBox);
        m_deckEqEffectSelectors.append(pEqComboBox);

        // Migrate EQ from mixxx.cfg, add it to the combobox, remove keys
        const ConfigKey groupKey =
                ConfigKey(kMixerProfile, kEffectForGroupPrefix + deckGroup);
        const QString configuredEffect = m_pConfig->getValueString(groupKey);
        // If the EQ key doesn't exist (isNull()), we keeo the default EQ which
        // has already been loaded by EffectsManager. Later on
        // slotPopulateDeckEqSelectors() will read the effect, so nothing to do.
        // Else, we use the uid from the config to load an EQ (or none).
        bool loadEQ = false;
        EffectManifestPointer pEqManifest;
        if (!configuredEffect.isNull()) {
            loadEQ = true;
            pEqManifest = m_pBackendManager->getManifestFromUniqueId(configuredEffect);

            // remove key so we migrate only once
            m_pConfig->remove(groupKey);
        }

        auto pEqChain = m_pEffectsManager->getEqualizerEffectChain(deckGroup);
        VERIFY_OR_DEBUG_ASSERT(pEqChain) {
            return;
        }
        auto pEqEffectSlot = pEqChain->getEffectSlot(0);
        VERIFY_OR_DEBUG_ASSERT(pEqEffectSlot) {
            return;
        }
        if (loadEQ) {
            pEqEffectSlot->loadEffectWithDefaults(pEqManifest);
        }
        connect(pEqComboBox,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                this,
                &DlgPrefMixer::slotEQEffectSelectionChanged);
        // Update the combobox in case the effect was changed from anywhere else.
        // This will wipe pending EQ effect changes.
        connect(pEqEffectSlot.data(),
                &EffectSlot::effectChanged,
                this,
                &DlgPrefMixer::slotPopulateDeckEqSelectors);

        // Create the QuickEffect selector /////////////////////////////////////
        auto pQuickEffectComboBox = make_parented<QComboBox>(this);
        setScrollSafeGuard(pQuickEffectComboBox);
        m_deckQuickEffectSelectors.append(pQuickEffectComboBox);
        connect(pQuickEffectComboBox,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                this,
                &DlgPrefMixer::slotQuickEffectSelectionChanged);
        // Update the combobox when the effect was changed in WEffectChainPresetSelector
        // or with controllers. This will wipe pending QuickEffect changes.
        EffectChainPointer pChain = m_pEffectsManager->getQuickEffectChain(deckGroup);
        DEBUG_ASSERT(pChain);
        connect(pChain.data(),
                &EffectChain::chainPresetChanged,
                this,
                &DlgPrefMixer::slotPopulateQuickEffectSelectors);

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

    // This also selects all currently loaded EQs and QuickEffects
    slotPopulateDeckEqSelectors();
    slotPopulateQuickEffectSelectors();

    // Ensure all newly created but unneeded widgets are hidden
    slotSingleEqToggled(m_singleEq);
}

void DlgPrefMixer::slotPopulateDeckEqSelectors() {
    if (m_applyingDeckEQs) {
        return;
    }

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
            if (pManifest.isNull()) {
                pBox->insertSeparator(pBox->count());
                continue;
            }
            pBox->addItem(pManifest->displayName(), QVariant(pManifest->uniqueId()));
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
    if (m_applyingQuickEffects) {
        return;
    }
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

            auto* quickBox = m_deckQuickEffectSelectors[deck];
            quickBox->setEnabled(true);
        }
        slotPopulateDeckEqSelectors();
        slotPopulateQuickEffectSelectors();
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
    SliderXFader->setValue(static_cast<int>(std::round(sliderVal)));

    m_xFaderMode = MIXXX_XFADER_ADDITIVE;
    radioButtonAdditive->setChecked(true);
    checkBoxReverse->setChecked(false);

    // EQ & QuickEffects //////////////////////////////////
    m_pEffectsManager->loadDefaultEqsAndQuickEffects();
    CheckBoxBypass->setChecked(false);
    CheckBoxEqOnly->setChecked(true);
    CheckBoxSingleEqEffect->setChecked(true);
    CheckBoxEqAutoReset->setChecked(false);
    CheckBoxGainAutoReset->setChecked(false);
#ifdef __STEM__
    CheckBoxStemAutoReset->setChecked(true);
#endif

    setDefaultShelves();
    comboBoxMainEq->setCurrentIndex(0); // '---' no EQ
}

void DlgPrefMixer::slotEQEffectSelectionChanged(int effectIndex) {
    Q_UNUSED(effectIndex);
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
    Q_UNUSED(effectIndex);
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

void DlgPrefMixer::applyDeckEQs() {
    m_applyingDeckEQs = true;
    m_ignoreEqQuickEffectBoxSignals = true;

    for (int deck = 0; deck < m_deckEqEffectSelectors.size(); deck++) {
        auto* pBox = m_deckEqEffectSelectors[deck];
        int effectIndex = pBox->currentIndex();

        bool needLoad = true;
        bool startingUp = m_eqIndiciesOnUpdate.size() < (deck + 1);
        if (!startingUp) {
            needLoad = effectIndex != m_eqIndiciesOnUpdate[deck];
        }

        auto pChainSlot = m_pEffectsManager->getEqualizerEffectChain(
                PlayerManager::groupForDeck(deck));
        DEBUG_ASSERT(pChainSlot);
        auto pEffectSlot = pChainSlot->getEffectSlot(0);
        DEBUG_ASSERT(pEffectSlot);
        pEffectSlot->setEnabled(!m_eqBypass);

        const EffectManifestPointer pManifest =
                m_pBackendManager->getManifestFromUniqueId(
                        pBox->currentData().toString());
        if (pManifest != nullptr && pManifest->isMixingEQ() && !m_eqBypass) {
            pChainSlot->setFilterWaveform(true);
        } else {
            pChainSlot->setFilterWaveform(false);
        }

        if (needLoad) {
            pEffectSlot->loadEffectWithDefaults(pManifest);
        }

        if (startingUp) {
            m_eqIndiciesOnUpdate.append(effectIndex);
        } else {
            m_eqIndiciesOnUpdate[deck] = effectIndex;
        }
    }
    m_ignoreEqQuickEffectBoxSignals = false;
    m_applyingDeckEQs = false;
}

void DlgPrefMixer::applyQuickEffects() {
    m_applyingQuickEffects = true;
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
    m_applyingQuickEffects = false;
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
    applyXFader();

    // EQ & QuickEffect settings ///////////////////////////////////////////////
    m_pConfig->set(kEnableEqsKey, ConfigValue(m_eqBypass ? 0 : 1));
    m_pConfig->set(kSingleEqKey, ConfigValue(m_singleEq ? 1 : 0));
    m_pConfig->set(kEqsOnlyKey, ConfigValue(m_eqEffectsOnly ? 1 : 0));
    m_pConfig->set(kEqAutoResetKey, ConfigValue(m_eqAutoReset ? 1 : 0));
    m_pConfig->set(kGainAutoResetKey, ConfigValue(m_gainAutoReset ? 1 : 0));
#ifdef __STEM__
    m_pConfig->set(kStemAutoResetKey, ConfigValue(m_stemAutoReset ? 1 : 0));
#endif
    applyDeckEQs();
    applyQuickEffects();

    storeEqShelves();
}

void DlgPrefMixer::applyXFader() {
    m_xfModeCO->set(m_xFaderMode);
    m_xfCurveCO->set(m_xFaderCurve);
    m_xfCalibrationCO->set(m_xFaderCal);
    if (m_xFaderReverse != m_xfReverseCO->toBool()) {
        double position = m_crossfader.get();
        m_crossfader.set(0.0 - position);
    }
    m_xfReverseCO->set(m_xFaderReverse ? 1.0 : 0.0);

    m_pConfig->setValue(kXfaderModeKey, m_xFaderMode);
    m_pConfig->setValue(kXfaderCurveKey, m_xFaderCurve);
    m_pConfig->setValue(kXfaderReverseKey, m_xFaderReverse);
}

void DlgPrefMixer::storeEqShelves() {
    if (m_initializing) {
        return;
    }

    m_pConfig->set(kHighEqFreqPreciseKey, ConfigValue(QString::number(m_highEqFreq, 'f')));
    m_pConfig->set(kLowEqFreqPreciseKey, ConfigValue(QString::number(m_lowEqFreq, 'f')));
}

void DlgPrefMixer::slotUpdate() {
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

#ifdef __STEM__
    m_stemAutoReset = m_pConfig->getValue(kStemAutoResetKey, true);
    CheckBoxStemAutoReset->setChecked(m_stemAutoReset);
#endif

    QString eqBaypassCfg = m_pConfig->getValueString(kEnableEqsKey);
    m_eqBypass = !(eqBaypassCfg == "yes" || eqBaypassCfg == "1" || eqBaypassCfg.isEmpty());
    CheckBoxBypass->setChecked(m_eqBypass);
    // Deactivate EQ comboboxes when Bypass is enabled
    slotBypassEqToggled(m_eqBypass);

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

void DlgPrefMixer::slotUpdateXFader() {
    // Read values from config only on first update if the xfader curve controls
    // are still at their default values. This should detect if controller mappings
    // (or skin attributes) have changed the xfader controls.
    // Else and on later calls, always read the current state from controls.
    if (m_initializing &&
            m_xfCurveCO->get() == m_xfCurveCO->getDefault() &&
            m_xfCalibrationCO->get() == m_xfCalibrationCO->getDefault() &&
            m_xfModeCO->get() == m_xfModeCO->getDefault() &&
            m_xfReverseCO->get() == m_xfReverseCO->getDefault()) {
        m_xFaderCurve = m_pConfig->getValue(kXfaderCurveKey, EngineXfader::kTransformDefault);
        // "xFaderCalibration" is not stored in the config and it's not expsoed
        // with a slider here. Each time the slider is touched it's calculated
        // to get us a smooth curve for ConstPower mode. And hos no effect for
        // Additive mode.
        // TODO This also means custom values set by controller mappings are
        // wiped on shutdown.
        m_xFaderCal = EngineXfader::getPowerCalibration(m_xFaderCurve);
        m_xFaderMode = m_pConfig->getValue<int>(kXfaderModeKey);
        m_xFaderReverse = m_pConfig->getValue<bool>(kXfaderReverseKey);
    } else {
        // Update xfader from controls
        // deactivated for now. resolve dupe debug etc.
        // slotXFaderControlChanged();
        m_xFaderCurve = m_xfCurveCO->get();
        m_xFaderCal = m_xfCalibrationCO->get();
        m_xFaderMode = static_cast<int>(m_xfModeCO->get());
        m_xFaderReverse = static_cast<bool>(m_xfReverseCO->get());
    }

    updateXFaderWidgets();
}

void DlgPrefMixer::updateXFaderWidgets() {
    const QSignalBlocker signalBlocker(this);

    // Range SliderXFader 0 .. 100
    double sliderVal = RescalerUtils::oneByXToLinear(
            m_xFaderCurve - EngineXfader::kTransformMin + 1,
            EngineXfader::kTransformMax - EngineXfader::kTransformMin + 1,
            SliderXFader->minimum(),
            SliderXFader->maximum());
    SliderXFader->setValue(static_cast<int>(std::round(sliderVal)));

    // Same here
    if (m_xFaderMode == MIXXX_XFADER_CONSTPWR) {
        radioButtonConstantPower->setChecked(true);
    } else {
        radioButtonAdditive->setChecked(true);
    }

    checkBoxReverse->setChecked(m_xFaderReverse);

    drawXfaderDisplay();
}

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
                m_xFaderCurve,
                m_xFaderCal,
                m_xFaderMode,
                m_xFaderReverse,
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

void DlgPrefMixer::slotXFaderReverseBoxToggled() {
    m_xFaderReverse = checkBoxReverse->isChecked();
}

void DlgPrefMixer::slotXFaderSliderChanged() {
    // m_xFaderCurve is in the range of 1 to 1000 while 50 % slider results
    // to ~2, which represents a medium rounded fader curve.
    double curve = RescalerUtils::linearToOneByX(
                           SliderXFader->value(),
                           SliderXFader->minimum(),
                           SliderXFader->maximum(),
                           EngineXfader::kTransformMax) -
            1 + EngineXfader::kTransformMin;
    // Round to 4 decimal places to avoid round-trip offsets with default 1.0
    m_xFaderCurve = std::round(curve * 10000) / 10000;
    // If the curve has been changed in the GUI we fetch the engine value for
    // calibration which gives us a smooth curve.
    // This wipes any previous value set by controller mappings for example.
    m_xFaderCal = EngineXfader::getPowerCalibration(m_xFaderCurve);
    drawXfaderDisplay();
}

void DlgPrefMixer::slotXFaderModeBoxToggled() {
    m_xFaderMode = radioButtonConstantPower->isChecked()
            ? MIXXX_XFADER_CONSTPWR
            : MIXXX_XFADER_ADDITIVE;

    drawXfaderDisplay();
}

void DlgPrefMixer::slotXFaderCurveControlChanged(double v) {
    if (v == m_xFaderCurve) {
        return;
    }
    m_xFaderCurve = v;
    updateXFaderWidgets();
}

void DlgPrefMixer::slotXFaderCalibrationControlChanged(double v) {
    if (v == m_xFaderCal) {
        return;
    }
    m_xFaderCal = v;
    updateXFaderWidgets();
}

void DlgPrefMixer::slotXFaderModeControlChanged(double v) {
    int mode = static_cast<int>(v);
    if (mode == m_xFaderMode) {
        return;
    }
    m_xFaderMode = mode;
    updateXFaderWidgets();
}

void DlgPrefMixer::slotXFaderReverseControlChanged(double v) {
    bool reverse = v > 0;
    if (reverse == m_xFaderReverse) {
        return;
    }
    m_xFaderReverse = reverse;
    updateXFaderWidgets();
}

void DlgPrefMixer::slotEqAutoResetToggled(bool checked) {
    m_eqAutoReset = checked;
}

void DlgPrefMixer::slotGainAutoResetToggled(bool checked) {
    m_gainAutoReset = checked;
}

#ifdef __STEM__
void DlgPrefMixer::slotStemAutoResetToggled(bool checked) {
    m_stemAutoReset = checked;
}
#endif

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

    // Populate the effect combobox and connect widgets
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
    // slotMainEqEffectChanged() applies the effect immediately, so connect _after_
    // setting the index to not override the current EffectsManager state.
    connect(pbResetMainEq, &QPushButton::clicked, this, &DlgPrefMixer::slotMainEQToDefault);
    connect(comboBoxMainEq,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefMixer::slotMainEqEffectChanged);

    // Since 2.4 the main EQ is stored in effects.xml, so try to load settings
    // from mixxx.cfg and apply immediately.
    // If no settings were read, the state from EffectsManager is adopted when
    // slotUpdate() is called later during initialization.
    const QString configuredEffectId =
            m_pConfig->getValueString(ConfigKey(kMixerProfile, kEffectGroupForMaster));
    if (configuredEffectId.isNull() || configuredEffectId.isEmpty()) {
        // Effect key doesn't exist or effect uid is empty. Nothing to do, keep
        // the state loaded from effects.xml
        // Remove all main EQ key residues
        const QList<ConfigKey> mixerKeys = m_pConfig->getKeysWithGroup(kMixerProfile);
        for (const auto& key : mixerKeys) {
            if (key.item.contains(kEffectGroupForMaster)) {
                m_pConfig->remove(key);
            }
        }
        return;
    }

    const EffectManifestPointer configuredEffectManifest =
            m_pBackendManager->getManifestFromUniqueId(configuredEffectId);
    if (!configuredEffectManifest) {
        return;
    }

    int configuredIndex = comboBoxMainEq->findData(configuredEffectManifest->uniqueId());
    if (configuredIndex == -1) {
        return;
    }
    // Set index and create required sliders and labels
    comboBoxMainEq->setCurrentIndex(configuredIndex);

    // Load parameters from preferences and set sliders
    for (QSlider* pSlider : std::as_const(m_mainEQSliders)) {
        int paramIndex = pSlider->property("index").toInt();
        QString strValue = m_pConfig->getValueString(
                ConfigKey(kMixerProfile,
                        kMainEQParameterKey + QString::number(paramIndex + 1)));
        bool ok;
        double paramValue = strValue.toDouble(&ok);
        if (!ok) {
            continue;
        }
        pSlider->setValue(static_cast<int>(paramValue * 100));
        // The label is updated in slotMainEQParameterSliderChanged()
    }
    // Remove all main EQ keys
    const QList<ConfigKey> mixerKeys = m_pConfig->getKeysWithGroup(kMixerProfile);
    for (const auto& key : mixerKeys) {
        if (key.item.contains(kEffectGroupForMaster)) {
            m_pConfig->remove(key);
        }
    }
}

void DlgPrefMixer::updateMainEQ() {
    EffectSlotPointer pEffectSlot(m_pEffectMainEQ);
    VERIFY_OR_DEBUG_ASSERT(pEffectSlot) {
        return;
    }

    // Read loaded EQ from EffectsManager
    const EffectManifestPointer pLoadedManifest = pEffectSlot->getManifest();
    int loadedIndex = pLoadedManifest ? comboBoxMainEq->findData(pLoadedManifest->uniqueId()) : 0;
    if (loadedIndex != comboBoxMainEq->currentIndex()) {
        m_updatingMainEQ = true;
    }
    // Set index and create required sliders and labels
    comboBoxMainEq->setCurrentIndex(loadedIndex);

    for (QSlider* pSlider : std::as_const(m_mainEQSliders)) {
        int paramIndex = pSlider->property("index").toInt();
        auto pParameterSlot = pEffectSlot->getEffectParameterSlot(
                EffectManifestParameter::ParameterType::Knob, paramIndex);
        VERIFY_OR_DEBUG_ASSERT(pParameterSlot && pParameterSlot->isLoaded()) {
            return;
        }
        auto pKnobSlot = qobject_cast<EffectKnobParameterSlot*>(pParameterSlot);
        VERIFY_OR_DEBUG_ASSERT(pKnobSlot) {
            return;
        }
        double pValue = pKnobSlot->getValueParameter();
        int sValue = static_cast<int>(
                             pValue * (pSlider->maximum() - pSlider->minimum())) +
                pSlider->minimum();
        pSlider->setValue(sValue);
        // Label is updated in  slotMainEQParameterSliderChanged()
    }
    m_updatingMainEQ = false;
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

    if (!m_updatingMainEQ) {
        pEffectSlot->loadEffectWithDefaults(pManifest);
    }

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
        // Store the index as a property because we need it in
        // slotMainEQParameterSliderChanged() and slotUpdate()
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

const QList<EffectManifestPointer> DlgPrefMixer::getDeckEqManifests() const {
    QList<EffectManifestPointer> allManifests =
            m_pBackendManager->getManifests();
    auto nonEqsStartIt = std::stable_partition(allManifests.begin(),
            allManifests.end(),
            [](const auto& pManifest) { return isMixingEQ(pManifest.data()); });
    if (m_eqEffectsOnly) {
        erase(&allManifests, nonEqsStartIt, allManifests.end());
    } else {
        // Add a null item between EQs and non-EQs. The combobox fill function
        // will use this to insert a separator.
        insert(&allManifests, nonEqsStartIt, EffectManifestPointer());
    }
    return allManifests;
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
