#include "preferences/dialog/dlgprefeq.h"

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
#include "mixer/playermanager.h"
#include "moc_dlgprefeq.cpp"

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

DlgPrefEQ::DlgPrefEQ(
        QWidget* pParent,
        std::shared_ptr<EffectsManager> pEffectsManager,
        UserSettingsPointer pConfig)
        : DlgPreferencePage(pParent),
          m_COLoFreq(kConfigGroup, QStringLiteral("LoEQFrequency")),
          m_COHiFreq(kConfigGroup, QStringLiteral("HiEQFrequency")),
          m_pConfig(pConfig),
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
    // Set the focus policy for comboboxes and sliders and connect them to the
    // custom event filter. See eventFilter() for details.
    // Deck EQ & QuickEffect comboxboxes are set up accordingly in slotNumDecksChanged(),
    // main EQ sliders in slotMainEqEffectChanged()
    SliderHiEQ->setFocusPolicy(Qt::StrongFocus);
    SliderHiEQ->installEventFilter(this);
    SliderLoEQ->setFocusPolicy(Qt::StrongFocus);
    SliderLoEQ->installEventFilter(this);
    comboBoxMainEq->setFocusPolicy(Qt::StrongFocus);
    comboBoxMainEq->installEventFilter(this);

    loadSettings();

    // Connection
    connect(SliderHiEQ, &QSlider::valueChanged, this, &DlgPrefEQ::slotUpdateHiEQ);
    connect(SliderHiEQ, &QSlider::sliderMoved, this, &DlgPrefEQ::slotUpdateHiEQ);
    connect(SliderHiEQ, &QSlider::sliderReleased, this, &DlgPrefEQ::slotUpdateHiEQ);

    connect(SliderLoEQ, &QSlider::valueChanged, this, &DlgPrefEQ::slotUpdateLoEQ);
    connect(SliderLoEQ, &QSlider::sliderMoved, this, &DlgPrefEQ::slotUpdateLoEQ);
    connect(SliderLoEQ, &QSlider::sliderReleased, this, &DlgPrefEQ::slotUpdateLoEQ);

    connect(CheckBoxEqAutoReset, &QCheckBox::stateChanged, this, &DlgPrefEQ::slotUpdateEqAutoReset);
    connect(CheckBoxGainAutoReset,
            &QCheckBox::stateChanged,
            this,
            &DlgPrefEQ::slotUpdateGainAutoReset);
    connect(CheckBoxBypass, &QCheckBox::stateChanged, this, &DlgPrefEQ::slotBypass);

    connect(CheckBoxEqOnly,
            &QCheckBox::stateChanged,
            this,
            &DlgPrefEQ::slotPopulateDeckEffectSelectors);

    connect(CheckBoxSingleEqEffect,
            &QCheckBox::stateChanged,
            this,
            &DlgPrefEQ::slotSingleEqChecked);
    // Quick hack to update the checkbox "Use the same EQ filter for all decks"
    // to not use the default state (checked) when slotNumDecksChanged() calls
    // slotSingleEqChecked(state) here in constructor, because that would be written
    // to config immediateley and thus reset the previous unchecked state.
    // TODO(ronso0) Write only in slotApply(), read from config only in slotUpdate().
    // Currently config is read in both slotUpdate() and loadSettings().
    CheckBoxSingleEqEffect->setChecked(m_pConfig->getValue(
                                               ConfigKey(kConfigKey, kSingleEq), "yes") == "yes");
    slotSingleEqChecked(CheckBoxSingleEqEffect->isChecked());

    // Add drop down lists for current decks and connect num_decks control
    // to slotNumDecksChanged
    m_pNumDecks = new ControlProxy("[Master]", "num_decks", this);
    m_pNumDecks->connectValueChanged(this, &DlgPrefEQ::slotNumDecksChanged);
    slotNumDecksChanged(m_pNumDecks->get());

    connect(m_pChainPresetManager.data(),
            &EffectChainPresetManager::quickEffectChainPresetListUpdated,
            this,
            &DlgPrefEQ::slotPopulateDeckEffectSelectors);

    setUpMainEQ();

    slotUpdate();
    slotApply();
}

DlgPrefEQ::~DlgPrefEQ() {
    qDeleteAll(m_deckEqEffectSelectors);
    m_deckEqEffectSelectors.clear();

    qDeleteAll(m_deckQuickEffectSelectors);
    m_deckQuickEffectSelectors.clear();
}

// Catch scroll events and filter them if they addressed an unfocused combobox or
// silder and send them to the scroll area instead.
// This avoids undesired value changes of unfocused widget when scrolling the page.
// Values can be changed only by explicit selection, dragging sliders and with
// Up/Down (Left/Right respectively), PageUp/PageDown as well as Home/End keys.
bool DlgPrefEQ::eventFilter(QObject* obj, QEvent* e) {
    if (e->type() == QEvent::Wheel) {
        // Reject scrolling only if widget is unfocused.
        // Object to widget cast is needed to check the focus state.
        QComboBox* combo = qobject_cast<QComboBox*>(obj);
        QSlider* slider = qobject_cast<QSlider*>(obj);
        if ((combo && !combo->hasFocus()) || (slider && !slider->hasFocus())) {
            QApplication::sendEvent(verticalLayout, e);
            return true;
        }
    }
    return QObject::eventFilter(obj, e);
}

void DlgPrefEQ::slotNumDecksChanged(double numDecks) {
    int oldDecks = m_deckEqEffectSelectors.size();
    while (m_deckEqEffectSelectors.size() < static_cast<int>(numDecks)) {
        int deckNo = m_deckEqEffectSelectors.size() + 1;

        QLabel* label = new QLabel(QObject::tr("Deck %1").arg(deckNo), this);

        // Create the drop down list for deck EQs
        QComboBox* pEqComboBox = new QComboBox(this);
        // Ignore scroll events if combobox is not focused.
        // See eventFilter() for details.
        pEqComboBox->setFocusPolicy(Qt::StrongFocus);
        pEqComboBox->installEventFilter(this);
        m_deckEqEffectSelectors.append(pEqComboBox);
        connect(pEqComboBox,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                this,
                &DlgPrefEQ::slotEffectChangedOnDeck);

        // Create the drop down list for Quick Effects
        QComboBox* pQuickEffectComboBox = new QComboBox(this);
        pQuickEffectComboBox->setFocusPolicy(Qt::StrongFocus);
        pQuickEffectComboBox->installEventFilter(this);
        m_deckQuickEffectSelectors.append(pQuickEffectComboBox);
        connect(pQuickEffectComboBox,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                this,
                &DlgPrefEQ::slotQuickEffectChangedOnDeck);

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
            if (CheckBoxEqOnly->isChecked()) {
                m_firstSelectorLabel->clear();
            }
        }

        // Setup the GUI
        gridLayout_3->addWidget(label, deckNo, 0);
        gridLayout_3->addWidget(pEqComboBox, deckNo, 1);
        gridLayout_3->addWidget(pQuickEffectComboBox, deckNo, 2);
        gridLayout_3->addItem(
                new QSpacerItem(
                        40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum),
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
    slotSingleEqChecked(CheckBoxSingleEqEffect->isChecked());
}

void DlgPrefEQ::slotPopulateDeckEffectSelectors() {
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

void DlgPrefEQ::populateDeckEqBoxList(
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

void DlgPrefEQ::populateDeckQuickEffectBoxList(
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

        // Add empty item at the top: no effect
        box->addItem(kNoEffectString);
        int i = 1;
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

void DlgPrefEQ::slotSingleEqChecked(int checked) {
    bool do_hide = static_cast<bool>(checked);
    m_pConfig->set(ConfigKey(kConfigGroup, kSingleEq),
            do_hide ? QString("yes") : QString("no"));
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

    if (m_firstSelectorLabel != nullptr) {
        if (do_hide) {
            m_firstSelectorLabel->clear();
        } else {
            m_firstSelectorLabel->setText(QObject::tr("Deck 1"));
        }
    }

    applySelections();
}

QUrl DlgPrefEQ::helpUrl() const {
    return QUrl(MIXXX_MANUAL_EQ_URL);
}

void DlgPrefEQ::loadSettings() {
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

void DlgPrefEQ::setDefaultShelves() {
    m_pConfig->set(ConfigKey(kConfigGroup, "HiEQFrequency"), ConfigValue(2500));
    m_pConfig->set(ConfigKey(kConfigGroup, "LoEQFrequency"), ConfigValue(250));
    m_pConfig->set(ConfigKey(kConfigGroup, "HiEQFrequencyPrecise"), ConfigValue(2500.0));
    m_pConfig->set(ConfigKey(kConfigGroup, "LoEQFrequencyPrecise"), ConfigValue(250.0));
}

void DlgPrefEQ::slotResetToDefaults() {
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

void DlgPrefEQ::slotEffectChangedOnDeck(int effectIndex) {
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

void DlgPrefEQ::slotQuickEffectChangedOnDeck(int effectIndex) {
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
    if (pChain && effectIndex > 0 && effectIndex <= presetList.size()) {
        pChain->loadChainPreset(presetList[effectIndex - 1]);
    }
}

void DlgPrefEQ::applySelections() {
    if (m_inSlotPopulateDeckEffectSelectors) {
        return;
    }

    applySelectionsToDecks();
}

void DlgPrefEQ::applySelectionsToDecks() {
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

void DlgPrefEQ::slotUpdateHiEQ() {
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

void DlgPrefEQ::slotUpdateLoEQ() {
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

void DlgPrefEQ::slotApplyMainEQParameter(int value) {
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

int DlgPrefEQ::getSliderPosition(double eqFreq, int minValue, int maxValue) {
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

void DlgPrefEQ::slotApply() {
    m_COLoFreq.set(m_lowEqFreq);
    m_COHiFreq.set(m_highEqFreq);
    m_pConfig->set(ConfigKey(kConfigGroup, "EqAutoReset"),
            ConfigValue(m_bEqAutoReset ? 1 : 0));
    m_pConfig->set(ConfigKey(kConfigGroup, "GainAutoReset"),
            ConfigValue(m_bGainAutoReset ? 1 : 0));
    applySelections();
}

// supposed to set the widgets to match internal state
void DlgPrefEQ::slotUpdate() {
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

void DlgPrefEQ::slotUpdateEqAutoReset(int i) {
    m_bEqAutoReset = static_cast<bool>(i);
}

void DlgPrefEQ::slotUpdateGainAutoReset(int i) {
    m_bGainAutoReset = static_cast<bool>(i);
}

void DlgPrefEQ::slotBypass(int state) {
    if (state) {
        m_pConfig->set(ConfigKey(kConfigGroup, kEnableEqs), QString("no"));
        // Disable effect processing for all decks by setting the appropriate
        // controls to 0 ("[EqualizerRackX_EffectUnitDeck_Effect1],enable")
        int deck = 0;
        for (const auto& box : std::as_const(m_deckEqEffectSelectors)) {
            QString group = getEQEffectGroupForDeck(deck);
            ControlObject::set(ConfigKey(group, "enabled"), 0);
            deck++;
            box->setEnabled(false);
        }
    } else {
        m_pConfig->set(ConfigKey(kConfigGroup, kEnableEqs), QString("yes"));
        // Enable effect processing for all decks by setting the appropriate
        // controls to 1 ("[EqualizerRackX_EffectUnitDeck_Effect1],enable")
        int deck = 0;
        for (const auto& box : std::as_const(m_deckEqEffectSelectors)) {
            QString group = getEQEffectGroupForDeck(deck);
            ControlObject::set(ConfigKey(group, "enabled"), 1);
            deck++;
            box->setEnabled(true);
        }
    }

    slotApply();
}

void DlgPrefEQ::setUpMainEQ() {
    connect(pbResetMainEq, &QPushButton::clicked, this, &DlgPrefEQ::slotMainEQToDefault);

    connect(comboBoxMainEq,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefEQ::slotMainEqEffectChanged);

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

void DlgPrefEQ::slotMainEqEffectChanged(int effectIndex) {
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
                    // Ignore scroll events if slider is not focused.
                    // See eventFilter() for details.
                    slider->setFocusPolicy(Qt::StrongFocus);
                    slider->installEventFilter(this);
                    slidersGridLayout->addWidget(slider, 1, i + 1, Qt::AlignCenter);
                    m_mainEQSliders.append(slider);
                    // catch drag event
                    connect(slider,
                            &QSlider::sliderMoved,
                            this,
                            &DlgPrefEQ::slotApplyMainEQParameter);
                    // catch scroll event
                    connect(slider,
                            &QSlider::valueChanged,
                            this,
                            &DlgPrefEQ::slotApplyMainEQParameter);

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

double DlgPrefEQ::getEqFreq(int sliderVal, int minValue, int maxValue) {
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

void DlgPrefEQ::validate_levels() {
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

QString DlgPrefEQ::getEQEffectGroupForDeck(int deck) const {
    return EqualizerEffectChain::formatEffectSlotGroup(
            PlayerManager::groupForDeck(deck));
}

void DlgPrefEQ::slotMainEQToDefault() {
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

void DlgPrefEQ::setMainEQParameter(int i, double value) {
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

const QList<EffectManifestPointer> DlgPrefEQ::getFilteredManifests(
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
