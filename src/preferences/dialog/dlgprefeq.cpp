/***************************************************************************
                          dlgprefeq.cpp  -  description
                             -------------------
    begin                : Thu Jun 7 2007
    copyright            : (C) 2007 by John Sully
    email                : jsully@scs.ryerson.ca
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "preferences/dialog/dlgprefeq.h"

#include <QHBoxLayout>
#include <QString>
#include <QWidget>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "defs_urls.h"
#include "effects/backends/builtin/biquadfullkilleqeffect.h"
#include "effects/effectslot.h"
#include "effects/specialeffectchains.h"
#include "mixer/playermanager.h"

namespace {
const QString kConfigKey = "[Mixer Profile]";
const QString kConfigKeyPrefix = "EffectForGroup_";
const QString kEnableEqs = "EnableEQs";
const QString kEqsOnly = "EQsOnly";
const QString kSingleEq = "SingleEQEffect";
const QString kDefaultEqId = BiquadFullKillEQEffect::getId() + " Built-in";
const QString kDefaultMasterEqId = QString();

const int kFrequencyUpperLimit = 20050;
const int kFrequencyLowerLimit = 16;

static bool isMixingEQ(EffectManifest* pManifest) {
    return pManifest->isMixingEQ();
}

static bool isMasterEQ(EffectManifest* pManifest) {
    return pManifest->isMasterEQ();
}
} // anonymous namespace

DlgPrefEQ::DlgPrefEQ(QWidget* pParent, EffectsManager* pEffectsManager, UserSettingsPointer pConfig)
        : DlgPreferencePage(pParent),
          m_COLoFreq(kConfigKey, "LoEQFrequency"),
          m_COHiFreq(kConfigKey, "HiEQFrequency"),
          m_pConfig(pConfig),
          m_lowEqFreq(0.0),
          m_highEqFreq(0.0),
          m_pEffectsManager(pEffectsManager),
          m_pBackendManager(pEffectsManager->getBackendManager()),
          m_firstSelectorLabel(NULL),
          m_pNumDecks(NULL),
          m_inSlotPopulateDeckEffectSelectors(false),
          m_bEqAutoReset(false),
          m_bGainAutoReset(false) {
    setupUi(this);
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
}

void DlgPrefEQ::slotNumDecksChanged(double numDecks) {
    int oldDecks = m_deckEqEffectSelectors.size();
    while (m_deckEqEffectSelectors.size() < static_cast<int>(numDecks)) {
        int deckNo = m_deckEqEffectSelectors.size() + 1;

        QLabel* label = new QLabel(QObject::tr("Deck %1 EQ Effect").arg(deckNo), this);

        // Create the drop down list for EQs
        QComboBox* eqComboBox = new QComboBox(this);
        m_deckEqEffectSelectors.append(eqComboBox);
        connect(eqComboBox,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                this,
                &DlgPrefEQ::slotEffectChangedOnDeck);

        if (deckNo == 1) {
            m_firstSelectorLabel = label;
            if (CheckBoxEqOnly->isChecked()) {
                m_firstSelectorLabel->setText(QObject::tr("EQ Effect"));
            }
        }

        // Setup the GUI
        gridLayout_3->addWidget(label, deckNo, 0);
        gridLayout_3->addWidget(eqComboBox, deckNo, 1);
        gridLayout_3->addItem(new QSpacerItem(
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
        QString configuredEffect = m_pConfig->getValue(ConfigKey(kConfigKey,
                                                               kConfigKeyPrefix + group),
                kDefaultEqId);

        const EffectManifestPointer pEQManifest =
                m_pBackendManager->getManifestFromUniqueId(configuredEffect);

        int selectedEQEffectIndex = 0;
        if (pEQManifest) {
            selectedEQEffectIndex = m_deckEqEffectSelectors[i]->findData(
                    QVariant(pEQManifest->uniqueId()));
        } else {
            // Select "None"
            selectedEQEffectIndex = m_deckEqEffectSelectors[i]->count() - 1;
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
        m_pConfig->set(ConfigKey(kConfigKey, kEqsOnly), QString("yes"));
        filterEQ = isMixingEQ;
    } else {
        m_pConfig->set(ConfigKey(kConfigKey, kEqsOnly), QString("no"));
        filterEQ = nullptr; // take all
    }

    populateDeckBoxList(m_deckEqEffectSelectors, filterEQ);

    m_inSlotPopulateDeckEffectSelectors = false;
}

void DlgPrefEQ::populateDeckBoxList(
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

        int i = 0;
        for (const auto& pManifest : pManifestList) {
            box->addItem(pManifest->name(), QVariant(pManifest->uniqueId()));
            if (pCurrentlySelectedManifest &&
                    pCurrentlySelectedManifest.data() == pManifest.data()) {
                currentIndex = i;
            }
            ++i;
        }
        //: Displayed when no effect is selected
        box->addItem(tr("None"), QVariant());
        if (pCurrentlySelectedManifest == nullptr) {
            currentIndex = box->count() - 1; // selects "None"
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

void DlgPrefEQ::slotSingleEqChecked(int checked) {
    bool do_hide = static_cast<bool>(checked);
    m_pConfig->set(ConfigKey(kConfigKey, kSingleEq),
            do_hide ? QString("yes") : QString("no"));
    int deck1EQIndex = m_deckEqEffectSelectors.at(0)->currentIndex();
    for (int i = 2; i < m_deckEqEffectSelectors.size() + 1; ++i) {
        if (do_hide) {
            m_deckEqEffectSelectors.at(i - 1)->setCurrentIndex(deck1EQIndex);
            gridLayout_3->itemAtPosition(i, 0)->widget()->hide();
            gridLayout_3->itemAtPosition(i, 1)->widget()->hide();
        } else {
            gridLayout_3->itemAtPosition(i, 0)->widget()->show();
            gridLayout_3->itemAtPosition(i, 1)->widget()->show();
        }
    }

    if (m_firstSelectorLabel != NULL) {
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
                                                        ConfigKey(kConfigKey, "EqAutoReset"))
                                               .toInt());
    CheckBoxEqAutoReset->setChecked(m_bEqAutoReset);
    m_bGainAutoReset = static_cast<bool>(m_pConfig->getValueString(
                                                          ConfigKey(kConfigKey, "GainAutoReset"))
                                                 .toInt());
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

void DlgPrefEQ::setDefaultShelves() {
    m_pConfig->set(ConfigKey(kConfigKey, "HiEQFrequency"), ConfigValue(2500));
    m_pConfig->set(ConfigKey(kConfigKey, "LoEQFrequency"), ConfigValue(250));
    m_pConfig->set(ConfigKey(kConfigKey, "HiEQFrequencyPrecise"), ConfigValue(2500.0));
    m_pConfig->set(ConfigKey(kConfigKey, "LoEQFrequencyPrecise"), ConfigValue(250.0));
}

void DlgPrefEQ::slotResetToDefaults() {
    slotMasterEQToDefault();
    setDefaultShelves();
    for (QComboBox* pCombo : std::as_const(m_deckEqEffectSelectors)) {
        pCombo->setCurrentIndex(
                pCombo->findData(kDefaultEqId));
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
            }

            if (!startingUp) {
                m_eqIndiciesOnUpdate[deck] = box->currentIndex();
            }

            QString configString;
            if (pManifest) {
                configString = pManifest->uniqueId();
            }
            m_pConfig->set(ConfigKey(kConfigKey, kConfigKeyPrefix + group),
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
    m_pConfig->set(ConfigKey(kConfigKey, "HiEQFrequency"),
            ConfigValue(QString::number(static_cast<int>(m_highEqFreq))));
    m_pConfig->set(ConfigKey(kConfigKey, "HiEQFrequencyPrecise"),
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
    m_pConfig->set(ConfigKey(kConfigKey, "LoEQFrequency"),
            ConfigValue(QString::number(static_cast<int>(m_lowEqFreq))));
    m_pConfig->set(ConfigKey(kConfigKey, "LoEQFrequencyPrecise"),
            ConfigValue(QString::number(m_lowEqFreq, 'f')));

    slotApply();
}

void DlgPrefEQ::slotUpdateMasterEQParameter(int value) {
    EffectSlotPointer pEffectSlot(m_pEffectMasterEQ);
    if (!pEffectSlot.isNull()) {
        QSlider* slider = qobject_cast<QSlider*>(sender());
        int index = slider->property("index").toInt();
        auto pParameterSlot = pEffectSlot->getEffectParameterSlot(
                EffectManifestParameter::ParameterType::KNOB, index);

        if (pParameterSlot->isLoaded()) {
            double dValue = value / 100.0;
            pParameterSlot->slotParameterValueChanged(dValue);
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
    double dsliderPos = (eqFreq - kFrequencyLowerLimit) /
            (kFrequencyUpperLimit - kFrequencyLowerLimit);
    dsliderPos = pow(dsliderPos, 1.0 / 4.0) * (maxValue - minValue) + minValue;
    return static_cast<int>(dsliderPos);
}

void DlgPrefEQ::slotApply() {
    m_COLoFreq.set(m_lowEqFreq);
    m_COHiFreq.set(m_highEqFreq);
    m_pConfig->set(ConfigKey(kConfigKey, "EqAutoReset"),
            ConfigValue(m_bEqAutoReset ? 1 : 0));
    m_pConfig->set(ConfigKey(kConfigKey, "GainAutoReset"),
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
        m_pConfig->set(ConfigKey(kConfigKey, kEnableEqs), QString("no"));
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
        m_pConfig->set(ConfigKey(kConfigKey, kEnableEqs), QString("yes"));
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

void DlgPrefEQ::setUpMasterEQ() {
    connect(pbResetMasterEq, &QPushButton::clicked, this, &DlgPrefEQ::slotMasterEQToDefault);

    connect(comboBoxMasterEq,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefEQ::slotMasterEqEffectChanged);

    const QString configuredEffectId = m_pConfig->getValue(ConfigKey(kConfigKey,
                                                                   "EffectForGroup_[Master]"),
            kDefaultMasterEqId);
    const EffectManifestPointer configuredEffectManifest =
            m_pBackendManager->getManifestFromUniqueId(configuredEffectId);

    const QList<EffectManifestPointer> availableMasterEQEffects =
            getFilteredManifests(isMasterEQ);

    for (const auto& pManifest : availableMasterEQEffects) {
        comboBoxMasterEq->addItem(pManifest->name(), QVariant(pManifest->uniqueId()));
    }
    //: Displayed when no effect is selected
    comboBoxMasterEq->addItem(tr("None"), QVariant());

    int masterEqIndex = availableMasterEQEffects.size(); // selects "None" by default
    if (configuredEffectManifest) {
        masterEqIndex = comboBoxMasterEq->findData(configuredEffectManifest->uniqueId());
    }
    comboBoxMasterEq->setCurrentIndex(masterEqIndex);

    // Load parameters from preferences:
    EffectSlotPointer pEffectSlot(m_pEffectMasterEQ);
    if (!pEffectSlot.isNull()) {
        int knobNum = pEffectSlot->numParameters(EffectManifestParameter::ParameterType::KNOB);
        for (int i = 0; i < knobNum; i++) {
            auto pParameterSlot = pEffectSlot->getEffectParameterSlot(
                    EffectManifestParameter::ParameterType::KNOB, i);

            if (pParameterSlot->isLoaded()) {
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

    const EffectManifestPointer pManifest =
            m_pBackendManager->getManifestFromUniqueId(
                    comboBoxMasterEq->itemData(effectIndex).toString());

    if (pManifest == nullptr) {
        pbResetMasterEq->hide();
    } else {
        pbResetMasterEq->show();
    }

    auto pChainSlot = m_pEffectsManager->getOutputEffectChain();
    if (pChainSlot) {
        auto pEffectSlot = pChainSlot->getEffectSlot(0);
        if (pEffectSlot) {
            pEffectSlot->loadEffectWithDefaults(pManifest);
            pEffectSlot->setEnabled(true);
            m_pEffectMasterEQ = pEffectSlot;

            int knobNum = pEffectSlot->numParameters(EffectManifestParameter::ParameterType::KNOB);

            // Create and set up Master EQ's sliders
            int i;
            for (i = 0; i < knobNum; i++) {
                auto pParameterSlot = pEffectSlot->getEffectParameterSlot(
                        EffectManifestParameter::ParameterType::KNOB, i);

                if (pParameterSlot->isLoaded()) {
                    EffectManifestParameterPointer pManifestParameter =
                            pParameterSlot->getManifest();

                    // Setup Label
                    QLabel* centerFreqLabel = new QLabel(this);
                    QString labelText = pParameterSlot->getManifest()->name();
                    m_masterEQLabels.append(centerFreqLabel);
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
                    slidersGridLayout->addWidget(slider, 1, i + 1, Qt::AlignCenter);
                    m_masterEQSliders.append(slider);
                    connect(slider,
                            &QSlider::sliderMoved,
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
    if (pManifest) {
        m_pConfig->set(ConfigKey(kConfigKey, "EffectForGroup_[Master]"),
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

void DlgPrefEQ::slotMasterEQToDefault() {
    EffectSlotPointer pEffectSlot(m_pEffectMasterEQ);
    if (!pEffectSlot.isNull()) {
        int knobNum = pEffectSlot->numParameters(EffectManifestParameter::ParameterType::KNOB);
        for (int i = 0; i < knobNum; i++) {
            auto pParameterSlot = pEffectSlot->getEffectParameterSlot(
                    EffectManifestParameter::ParameterType::KNOB, i);
            if (pParameterSlot->isLoaded()) {
                double defaultValue = pParameterSlot->getManifest()->getDefault();
                setMasterEQParameter(i, defaultValue);
            }
        }
    }
}

void DlgPrefEQ::setMasterEQParameter(int i, double value) {
    EffectSlotPointer pEffectSlot(m_pEffectMasterEQ);
    if (!pEffectSlot.isNull()) {
        auto pParameterSlot = pEffectSlot->getEffectParameterSlot(
                EffectManifestParameter::ParameterType::KNOB, i);

        if (pParameterSlot->isLoaded()) {
            pParameterSlot->slotParameterValueChanged(value);
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
