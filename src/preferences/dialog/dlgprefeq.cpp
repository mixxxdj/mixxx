#include "preferences/dialog/dlgprefeq.h"

#include <QHBoxLayout>
#include <QString>
#include <QWidget>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "defs_urls.h"
#include "effects/builtin/biquadfullkilleqeffect.h"
#include "effects/builtin/filtereffect.h"
#include "effects/effectrack.h"
#include "effects/effectslot.h"
#include "engine/filters/enginefilterbessel4.h"
#include "mixer/playermanager.h"
#include "moc_dlgprefeq.cpp"
#include "util/math.h"

const QString kConfigKey = "[Mixer Profile]";
const QString kEnableEqs = "EnableEQs";
const QString kEqsOnly = "EQsOnly";
const QString kSingleEq = "SingleEQEffect";
const QString kDefaultEqId = BiquadFullKillEQEffect::getId();
const QString kDefaultMasterEqId = QString();
const QString kDefaultQuickEffectId = FilterEffect::getId();

const int kFrequencyUpperLimit = 20050;
const int kFrequencyLowerLimit = 16;

DlgPrefEQ::DlgPrefEQ(
        QWidget* pParent,
        std::shared_ptr<EffectsManager> pEffectsManager,
        UserSettingsPointer pConfig)
        : DlgPreferencePage(pParent),
          m_COLoFreq(kConfigKey, "LoEQFrequency"),
          m_COHiFreq(kConfigKey, "HiEQFrequency"),
          m_pConfig(pConfig),
          m_lowEqFreq(0.0),
          m_highEqFreq(0.0),
          m_pEffectsManager(pEffectsManager),
          m_firstSelectorLabel(nullptr),
          m_pNumDecks(nullptr),
          m_inSlotPopulateDeckEffectSelectors(false),
          m_bEqAutoReset(false),
          m_bGainAutoReset(false) {
    m_pEQEffectRack = m_pEffectsManager->getEqualizerRack(0);
    m_pQuickEffectRack = m_pEffectsManager->getQuickEffectRack(0);
    m_pOutputEffectRack = m_pEffectsManager->getOutputsEffectRack();

    setupUi(this);
    // Connection
    connect(SliderHiEQ, &QAbstractSlider::valueChanged, this, &DlgPrefEQ::slotUpdateHiEQ);
    connect(SliderHiEQ, &QAbstractSlider::sliderMoved, this, &DlgPrefEQ::slotUpdateHiEQ);
    connect(SliderHiEQ, &QAbstractSlider::sliderReleased, this, &DlgPrefEQ::slotUpdateHiEQ);

    connect(SliderLoEQ, &QAbstractSlider::valueChanged, this, &DlgPrefEQ::slotUpdateLoEQ);
    connect(SliderLoEQ, &QAbstractSlider::sliderMoved, this, &DlgPrefEQ::slotUpdateLoEQ);
    connect(SliderLoEQ, &QAbstractSlider::sliderReleased, this, &DlgPrefEQ::slotUpdateLoEQ);

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

    // Add drop down lists for current decks and connect num_decks control
    // to slotNumDecksChanged
    m_pNumDecks = new ControlProxy("[Master]", "num_decks", this);
    m_pNumDecks->connectValueChanged(this, &DlgPrefEQ::slotNumDecksChanged);
    slotNumDecksChanged(m_pNumDecks->get());

    setUpMasterEQ();

    loadSettings();
    slotUpdate();
    slotApply();
}

DlgPrefEQ::~DlgPrefEQ() {
    qDeleteAll(m_deckEqEffectSelectors);
    m_deckEqEffectSelectors.clear();

    qDeleteAll(m_deckQuickEffectSelectors);
    m_deckQuickEffectSelectors.clear();

    qDeleteAll(m_filterWaveformEnableCOs);
    m_filterWaveformEnableCOs.clear();
}

void DlgPrefEQ::slotNumDecksChanged(double numDecks) {
    int oldDecks = m_deckEqEffectSelectors.size();
    while (m_deckEqEffectSelectors.size() < static_cast<int>(numDecks)) {
        int deckNo = m_deckEqEffectSelectors.size() + 1;

        QLabel* label = new QLabel(QObject::tr("Deck %1 EQ Effect").
                             arg(deckNo), this);

        QString group = PlayerManager::groupForDeck(
                m_deckEqEffectSelectors.size());

        m_filterWaveformEnableCOs.append(
                new ControlObject(ConfigKey(group, "filterWaveformEnable")));
        m_filterWaveformEffectLoaded.append(false);

        // Create the drop down list for EQs
        QComboBox* eqComboBox = new QComboBox(this);
        m_deckEqEffectSelectors.append(eqComboBox);
        connect(eqComboBox,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                this,
                &DlgPrefEQ::slotEqEffectChangedOnDeck);

        // Create the drop down list for EQs
        QComboBox* quickEffectComboBox = new QComboBox(this);
        m_deckQuickEffectSelectors.append(quickEffectComboBox);
        connect(quickEffectComboBox,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                this,
                &DlgPrefEQ::slotQuickEffectChangedOnDeck);

        if (deckNo == 1) {
            m_firstSelectorLabel = label;
            if (CheckBoxEqOnly->isChecked()) {
                m_firstSelectorLabel->setText(QObject::tr("EQ Effect"));
            }
        }

        // Setup the GUI
        gridLayout_3->addWidget(label, deckNo, 0);
        gridLayout_3->addWidget(eqComboBox, deckNo, 1);
        gridLayout_3->addWidget(quickEffectComboBox, deckNo, 2);
        gridLayout_3->addItem(new QSpacerItem(
                40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum),
                deckNo, 3, 1, 1);
    }
    slotPopulateDeckEffectSelectors();
    for (int i = oldDecks; i < static_cast<int>(numDecks); ++i) {
        // Set the configured effect for box and simpleBox or Bessel8 LV-Mix EQ
        // if none is configured
        QString group = PlayerManager::groupForDeck(i);
        QString configuredEffect = m_pConfig->getValue(ConfigKey(kConfigKey,
                "EffectForGroup_" + group), kDefaultEqId);
        int selectedEffectIndex = m_deckEqEffectSelectors[i]->findData(configuredEffect);
        if (selectedEffectIndex < 0) {
            selectedEffectIndex = m_deckEqEffectSelectors[i]->findData(kDefaultEqId);
            configuredEffect = kDefaultEqId;
        }
        m_deckEqEffectSelectors[i]->setCurrentIndex(selectedEffectIndex);
        m_filterWaveformEffectLoaded[i] = m_pEffectsManager->isEQ(configuredEffect);
        m_filterWaveformEnableCOs[i]->set(
                m_filterWaveformEffectLoaded[i] &&
                !CheckBoxBypass->checkState());

        QString configuredQuickEffect = m_pConfig->getValue(ConfigKey(kConfigKey,
                "QuickEffectForGroup_" + group), kDefaultQuickEffectId);
        int selectedQuickEffectIndex =
                m_deckQuickEffectSelectors[i]->findData(configuredQuickEffect);
        if (selectedQuickEffectIndex < 0) {
            selectedQuickEffectIndex =
                    m_deckEqEffectSelectors[i]->findData(kDefaultQuickEffectId);
            configuredEffect = kDefaultQuickEffectId;
        }
        m_deckQuickEffectSelectors[i]->setCurrentIndex(selectedQuickEffectIndex);
    }
    applySelections();
    slotSingleEqChecked(CheckBoxSingleEqEffect->isChecked());
}

static bool isMixingEQ(EffectManifest* pManifest) {
    return pManifest->isMixingEQ();
}

static bool isMasterEQ(EffectManifest* pManifest) {
    return pManifest->isMasterEQ();
}

static bool hasSuperKnobLinking(EffectManifest* pManifest) {
    for (const auto& pParameterManifest : pManifest->parameters()) {
        if (pParameterManifest->defaultLinkType() !=
            EffectManifestParameter::LinkType::NONE) {
            return true;
        }
    }
    return false;
}

void DlgPrefEQ::slotPopulateDeckEffectSelectors() {
    m_inSlotPopulateDeckEffectSelectors = true; // prevents a recursive call

    EffectsManager::EffectManifestFilterFnc filterEQ;
    if (CheckBoxEqOnly->isChecked()) {
        m_pConfig->set(ConfigKey(kConfigKey, kEqsOnly), QString("yes"));
        filterEQ = isMixingEQ;
    } else {
        m_pConfig->set(ConfigKey(kConfigKey, kEqsOnly), QString("no"));
        filterEQ = nullptr; // take all
    }

    const QList<EffectManifestPointer> availableEQEffects =
        m_pEffectsManager->getAvailableEffectManifestsFiltered(filterEQ);
    const QList<EffectManifestPointer> availableQuickEffects =
        m_pEffectsManager->getAvailableEffectManifestsFiltered(hasSuperKnobLinking);

    for (QComboBox* box : qAsConst(m_deckEqEffectSelectors)) {
        // Populate comboboxes with all available effects
        // Save current selection
        QString selectedEffectId = box->itemData(box->currentIndex()).toString();
        QString selectedEffectName = box->itemText(box->currentIndex());
        box->clear();
        int currentIndex = -1; // Nothing selected

        int i;
        for (i = 0; i < availableEQEffects.size(); ++i) {
            EffectManifestPointer pManifest = availableEQEffects.at(i);
            box->addItem(pManifest->name(), QVariant(pManifest->id()));
            if (selectedEffectId == pManifest->id()) {
                currentIndex = i;
            }
        }
        //: Displayed when no effect is selected
        box->addItem(tr("None"), QVariant(QString("")));
        if (selectedEffectId.isEmpty()) {
            currentIndex = availableEQEffects.size(); // selects "None"
        } else if (currentIndex < 0 && !selectedEffectName.isEmpty() ) {
            // current selection is not part of the new list
            // So we need to add it
            box->addItem(selectedEffectName, QVariant(selectedEffectId));
            currentIndex = i + 1;
        }
        box->setCurrentIndex(currentIndex);
    }

    for (QComboBox* box : qAsConst(m_deckQuickEffectSelectors)) {
        // Populate comboboxes with all available effects
        // Save current selection
        QString selectedEffectId = box->itemData(box->currentIndex()).toString();
        QString selectedEffectName = box->itemText(box->currentIndex());
        box->clear();
        int currentIndex = -1;// Nothing selected

        int i;
        for (i = 0; i < availableQuickEffects.size(); ++i) {
            EffectManifestPointer pManifest = availableQuickEffects.at(i);
            box->addItem(pManifest->name(), QVariant(pManifest->id()));
            if (selectedEffectId == pManifest->id()) {
                currentIndex = i;
            }
        }
        //: Displayed when no effect is selected
        box->addItem(tr("None"), QVariant(QString("")));
        if (selectedEffectId.isEmpty()) {
            currentIndex = availableQuickEffects.size(); // selects "None"
        } else if (currentIndex < 0 && !selectedEffectName.isEmpty()) {
            // current selection is not part of the new list
            // So we need to add it
            box->addItem(selectedEffectName, QVariant(selectedEffectId));
            currentIndex = i + 1;
        }
        box->setCurrentIndex(currentIndex);
    }

    m_inSlotPopulateDeckEffectSelectors = false;
}

void DlgPrefEQ::slotSingleEqChecked(int checked) {
    bool do_hide = static_cast<bool>(checked);
    m_pConfig->set(ConfigKey(kConfigKey, kSingleEq),
                   do_hide ? QString("yes") : QString("no"));
    for (int i = 2; i < m_deckEqEffectSelectors.size() + 1; ++i) {
        if (do_hide) {
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
            m_firstSelectorLabel->setText(QObject::tr("EQ Effect"));
        } else {
            m_firstSelectorLabel->setText(QObject::tr("Deck 1 EQ Effect"));
        }
    }

    applySelections();
}

QUrl DlgPrefEQ::helpUrl() const {
    return QUrl(MIXXX_MANUAL_EQ_URL);
}

void DlgPrefEQ::loadSettings() {
    QString highEqCourse = m_pConfig->getValueString(ConfigKey(kConfigKey, "HiEQFrequency"));
    QString highEqPrecise = m_pConfig->getValueString(ConfigKey(kConfigKey, "HiEQFrequencyPrecise"));
    QString lowEqCourse = m_pConfig->getValueString(ConfigKey(kConfigKey, "LoEQFrequency"));
    QString lowEqPrecise = m_pConfig->getValueString(ConfigKey(kConfigKey, "LoEQFrequencyPrecise"));
    m_bEqAutoReset = static_cast<bool>(m_pConfig->getValueString(
            ConfigKey(kConfigKey, "EqAutoReset")).toInt());
    CheckBoxEqAutoReset->setChecked(m_bEqAutoReset);
    m_bGainAutoReset = static_cast<bool>(m_pConfig->getValueString(
            ConfigKey(kConfigKey, "GainAutoReset")).toInt());
    CheckBoxGainAutoReset->setChecked(m_bGainAutoReset);
    CheckBoxBypass->setChecked(m_pConfig->getValue(
            ConfigKey(kConfigKey, kEnableEqs), QString("yes")) == "no");
    CheckBoxEqOnly->setChecked(m_pConfig->getValue(
            ConfigKey(kConfigKey, kEqsOnly), "yes") == "yes");
    CheckBoxSingleEqEffect->setChecked(m_pConfig->getValue(
            ConfigKey(kConfigKey, kSingleEq), "yes") == "yes");
    slotSingleEqChecked(CheckBoxSingleEqEffect->isChecked());

    double lowEqFreq = 0.0;
    double highEqFreq = 0.0;

    // Precise takes precedence over course.
    lowEqFreq = lowEqCourse.isEmpty() ? lowEqFreq : lowEqCourse.toDouble();
    lowEqFreq = lowEqPrecise.isEmpty() ? lowEqFreq : lowEqPrecise.toDouble();
    highEqFreq = highEqCourse.isEmpty() ? highEqFreq : highEqCourse.toDouble();
    highEqFreq = highEqPrecise.isEmpty() ? highEqFreq : highEqPrecise.toDouble();

    if (lowEqFreq == 0.0 || highEqFreq == 0.0 || lowEqFreq == highEqFreq) {
        setDefaultShelves();
        lowEqFreq = m_pConfig->getValueString(ConfigKey(kConfigKey, "LoEQFrequencyPrecise")).toDouble();
        highEqFreq = m_pConfig->getValueString(ConfigKey(kConfigKey, "HiEQFrequencyPrecise")).toDouble();
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
            ConfigKey(kConfigKey, kEnableEqs), "yes") == "yes") {
        CheckBoxBypass->setChecked(false);
    }
}

void DlgPrefEQ::setDefaultShelves()
{
    m_pConfig->set(ConfigKey(kConfigKey, "HiEQFrequency"), ConfigValue(2500));
    m_pConfig->set(ConfigKey(kConfigKey, "LoEQFrequency"), ConfigValue(250));
    m_pConfig->set(ConfigKey(kConfigKey, "HiEQFrequencyPrecise"), ConfigValue(2500.0));
    m_pConfig->set(ConfigKey(kConfigKey, "LoEQFrequencyPrecise"), ConfigValue(250.0));
}

void DlgPrefEQ::slotResetToDefaults() {
    slotMasterEQToDefault();
    setDefaultShelves();
    foreach(QComboBox* pCombo, m_deckEqEffectSelectors) {
        pCombo->setCurrentIndex(
               pCombo->findData(kDefaultEqId));
    }
    foreach(QComboBox* pCombo, m_deckQuickEffectSelectors) {
        pCombo->setCurrentIndex(
               pCombo->findData(kDefaultQuickEffectId));
    }
    loadSettings();
    CheckBoxBypass->setChecked(false);
    CheckBoxEqOnly->setChecked(true);
    CheckBoxSingleEqEffect->setChecked(true);
    m_bEqAutoReset = false;
    CheckBoxEqAutoReset->setChecked(false);
    m_bGainAutoReset = false;
    CheckBoxGainAutoReset->setChecked(false);
    slotUpdate();
    slotApply();
}

void DlgPrefEQ::slotEqEffectChangedOnDeck(int effectIndex) {
    QComboBox* c = qobject_cast<QComboBox*>(sender());
    // Check if qobject_cast was successful
    if (c && !m_inSlotPopulateDeckEffectSelectors) {
        int deckNumber = m_deckEqEffectSelectors.indexOf(c);

        // If we are in single-effect mode and the first effect was changed,
        // change the others as well.
        if (deckNumber == 0 && CheckBoxSingleEqEffect->isChecked()) {
            for (int otherDeck = 1;
                    otherDeck < static_cast<int>(m_pNumDecks->get());
                    ++otherDeck) {
                QComboBox* box = m_deckEqEffectSelectors[otherDeck];
                box->setCurrentIndex(effectIndex);
            }
        }

        // This is required to remove a previous selected effect that does not
        // fit to the current ShowAllEffects checkbox
        slotPopulateDeckEffectSelectors();
    }
}

void DlgPrefEQ::slotQuickEffectChangedOnDeck(int effectIndex) {
    QComboBox* c = qobject_cast<QComboBox*>(sender());
    // Check if qobject_cast was successful
    if (c && !m_inSlotPopulateDeckEffectSelectors) {
        int deckNumber = m_deckQuickEffectSelectors.indexOf(c);

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

        // This is required to remove a previous selected effect that does not
        // fit to the current ShowAllEffects checkbox
        slotPopulateDeckEffectSelectors();
    }
}

void DlgPrefEQ::applySelections() {
    if (m_inSlotPopulateDeckEffectSelectors) {
        return;
    }

    int deck = 0;
    QString firstEffectId;
    int firstEffectIndex = 0;
    for (QComboBox* box : qAsConst(m_deckEqEffectSelectors)) {
        QString effectId = box->itemData(box->currentIndex()).toString();
        if (deck == 0) {
            firstEffectId = effectId;
            firstEffectIndex = box->currentIndex();
        } else if (CheckBoxSingleEqEffect->isChecked()) {
            effectId = firstEffectId;
            box->setCurrentIndex(firstEffectIndex);
        }
        QString group = PlayerManager::groupForDeck(deck);

        // Only apply the effect if it changed -- so first interrogate the
        // loaded effect if any.
        bool need_load = true;
        if (m_pEQEffectRack->numEffectChainSlots() > deck) {
            // It's not correct to get a chainslot by index number -- get by
            // group name instead.
            EffectChainSlotPointer chainslot =
                    m_pEQEffectRack->getGroupEffectChainSlot(group);
            if (chainslot && chainslot->numSlots()) {
                EffectPointer effectpointer =
                        chainslot->getEffectSlot(0)->getEffect();
                if (effectpointer &&
                        effectpointer->getManifest()->id() == effectId) {
                    need_load = false;
                }
            }
        }
        if (need_load) {
            EffectPointer pEffect = m_pEffectsManager->instantiateEffect(effectId);
            m_pEQEffectRack->loadEffectToGroup(group, pEffect);
            m_pConfig->set(ConfigKey(kConfigKey, "EffectForGroup_" + group),
                    ConfigValue(effectId));
            m_filterWaveformEnableCOs[deck]->set(m_pEffectsManager->isEQ(effectId));

            // This is required to remove a previous selected effect that does not
            // fit to the current ShowAllEffects checkbox
            slotPopulateDeckEffectSelectors();
        }
        ++deck;
    }

    deck = 0;
    for (QComboBox* box : qAsConst(m_deckQuickEffectSelectors)) {
        QString effectId = box->itemData(box->currentIndex()).toString();
        QString group = PlayerManager::groupForDeck(deck);

        if (deck == 0) {
            firstEffectId = effectId;
            firstEffectIndex = box->currentIndex();
        } else if (CheckBoxSingleEqEffect->isChecked()) {
            effectId = firstEffectId;
            box->setCurrentIndex(firstEffectIndex);
        }

        // Only apply the effect if it changed -- so first interrogate the
        // loaded effect if any.
        bool need_load = true;
        if (m_pQuickEffectRack->numEffectChainSlots() > deck) {
            // It's not correct to get a chainslot by index number -- get by
            // group name instead.
            EffectChainSlotPointer chainslot =
                    m_pQuickEffectRack->getGroupEffectChainSlot(group);
            if (chainslot && chainslot->numSlots()) {
                EffectPointer effectpointer =
                        chainslot->getEffectSlot(0)->getEffect();
                if (effectpointer &&
                        effectpointer->getManifest()->id() == effectId) {
                    need_load = false;
                }
            }
        }
        if (need_load) {
            EffectPointer pEffect = m_pEffectsManager->instantiateEffect(effectId);
            m_pQuickEffectRack->loadEffectToGroup(group, pEffect);

            m_pConfig->set(ConfigKey(kConfigKey, "QuickEffectForGroup_" + group),
                    ConfigValue(effectId));

            // This is required to remove a previous selected effect that does not
            // fit to the current ShowAllEffects checkbox
            slotPopulateDeckEffectSelectors();
        }
        ++deck;
    }
}

void DlgPrefEQ::slotUpdateHiEQ() {
    if (SliderHiEQ->value() < SliderLoEQ->value())
    {
        SliderHiEQ->setValue(SliderLoEQ->value());
    }
    m_highEqFreq = getEqFreq(SliderHiEQ->value(),
                             SliderHiEQ->minimum(),
                             SliderHiEQ->maximum());
    validate_levels();
    if (m_highEqFreq < 1000) {
        TextHiEQ->setText( QString("%1 Hz").arg((int)m_highEqFreq));
    } else {
        TextHiEQ->setText( QString("%1 kHz").arg((int)m_highEqFreq / 1000.));
    }
    m_pConfig->set(ConfigKey(kConfigKey, "HiEQFrequency"),
                   ConfigValue(QString::number(static_cast<int>(m_highEqFreq))));
    m_pConfig->set(ConfigKey(kConfigKey, "HiEQFrequencyPrecise"),
                   ConfigValue(QString::number(m_highEqFreq, 'f')));

    slotApply();
}

void DlgPrefEQ::slotUpdateLoEQ() {
    if (SliderLoEQ->value() > SliderHiEQ->value())
    {
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
    m_pConfig->set(ConfigKey(kConfigKey, "LoEQFrequency"),
                   ConfigValue(QString::number(static_cast<int>(m_lowEqFreq))));
    m_pConfig->set(ConfigKey(kConfigKey, "LoEQFrequencyPrecise"),
                   ConfigValue(QString::number(m_lowEqFreq, 'f')));

    slotApply();
}

void DlgPrefEQ::slotUpdateMasterEQParameter(int value) {
    EffectPointer effect(m_pEffectMasterEQ);
    if (!effect.isNull()) {
        QSlider* slider = qobject_cast<QSlider*>(sender());
        int index = slider->property("index").toInt();
        EffectParameter* param = effect->getKnobParameterForSlot(index);
        if (param) {
            double dValue = value / 100.0;
            param->setValue(dValue);
            QLabel* valueLabel = m_masterEQValues[index];
            QString valueText = QString::number(dValue);
            valueLabel->setText(valueText);

            m_pConfig->set(ConfigKey(kConfigKey,
                    QString("EffectForGroup_[Master]_parameter%1").arg(index + 1)),
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
    double dsliderPos = (eqFreq - kFrequencyLowerLimit) / (kFrequencyUpperLimit-kFrequencyLowerLimit);
    dsliderPos = pow(dsliderPos, 1.0 / 4.0) * (maxValue - minValue) + minValue;
    return static_cast<int>(dsliderPos);
}

void DlgPrefEQ::slotApply() {
    m_COLoFreq.set(m_lowEqFreq);
    m_COHiFreq.set(m_highEqFreq);
    m_pConfig->set(ConfigKey(kConfigKey,"EqAutoReset"),
            ConfigValue(m_bEqAutoReset ? 1 : 0));
    m_pConfig->set(ConfigKey(kConfigKey,"GainAutoReset"),
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
}

void DlgPrefEQ::slotUpdateEqAutoReset(int i) {
    m_bEqAutoReset = static_cast<bool>(i);
}

void DlgPrefEQ::slotUpdateGainAutoReset(int i) {
    m_bGainAutoReset = static_cast<bool>(i);
}

void DlgPrefEQ::slotBypass(int state) {
    if (state) {
        m_pConfig->set(ConfigKey(kConfigKey, kEnableEqs), QString("no"));
        // Disable effect processing for all decks by setting the appropriate
        // controls to 0 ("[EqualizerRackX_EffectUnitDeck_Effect1],enable")
        int deck = 0;
        for (const auto& box : qAsConst(m_deckEqEffectSelectors)) {
            QString group = getEQEffectGroupForDeck(deck);
            ControlObject::set(ConfigKey(group, "enabled"), 0);
            m_filterWaveformEnableCOs[deck]->set(0);
            deck++;
            box->setEnabled(false);
        }
    } else {
        m_pConfig->set(ConfigKey(kConfigKey, kEnableEqs), QString("yes"));
        // Enable effect processing for all decks by setting the appropriate
        // controls to 1 ("[EqualizerRackX_EffectUnitDeck_Effect1],enable")
        int deck = 0;
        for (const auto& box : qAsConst(m_deckEqEffectSelectors)) {
            QString group = getEQEffectGroupForDeck(deck);
            ControlObject::set(ConfigKey(group, "enabled"), 1);
            m_filterWaveformEnableCOs[deck]->set(m_filterWaveformEffectLoaded[deck]);
            deck++;
            box->setEnabled(true);
        }
    }

    slotApply();
}

void DlgPrefEQ::setUpMasterEQ() {
    connect(pbResetMasterEq, &QAbstractButton::clicked, this, &DlgPrefEQ::slotMasterEQToDefault);

    connect(comboBoxMasterEq,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefEQ::slotMasterEqEffectChanged);

    QString configuredEffect = m_pConfig->getValue(ConfigKey(kConfigKey,
            "EffectForGroup_[Master]"), kDefaultMasterEqId);

    const QList<EffectManifestPointer> availableMasterEQEffects =
        m_pEffectsManager->getAvailableEffectManifestsFiltered(isMasterEQ);

    for (const auto& pManifest : availableMasterEQEffects) {
        comboBoxMasterEq->addItem(pManifest->name(), QVariant(pManifest->id()));
    }
    //: Displayed when no effect is selected
    comboBoxMasterEq->addItem(tr("None"), QVariant());

    int masterEqIndex = comboBoxMasterEq->findData(configuredEffect);
    if (masterEqIndex < 0) {
        masterEqIndex = availableMasterEQEffects.size(); // selects "None"
    }
    comboBoxMasterEq->setCurrentIndex(masterEqIndex);

    // Load parameters from preferences:
    EffectPointer effect(m_pEffectMasterEQ);
    if (!effect.isNull()) {
        int knobNum = effect->numKnobParameters();
        for (int i = 0; i < knobNum; i++) {
            EffectParameter* param = effect->getKnobParameterForSlot(i);
            if (param) {
                QString strValue = m_pConfig->getValueString(ConfigKey(kConfigKey,
                        QString("EffectForGroup_[Master]_parameter%1").arg(i + 1)));
                bool ok;
                double value = strValue.toDouble(&ok);
                if (ok) {
                    setMasterEQParameter(i, value);
                }
            }
        }
    }
}

void DlgPrefEQ::slotMasterEqEffectChanged(int effectIndex) {
    // clear parameters view first
    qDeleteAll(m_masterEQSliders);
    m_masterEQSliders.clear();
    qDeleteAll(m_masterEQValues);
    m_masterEQValues.clear();
    qDeleteAll(m_masterEQLabels);
    m_masterEQLabels.clear();

    QString effectId = comboBoxMasterEq->itemData(effectIndex).toString();

    if (effectId.isNull()) {
        pbResetMasterEq->hide();
    } else {
        pbResetMasterEq->show();
    }

    EffectChainSlotPointer pChainSlot = m_pOutputEffectRack->getEffectChainSlot(0);

    if (pChainSlot) {
        EffectChainPointer pChain = pChainSlot->getEffectChain();
        VERIFY_OR_DEBUG_ASSERT(pChain) {
            pChain = pChainSlot->getOrCreateEffectChain(m_pEffectsManager.get());
        }
        EffectPointer pEffect = m_pEffectsManager->instantiateEffect(effectId);
        pChain->replaceEffect(0, pEffect);

        if (pEffect) {
            pEffect->setEnabled(true);
            m_pEffectMasterEQ = pEffect;

            int knobNum = pEffect->numKnobParameters();

            // Create and set up Master EQ's sliders
            int i;
            for (i = 0; i < knobNum; i++) {
                EffectParameter* param = pEffect->getKnobParameterForSlot(i);
                if (param) {
                    // Setup Label
                    QLabel* centerFreqLabel = new QLabel(this);
                    QString labelText = param->manifest()->name();
                    m_masterEQLabels.append(centerFreqLabel);
                    centerFreqLabel->setText(labelText);
                    slidersGridLayout->addWidget(centerFreqLabel, 0, i + 1, Qt::AlignCenter);

                    QSlider* slider = new QSlider(this);
                    slider->setMinimum(static_cast<int>(param->getMinimum() * 100));
                    slider->setMaximum(static_cast<int>(param->getMaximum() * 100));
                    slider->setSingleStep(1);
                    slider->setValue(static_cast<int>(param->getDefault() * 100));
                    slider->setMinimumHeight(90);
                    // Set the index as a property because we need it inside slotUpdateFilter()
                    slider->setProperty("index", QVariant(i));
                    slidersGridLayout->addWidget(slider, 1, i + 1, Qt::AlignCenter);
                    m_masterEQSliders.append(slider);
                    connect(slider,
                            &QAbstractSlider::sliderMoved,
                            this,
                            &DlgPrefEQ::slotUpdateMasterEQParameter);

                    QLabel* valueLabel = new QLabel(this);
                    m_masterEQValues.append(valueLabel);
                    QString valueText = QString::number((double)slider->value() / 100);
                    valueLabel->setText(valueText);
                    slidersGridLayout->addWidget(valueLabel, 2, i + 1, Qt::AlignCenter);

                }
            }
        }
    }

    // Update the configured effect for the current QComboBox
    m_pConfig->set(ConfigKey(kConfigKey, "EffectForGroup_[Master]"),
            ConfigValue(effectId));
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
    m_highEqFreq = math_clamp<double>(m_highEqFreq, kFrequencyLowerLimit,
                                      kFrequencyUpperLimit);
    m_lowEqFreq = math_clamp<double>(m_lowEqFreq, kFrequencyLowerLimit,
                                     kFrequencyUpperLimit);
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
    // The EQ effect is loaded in effect slot 0.
    if (m_pEQEffectRack) {
        return m_pEQEffectRack->formatEffectSlotGroupString(
            0, PlayerManager::groupForDeck(deck));
    }
    return QString();
}

QString DlgPrefEQ::getQuickEffectGroupForDeck(int deck) const {
    // The quick effect is loaded in effect slot 0.
    if (m_pQuickEffectRack) {
        return m_pQuickEffectRack->formatEffectSlotGroupString(
            0, PlayerManager::groupForDeck(deck));
    }
    return QString();
}

void DlgPrefEQ::slotMasterEQToDefault() {
    EffectPointer effect(m_pEffectMasterEQ);
    if (!effect.isNull()) {
        int knobNum = effect->numKnobParameters();
        for (int i = 0; i < knobNum; i++) {
            EffectParameter* param = effect->getKnobParameterForSlot(i);
            if (param) {
                double defaultValue = param->getDefault();
                setMasterEQParameter(i, defaultValue);
            }
        }
    }
}

void DlgPrefEQ::setMasterEQParameter(int i, double value) {
    EffectPointer effect(m_pEffectMasterEQ);
    if (!effect.isNull()) {
        EffectParameter* param = effect->getKnobParameterForSlot(i);
        if (param) {
            param->setValue(value);
            m_masterEQSliders[i]->setValue(static_cast<int>(value * 100));

            QLabel* valueLabel = m_masterEQValues[i];
            QString valueText = QString::number(value);
            valueLabel->setText(valueText);

            m_pConfig->set(ConfigKey(kConfigKey,
                    QString("EffectForGroup_[Master]_parameter%1").arg(i + 1)),
                            ConfigValue(valueText));
        }
    }
}
