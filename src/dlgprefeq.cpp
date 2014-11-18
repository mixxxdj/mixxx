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
          m_deckEffectSelectorsSetup(false),
          m_bEqAutoReset(false) {

    // Get the EQ Effect Rack
    m_pEQEffectRack = m_pEffectsManager->getEQEffectRack().data();
    m_eqRackGroup = QString("[EffectRack%1_EffectUnit%2_Effect1]").
            arg(m_pEffectsManager->getEQEffectRackNumber() + 1);

    setupUi(this);
    // Connection
    connect(SliderHiEQ, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateHiEQ()));
    connect(SliderHiEQ, SIGNAL(sliderMoved(int)), this, SLOT(slotUpdateHiEQ()));
    connect(SliderHiEQ, SIGNAL(sliderReleased()), this, SLOT(slotUpdateHiEQ()));

    connect(SliderLoEQ, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateLoEQ()));
    connect(SliderLoEQ, SIGNAL(sliderMoved(int)), this, SLOT(slotUpdateLoEQ()));
    connect(SliderLoEQ, SIGNAL(sliderReleased()), this, SLOT(slotUpdateLoEQ()));

    connect(bEqAutoReset, SIGNAL(stateChanged(int)), this, SLOT(slotUpdateEqAutoReset(int)));
    connect(CheckBoxBypass, SIGNAL(stateChanged(int)), this, SLOT(slotBypass(int)));

    connect(CheckBoxHideEffects, SIGNAL(stateChanged(int)),
            this, SLOT(slotPopulateDeckEffectSelector()));

    connect(this,
            SIGNAL(effectOnChainSlot(const unsigned int, const unsigned int, QString)),
            m_pEQEffectRack,
            SLOT(slotLoadEffectOnChainSlot(const unsigned int, const unsigned int, QString)));

    // Set to basic view if a previous configuration is missing
    CheckBoxHideEffects->setChecked(m_pConfig->getValueString(
            ConfigKey(kConfigKey, "AdvancedView"), QString("no")) == QString("no"));

    // Add drop down lists for current decks and connect num_decks control
    // to slotAddComboBox
    m_pNumDecks = new ControlObjectSlave("[Master]", "num_decks", this);
    m_pNumDecks->connectValueChanged(SLOT(slotNumDecksChanged(double)));
    setupSelectors();

    loadSettings();
    slotUpdate();
    slotApply();
}

DlgPrefEQ::~DlgPrefEQ() {
    delete m_deckEffectSelector;
    delete m_filterWaveformEnableCO;
}

void DlgPrefEQ::setupSelectors() {
    QHBoxLayout* innerHLayout = new QHBoxLayout();
    QLabel* label = new QLabel(QObject::tr("Equalizer Effect"), this);

    m_filterWaveformEnableCO =
            new ControlObject(ConfigKey("[Master]", "filterWaveformEnable"));

    // Create the drop down list and populate it with the available effects
    m_deckEffectSelector = new QComboBox(this);
    connect(m_deckEffectSelector, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotEqEffectChanged(int)));

    // Setup the GUI
    innerHLayout->addWidget(label);
    innerHLayout->addWidget(m_deckEffectSelector);
    innerHLayout->addStretch();
    verticalLayout_2->addLayout(innerHLayout);

     slotPopulateDeckEffectSelector();
    // Set the configured effect for box and simpleBox or Bessel8 LV-Mix EQ
    // if none is configured
    QString configuredEffect;
    int selectedEffectIndex;
    configuredEffect = m_pConfig->getValueString(ConfigKey(kConfigKey,
            "EqEffect"), kDefaultEqId);
    selectedEffectIndex = m_deckEffectSelector->findData(configuredEffect);
    if (selectedEffectIndex < 0) {
        selectedEffectIndex = m_deckEffectSelector->findData(kDefaultEqId);
        configuredEffect = kDefaultEqId;
    }
    m_deckEffectSelector->setCurrentIndex(selectedEffectIndex);
    m_filterWaveformEnableCO->set(m_pEffectsManager->isEQ(configuredEffect));
}

static bool isEQ(EffectManifest* pManifest) {
    return pManifest->isEQ();
}

void DlgPrefEQ::slotPopulateDeckEffectSelector() {
    m_deckEffectSelectorsSetup = true; // prevents a recursive call
    QList<QPair<QString, QString> > availableEQEffectNames;
    EffectsManager::EffectManifestFilterFnc filter;
    if (CheckBoxHideEffects->isChecked()) {
        m_pConfig->set(ConfigKey(kConfigKey, "AdvancedView"), QString("no"));
        filter = isEQ;
    } else {
        m_pConfig->set(ConfigKey(kConfigKey, "AdvancedView"), QString("yes"));
        filter = NULL; // take all;
    }
    availableEQEffectNames =
            m_pEffectsManager->getEffectNamesFiltered(filter);

    // Populate comboboxes with all available effects
    // Save current selection
    QString selectedEffectId = m_deckEffectSelector->itemData(
            m_deckEffectSelector->currentIndex()).toString();
    QString selectedEffectName = m_deckEffectSelector->itemText(
            m_deckEffectSelector->currentIndex());
    m_deckEffectSelector->clear();
    int currentIndex = -1;// Nothing selected

    int i;
    for (i = 0; i < availableEQEffectNames.size(); ++i) {
        m_deckEffectSelector->addItem(availableEQEffectNames[i].second);
        m_deckEffectSelector->setItemData(i, QVariant(availableEQEffectNames[i].first));
        if (selectedEffectId == availableEQEffectNames[i].first) {
            currentIndex = i;
        }
    }
    if (currentIndex < 0 && !selectedEffectName.isEmpty()) {
        // current selection is not part of the new list
        // So we need to add it
        m_deckEffectSelector->addItem(selectedEffectName);
        m_deckEffectSelector->setItemData(i, QVariant(selectedEffectId));
        currentIndex = i;
    }
    m_deckEffectSelector->setCurrentIndex(currentIndex);
    m_deckEffectSelectorsSetup = false;
}

void DlgPrefEQ::loadSettings() {
    QString highEqCourse = m_pConfig->getValueString(ConfigKey(kConfigKey, "HiEQFrequency"));
    QString highEqPrecise = m_pConfig->getValueString(ConfigKey(kConfigKey, "HiEQFrequencyPrecise"));
    QString lowEqCourse = m_pConfig->getValueString(ConfigKey(kConfigKey, "LoEQFrequency"));
    QString lowEqPrecise = m_pConfig->getValueString(ConfigKey(kConfigKey, "LoEQFrequencyPrecise"));
    m_bEqAutoReset = static_cast<bool>(m_pConfig->getValueString(
            ConfigKey(kConfigKey, "EqAutoReset")).toInt());
    bEqAutoReset->setChecked(m_bEqAutoReset);
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
    m_deckEffectSelector->setCurrentIndex(
           m_deckEffectSelector->findData(kDefaultEqId));
    loadSettings();
    CheckBoxBypass->setChecked(Qt::Unchecked);
    CheckBoxHideEffects->setChecked(Qt::Checked);
    m_bEqAutoReset = false;
    bEqAutoReset->setChecked(Qt::Unchecked);
    slotUpdate();
    slotApply();
}

void DlgPrefEQ::slotEqEffectChanged(int effectIndex) {
    // Check if qobject_cast was successful
    if (!m_deckEffectSelectorsSetup) {
        QString effectId = m_deckEffectSelector->itemData(effectIndex).toString();
        for (int i = 0; i < static_cast<int>(m_pNumDecks->get()); ++i) {
            emit(effectOnChainSlot(i, 0, effectId));
        }

        // Update the configured effect for the current QComboBox
        m_pConfig->set(ConfigKey(kConfigKey, "EqEffect"),
                ConfigValue(effectId));

        m_filterWaveformEnableCO->set(m_pEffectsManager->isEQ(effectId));

        // This is required to remove a previous selected effect that does not
        // fit to the current ShowAllEffects checkbox
        slotPopulateDeckEffectSelector();
    }
}

void DlgPrefEQ::slotNumDecksChanged(double num_decks) {
    Q_UNUSED(num_decks);
    // Make sure all the decks are using the same EQ effect.
    slotEqEffectChanged(m_deckEffectSelector->currentIndex());
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
    slotPopulateDeckEffectSelector();
    bEqAutoReset->setChecked(m_bEqAutoReset);
}

void DlgPrefEQ::slotUpdateEqAutoReset(int i) {
    m_bEqAutoReset = static_cast<bool>(i);
}

void DlgPrefEQ::slotBypass(int state) {
    if (state) {
        m_pConfig->set(ConfigKey(kConfigKey, kEnableEqs), QString("no"));
        // Disable effect processing for all decks by setting the appropriate
        // controls to 0 ("[EffectRackX_EffectUnitDeck_Effect1],enable")
        for (int i = 0; i < static_cast<int>(m_pNumDecks->get()); ++i) {
            ControlObject::set(ConfigKey(m_eqRackGroup.arg(i), "enabled"), 0);
        }
        m_filterWaveformEnableCO->set(0);
        m_deckEffectSelector->setEnabled(false);
    } else {
        m_pConfig->set(ConfigKey(kConfigKey, kEnableEqs), QString("yes"));
        // Enable effect processing for all decks by setting the appropriate
        // controls to 1 ("[EffectRackX_EffectUnitDeck_Effect1],enable")
        ControlObjectSlave enableControl;
        for (int i = 0; i < static_cast<int>(m_pNumDecks->get()); ++i) {
            ControlObject::set(ConfigKey(m_eqRackGroup.arg(i), "enabled"), 1);
        }
        m_filterWaveformEnableCO->set(1);
        m_deckEffectSelector->setEnabled(true);
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
