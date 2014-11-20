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

#include <QWidget>
#include <QString>
#include <QHBoxLayout>

#include "dlgprefeq.h"
#include "engine/enginefilterbessel4.h"
#include "controlobject.h"
#include "controlobjectslave.h"
#include "util/math.h"
#include "playermanager.h"

const char* kConfigKey = "[Mixer Profile]";
const char* kEnableEqs = "EnableEQs";
const char* kDefaultEqId = "org.mixxx.effects.bessel8lvmixeq";

const int kFrequencyUpperLimit = 20050;
const int kFrequencyLowerLimit = 16;

DlgPrefEQ::DlgPrefEQ(QWidget* pParent, EffectsManager* pEffectsManager,
                     ConfigObject<ConfigValue>* pConfig)
        : DlgPreferencePage(pParent),
          m_COLoFreq(kConfigKey, "LoEQFrequency"),
          m_COHiFreq(kConfigKey, "HiEQFrequency"),
          m_pConfig(pConfig),
          m_lowEqFreq(0.0),
          m_highEqFreq(0.0),
          m_pEffectsManager(pEffectsManager),
          m_inSlotPopulateDeckEffectSelectors(false),
          m_bEqAutoReset(false) {

    // Get the EQ Effect Rack
    m_pEQEffectRack = m_pEffectsManager->getEQEffectRack().data();
    m_pQuickEffectRack = m_pEffectsManager->getQuickEffectRack().data();

    setupUi(this);
    // Connection
    connect(SliderHiEQ, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateHiEQ()));
    connect(SliderHiEQ, SIGNAL(sliderMoved(int)), this, SLOT(slotUpdateHiEQ()));
    connect(SliderHiEQ, SIGNAL(sliderReleased()), this, SLOT(slotUpdateHiEQ()));

    connect(SliderLoEQ, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateLoEQ()));
    connect(SliderLoEQ, SIGNAL(sliderMoved(int)), this, SLOT(slotUpdateLoEQ()));
    connect(SliderLoEQ, SIGNAL(sliderReleased()), this, SLOT(slotUpdateLoEQ()));

    connect(CheckBoxEqAutoReset, SIGNAL(stateChanged(int)), this, SLOT(slotUpdateEqAutoReset(int)));
    connect(CheckBoxBypass, SIGNAL(stateChanged(int)), this, SLOT(slotBypass(int)));

    connect(CheckBoxHideEffects, SIGNAL(stateChanged(int)),
            this, SLOT(slotPopulateDeckEffectSelectors()));

    // Set to basic view if a previous configuration is missing
    CheckBoxHideEffects->setChecked(m_pConfig->getValueString(
            ConfigKey(kConfigKey, "AdvancedView"), QString("no")) == QString("no"));

    // Add drop down lists for current decks and connect num_decks control
    // to slotAddComboBox
    m_pNumDecks = new ControlObjectSlave("[Master]", "num_decks", this);
    m_pNumDecks->connectValueChanged(SLOT(slotAddComboBox(double)));
    slotAddComboBox(m_pNumDecks->get());

    loadSettings();
    slotUpdate();
    slotApply();
}

DlgPrefEQ::~DlgPrefEQ() {
    qDeleteAll(m_deckEqEffectSelectors);
    m_deckEqEffectSelectors.clear();

    qDeleteAll(m_deckFilterEffectSelectors);
    m_deckFilterEffectSelectors.clear();

    qDeleteAll(m_filterWaveformEnableCOs);
    m_filterWaveformEnableCOs.clear();
}

void DlgPrefEQ::slotAddComboBox(double numDecks) {
    int oldDecks = m_deckEqEffectSelectors.size();     
    while (m_deckEqEffectSelectors.size() < static_cast<int>(numDecks)) {
        int deckNo = m_deckEqEffectSelectors.size() + 1;
        QLabel* label = new QLabel(QObject::tr("Deck %1").
                            arg(deckNo), this);

        QString group = PlayerManager::groupForDeck(
                m_deckEqEffectSelectors.size());

        m_filterWaveformEnableCOs.append(
                new ControlObject(ConfigKey(group, "filterWaveformEnable")));
        m_filterWaveformEffectLoaded.append(false);

        // Create the drop down list for EQs
        QComboBox* eqComboBox = new QComboBox(this);
        m_deckEqEffectSelectors.append(eqComboBox);
        connect(eqComboBox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(slotEqEffectChangedOnDeck(int)));

        // Create the drop down list for EQs
        QComboBox* filterComboBox = new QComboBox(this);
        m_deckFilterEffectSelectors.append(filterComboBox);
        connect(filterComboBox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(slotQuickEffectChangedOnDeck(int)));

        // Setup the GUI
        gridLayout_3->addWidget(label, deckNo, 0);
        gridLayout_3->addWidget(eqComboBox, deckNo, 1);
        gridLayout_3->addWidget(filterComboBox, deckNo, 2);
        gridLayout_3->addItem(new QSpacerItem(
                    40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum),
                deckNo, 3, 1, 1);
    }
    slotPopulateDeckEffectSelectors(); 
    for (int i = oldDecks; i < static_cast<int>(numDecks); ++i) {
        // Set the configured effect for box and simpleBox or Bessel8 LV-Mix EQ
        // if none is configured
        QString configuredEffect;
        int selectedEffectIndex;
        QString group = PlayerManager::groupForDeck(i);
        configuredEffect = m_pConfig->getValueString(ConfigKey(kConfigKey,
                "EffectForGroup_" + group), kDefaultEqId);
        selectedEffectIndex = m_deckEqEffectSelectors[i]->findData(configuredEffect);
        if (selectedEffectIndex < 0) {
            selectedEffectIndex = m_deckEqEffectSelectors[i]->findData(kDefaultEqId);
            configuredEffect = kDefaultEqId;
        }
        m_deckEqEffectSelectors[i]->setCurrentIndex(selectedEffectIndex);
        m_filterWaveformEffectLoaded[i] = m_pEffectsManager->isEQ(configuredEffect);
        m_filterWaveformEnableCOs[i]->set(
                m_filterWaveformEffectLoaded[i] &&
                !CheckBoxBypass->checkState());
    }	
}

static bool isMixingEQ(EffectManifest* pManifest) {
    return pManifest->isMixingEQ();
}

static bool isForFilterKnob(EffectManifest* pManifest) {
    return pManifest->isForFilterKnob();
}

void DlgPrefEQ::slotPopulateDeckEffectSelectors() {
    m_inSlotPopulateDeckEffectSelectors = true; // prevents a recursive call

    QList<QPair<QString, QString> > availableEQEffectNames; 
    QList<QPair<QString, QString> > availableFilterEffectNames;
    EffectsManager::EffectManifestFilterFnc filterEQ;
    EffectsManager::EffectManifestFilterFnc filterFilter;
    if (CheckBoxHideEffects->isChecked()) {
        m_pConfig->set(ConfigKey(kConfigKey, "AdvancedView"), QString("no"));
        filterEQ = isMixingEQ;
        filterFilter = isForFilterKnob;
    } else {
        m_pConfig->set(ConfigKey(kConfigKey, "AdvancedView"), QString("yes"));
        filterEQ = NULL; // take all;
        filterFilter = NULL;
    }
    availableEQEffectNames =
            m_pEffectsManager->getEffectNamesFiltered(filterEQ);
    availableFilterEffectNames =
            m_pEffectsManager->getEffectNamesFiltered(filterFilter);

    foreach (QComboBox* box, m_deckEqEffectSelectors) {
        // Populate comboboxes with all available effects
        // Save current selection
        QString selectedEffectId = box->itemData(box->currentIndex()).toString();
        QString selectedEffectName = box->itemText(box->currentIndex());
        box->clear();
        int currentIndex = -1;// Nothing selected

        int i;
        for (i = 0; i < availableEQEffectNames.size(); ++i) {
            box->addItem(availableEQEffectNames[i].second);
            box->setItemData(i, QVariant(availableEQEffectNames[i].first));
            if (selectedEffectId == availableEQEffectNames[i].first) {
                currentIndex = i; 
            }
        }
        if (currentIndex < 0 && !selectedEffectName.isEmpty()) {
            // current selection is not part of the new list
            // So we need to add it
            box->addItem(selectedEffectName);
            box->setItemData(i, QVariant(selectedEffectId));
            currentIndex = i;
        }
        box->setCurrentIndex(currentIndex); 
    }

    availableFilterEffectNames.append(QPair<QString,QString>("", tr("None")));

    foreach (QComboBox* box, m_deckFilterEffectSelectors) {
        // Populate comboboxes with all available effects
        // Save current selection
        QString selectedEffectId = box->itemData(box->currentIndex()).toString();
        QString selectedEffectName = box->itemText(box->currentIndex());
        box->clear();
        int currentIndex = -1;// Nothing selected

        int i;
        for (i = 0; i < availableFilterEffectNames.size(); ++i) {
            box->addItem(availableFilterEffectNames[i].second);
            box->setItemData(i, QVariant(availableFilterEffectNames[i].first));
            if (selectedEffectId == availableFilterEffectNames[i].first) {
                currentIndex = i;
            }
        }
        if (currentIndex < 0 && !selectedEffectName.isEmpty()) {
            // current selection is not part of the new list
            // So we need to add it
            box->addItem(selectedEffectName);
            box->setItemData(i, QVariant(selectedEffectId));
            currentIndex = i;
        }
        box->setCurrentIndex(currentIndex);
    }


    m_inSlotPopulateDeckEffectSelectors = false;
}

void DlgPrefEQ::loadSettings() {
    QString highEqCourse = m_pConfig->getValueString(ConfigKey(kConfigKey, "HiEQFrequency"));
    QString highEqPrecise = m_pConfig->getValueString(ConfigKey(kConfigKey, "HiEQFrequencyPrecise"));
    QString lowEqCourse = m_pConfig->getValueString(ConfigKey(kConfigKey, "LoEQFrequency"));
    QString lowEqPrecise = m_pConfig->getValueString(ConfigKey(kConfigKey, "LoEQFrequencyPrecise"));
    m_bEqAutoReset = static_cast<bool>(m_pConfig->getValueString(
            ConfigKey(kConfigKey, "EqAutoReset")).toInt());
    CheckBoxEqAutoReset->setChecked(m_bEqAutoReset);
    CheckBoxBypass->setChecked(m_pConfig->getValueString(
            ConfigKey(kConfigKey, kEnableEqs), QString("yes")) == QString("no"));

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

    if (m_pConfig->getValueString(
            ConfigKey(kConfigKey, kEnableEqs), "yes") == QString("yes")) {
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
    setDefaultShelves();
    foreach(QComboBox* pCombo, m_deckEqEffectSelectors) {
        pCombo->setCurrentIndex(
               pCombo->findData(kDefaultEqId));
    }
    loadSettings();
    CheckBoxBypass->setChecked(Qt::Unchecked);
    CheckBoxHideEffects->setChecked(Qt::Checked);
    m_bEqAutoReset = false;
    CheckBoxEqAutoReset->setChecked(Qt::Unchecked);
    slotUpdate();
    slotApply();
}

void DlgPrefEQ::slotEqEffectChangedOnDeck(int effectIndex) {
    QComboBox* c = qobject_cast<QComboBox*>(sender());
    // Check if qobject_cast was successful
    if (c && !m_inSlotPopulateDeckEffectSelectors) {
        int deckNumber = m_deckEqEffectSelectors.indexOf(c);
        QString effectId = c->itemData(effectIndex).toString();
        m_pEQEffectRack->loadEffectToChainSlot(deckNumber, 0, effectId);

        QString group = PlayerManager::groupForDeck(deckNumber);

        // Update the configured effect for the current QComboBox
        m_pConfig->set(ConfigKey(kConfigKey, "EffectForGroup_" + group),
                ConfigValue(effectId));


        m_filterWaveformEffectLoaded[deckNumber] = m_pEffectsManager->isEQ(effectId);
        m_filterWaveformEnableCOs[deckNumber]->set(
                m_filterWaveformEffectLoaded[deckNumber] &&
                !CheckBoxBypass->checkState());

        // This is required to remove a previous selected effect that does not
        // fit to the current ShowAllEffects checkbox
        slotPopulateDeckEffectSelectors();
    }
}

void DlgPrefEQ::slotQuickEffectChangedOnDeck(int effectIndex) {
    QComboBox* c = qobject_cast<QComboBox*>(sender());
    // Check if qobject_cast was successful
    if (c && !m_inSlotPopulateDeckEffectSelectors) {
        int deckNumber = m_deckFilterEffectSelectors.indexOf(c);
        QString effectId = c->itemData(effectIndex).toString();
        m_pQuickEffectRack->loadEffectToChainSlot(deckNumber, 0, effectId);

        // Update the configured effect for the current QComboBox
        //m_pConfig->set(ConfigKey(CONFIG_KEY, QString("EffectForDeck%1").
        //               arg(deckNumber + 1)), ConfigValue(effectId));

        // This is required to remove a previous selected effect that does not
        // fit to the current ShowAllEffects checkbox
        slotPopulateDeckEffectSelectors();
    }
}

void DlgPrefEQ::slotUpdateHiEQ()
{
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

void DlgPrefEQ::slotUpdateLoEQ()
{
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

int DlgPrefEQ::getSliderPosition(double eqFreq, int minValue, int maxValue)
{
    if (eqFreq >= kFrequencyUpperLimit) {
        return maxValue;
    } else if (eqFreq <= kFrequencyLowerLimit) {
        return minValue;
    }
    double dsliderPos = (eqFreq - kFrequencyLowerLimit) / (kFrequencyUpperLimit-kFrequencyLowerLimit);
    dsliderPos = pow(dsliderPos, 1.0 / 4.0) * (maxValue - minValue) + minValue;
    return dsliderPos;
}

void DlgPrefEQ::slotApply() {
    m_COLoFreq.set(m_lowEqFreq);
    m_COHiFreq.set(m_highEqFreq);
    m_pConfig->set(ConfigKey(kConfigKey,"EqAutoReset"),
            ConfigValue(m_bEqAutoReset ? 1 : 0));
}

// supposed to set the widgets to match internal state
void DlgPrefEQ::slotUpdate() {
    slotUpdateLoEQ();
    slotUpdateHiEQ();
    slotPopulateDeckEffectSelectors();
    CheckBoxEqAutoReset->setChecked(m_bEqAutoReset);
}

void DlgPrefEQ::slotUpdateEqAutoReset(int i) {
    m_bEqAutoReset = static_cast<bool>(i);
}

void DlgPrefEQ::slotBypass(int state) {
    if (state) {
        m_pConfig->set(ConfigKey(kConfigKey, kEnableEqs), QString("no"));
        // Disable effect processing for all decks by setting the appropriate
        // controls to 0 ("[EffectRackX_EffectUnitDeck_Effect1],enable")
        int deck = 0;
        foreach(QComboBox* box, m_deckEqEffectSelectors) {
            QString group = getEQEffectGroupForDeck(deck);
            ControlObject::set(ConfigKey(group, "enabled"), 0);
            m_filterWaveformEnableCOs[deck]->set(0);
            deck++;
            box->setEnabled(false);
        }
    } else {
        m_pConfig->set(ConfigKey(kConfigKey, kEnableEqs), QString("yes"));
        // Enable effect processing for all decks by setting the appropriate
        // controls to 1 ("[EffectRackX_EffectUnitDeck_Effect1],enable")
        int deck = 0;
        ControlObjectSlave enableControl;
        foreach(QComboBox* box, m_deckEqEffectSelectors) {
            QString group = getEQEffectGroupForDeck(deck);
            ControlObject::set(ConfigKey(group, "enabled"), 1);
            m_filterWaveformEnableCOs[deck]->set(m_filterWaveformEffectLoaded[deck]);
            deck++;
            box->setEnabled(true);
        }
    }

    slotApply();
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
    return EffectSlot::formatGroupString(
        EffectChainSlot::formatGroupString(
                m_pEQEffectRack->getGroup(),
                PlayerManager::groupForDeck(deck)),
        0);
}

QString DlgPrefEQ::getQuickEffectGroupForDeck(int deck) const {
    return EffectSlot::formatGroupString(
        EffectChainSlot::formatGroupString(
                m_pQuickEffectRack->getGroup(),
                PlayerManager::groupForDeck(deck)),
        0);
}
