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
const char* kEqsOnly = "EQsOnly";
const char* kSingleEq = "SingleEQEffect";
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

    connect(CheckBoxEqOnly, SIGNAL(stateChanged(int)),
            this, SLOT(slotPopulateDeckEffectSelectors()));

    connect(CheckBoxSingleEqEffect, SIGNAL(stateChanged(int)),
            this, SLOT(slotSingleEqChecked(int)));

    connect(this,
            SIGNAL(effectOnChainSlot(const unsigned int, const unsigned int, QString)),
            m_pEQEffectRack,
            SLOT(slotLoadEffectOnChainSlot(const unsigned int, const unsigned int, QString)));

    // Add drop down lists for current decks and connect num_decks control
    // to slotAddComboBox
    m_pNumDecks = new ControlObjectSlave("[Master]", "num_decks", this);
    m_pNumDecks->connectValueChanged(SLOT(slotNumDecksChanged(double)));
    slotNumDecksChanged(m_pNumDecks->get());

    loadSettings();
    slotUpdate();
    slotApply();
}

DlgPrefEQ::~DlgPrefEQ() {
    qDeleteAll(m_deckEffectSelectors);
    m_deckEffectSelectors.clear();

    qDeleteAll(m_filterWaveformEnableCOs);
    m_filterWaveformEnableCOs.clear();

    qDeleteAll(m_deckSelectorContainers);
    m_deckSelectorContainers.clear();
}

void DlgPrefEQ::slotNumDecksChanged(double numDecks) {
    int oldDecks = m_deckEffectSelectors.size();
    while (m_deckEffectSelectors.size() < static_cast<int>(numDecks)) {
        QWidget* container = new QWidget();
        m_deckSelectorContainers.append(container);

        QHBoxLayout* innerHLayout = new QHBoxLayout(container);

        QLabel* label = new QLabel(QObject::tr("Deck %1 EQ Effect").
                            arg(m_deckEffectSelectors.size() + 1), this);

        QString group = PlayerManager::groupForDeck(
                m_deckEffectSelectors.size());

        m_filterWaveformEnableCOs.append(
                new ControlObject(ConfigKey(group, "filterWaveformEnable")));

        // Create the drop down list and populate it with the available effects
        QComboBox* box = new QComboBox(this);
        m_deckEffectSelectors.append(box);
        connect(box, SIGNAL(currentIndexChanged(int)),
                this, SLOT(slotEffectChangedOnDeck(int)));

        // Setup the GUI
        innerHLayout->addWidget(label);
        innerHLayout->addWidget(box);
        innerHLayout->addStretch();
        verticalLayout_2->addWidget(container);
        if (m_deckEffectSelectors.size() == 1) {
            m_firstSelectorLabel = label;
            if (CheckBoxEqOnly->isChecked()) {
                m_firstSelectorLabel->setText(QObject::tr("EQ Effect"));
            }
        }
        if (m_deckEffectSelectors.size() > 1 && CheckBoxEqOnly->isChecked()) {
            container->hide();
        }
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
        selectedEffectIndex = m_deckEffectSelectors[i]->findData(configuredEffect);
        if (selectedEffectIndex < 0) {
            selectedEffectIndex = m_deckEffectSelectors[i]->findData(kDefaultEqId);
            configuredEffect = kDefaultEqId;
        }
        m_deckEffectSelectors[i]->setCurrentIndex(selectedEffectIndex);
        m_filterWaveformEnableCOs[i]->set(m_pEffectsManager->isEQ(configuredEffect));
    }
    applyEqSelections();
}

static bool isEQ(EffectManifest* pManifest) {
    return pManifest->isEQ();
}

void DlgPrefEQ::slotPopulateDeckEffectSelectors() {
    m_deckEffectSelectorsSetup = true; // prevents a recursive call
    QList<QPair<QString, QString> > availableEQEffectNames;
    EffectsManager::EffectManifestFilterFnc filter;
    if (CheckBoxEqOnly->isChecked()) {
        m_pConfig->set(ConfigKey(kConfigKey, kEqsOnly), QString("yes"));
        filter = isEQ;
    } else {
        m_pConfig->set(ConfigKey(kConfigKey, kEqsOnly), QString("no"));
        filter = NULL; // take all;
    }
    availableEQEffectNames =
            m_pEffectsManager->getEffectNamesFiltered(filter);

    foreach (QComboBox* box, m_deckEffectSelectors) {
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
    m_deckEffectSelectorsSetup = false;
}

void DlgPrefEQ::slotSingleEqChecked(int checked) {
    bool do_hide = static_cast<bool>(checked);
    m_pConfig->set(ConfigKey(kConfigKey, kSingleEq),
                   do_hide ? QString("yes") : QString("no"));
    for (int i = 1; i < m_deckSelectorContainers.size(); ++i) {
        if (do_hide) {
            m_deckSelectorContainers[i]->hide();
        } else {
            m_deckSelectorContainers[i]->show();
        }
    }

    if (do_hide) {
        m_firstSelectorLabel->setText(QObject::tr("EQ Effect"));
    } else {
        m_firstSelectorLabel->setText(QObject::tr("Deck 1 EQ Effect"));
    }

    applyEqSelections();
}

void DlgPrefEQ::loadSettings() {
    QString highEqCourse = m_pConfig->getValueString(ConfigKey(kConfigKey, "HiEQFrequency"));
    QString highEqPrecise = m_pConfig->getValueString(ConfigKey(kConfigKey, "HiEQFrequencyPrecise"));
    QString lowEqCourse = m_pConfig->getValueString(ConfigKey(kConfigKey, "LoEQFrequency"));
    QString lowEqPrecise = m_pConfig->getValueString(ConfigKey(kConfigKey, "LoEQFrequencyPrecise"));
    m_bEqAutoReset = static_cast<bool>(m_pConfig->getValueString(
            ConfigKey(kConfigKey, "EqAutoReset")).toInt());
    bEqAutoReset->setChecked(m_bEqAutoReset);

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

    CheckBoxBypass->setChecked(m_pConfig->getValueString(
            ConfigKey(kConfigKey, kEnableEqs), "yes") == "no");

    CheckBoxEqOnly->setChecked(m_pConfig->getValueString(
            ConfigKey(kConfigKey, kEqsOnly), "yes") == "yes");

    CheckBoxSingleEqEffect->setChecked(m_pConfig->getValueString(
            ConfigKey(kConfigKey, kSingleEq), "yes") == "yes");
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
    foreach(QComboBox* pCombo, m_deckEffectSelectors) {
        pCombo->setCurrentIndex(
               pCombo->findData(kDefaultEqId));
    }
    loadSettings();
    CheckBoxBypass->setChecked(Qt::Unchecked);
    CheckBoxEqOnly->setChecked(Qt::Checked);
    CheckBoxSingleEqEffect->setChecked(Qt::Checked);
    m_bEqAutoReset = false;
    bEqAutoReset->setChecked(Qt::Unchecked);
    slotUpdate();
    slotApply();
}

void DlgPrefEQ::slotEffectChangedOnDeck(int effectIndex) {
    QComboBox* c = qobject_cast<QComboBox*>(sender());
    // Check if qobject_cast was successful
    if (c && !m_deckEffectSelectorsSetup) {
        int deckNumber = m_deckEffectSelectors.indexOf(c);
        QString effectId = c->itemData(effectIndex).toString();
        emit(effectOnChainSlot(deckNumber, 0, effectId));

        QString group = PlayerManager::groupForDeck(deckNumber);

        // Update the configured effect for the current QComboBox
        m_pConfig->set(ConfigKey(kConfigKey, "EffectForGroup_" + group),
                ConfigValue(effectId));

        m_filterWaveformEnableCOs[deckNumber]->set(m_pEffectsManager->isEQ(effectId));

        // This is required to remove a previous selected effect that does not
        // fit to the current ShowAllEffects checkbox
        slotPopulateDeckEffectSelectors();
    }
}

void DlgPrefEQ::applyEqSelections() {
    int deck = 0;
    QString firstEffectId;
    foreach(QComboBox* box, m_deckEffectSelectors) {
        if (!m_deckEffectSelectorsSetup) {
            QString effectId = box->itemData(box->currentIndex()).toString();
            if (deck == 0) {
                firstEffectId = effectId;
            } else if (CheckBoxSingleEqEffect->isChecked()) {
                effectId = firstEffectId;
            }
            emit(effectOnChainSlot(deck, 0, effectId));

            QString group = PlayerManager::groupForDeck(deck);

            // Update the configured effect for the current QComboBox, unless
            // we're reusing the first deck id.
            if (deck == 0 || !CheckBoxSingleEqEffect->isChecked()) {
                m_pConfig->set(ConfigKey(kConfigKey, "EffectForGroup_" + group),
                        ConfigValue(effectId));
            }

            m_filterWaveformEnableCOs[deck]->set(m_pEffectsManager->isEQ(effectId));

            // This is required to remove a previous selected effect that does not
            // fit to the current ShowAllEffects checkbox
            slotPopulateDeckEffectSelectors();
        }
        ++deck;
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
    applyEqSelections();
}

// supposed to set the widgets to match internal state
void DlgPrefEQ::slotUpdate() {
    slotUpdateLoEQ();
    slotUpdateHiEQ();
    slotPopulateDeckEffectSelectors();
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
        for (int deck = 0; deck < m_deckEffectSelectors.size(); ++deck) {
            ControlObject::set(ConfigKey(m_eqRackGroup.arg(deck), "enabled"), 0);
            m_filterWaveformEnableCOs[deck]->set(0);
            m_deckSelectorContainers.at(deck)->setEnabled(false);
        }
        CheckBoxEqOnly->setEnabled(false);
        CheckBoxSingleEqEffect->setEnabled(false);
    } else {
        m_pConfig->set(ConfigKey(kConfigKey, kEnableEqs), QString("yes"));
        // Enable effect processing for all decks by setting the appropriate
        // controls to 1 ("[EffectRackX_EffectUnitDeck_Effect1],enable")
        ControlObjectSlave enableControl;
        for (int deck = 0; deck < m_deckEffectSelectors.size(); ++deck) {
            ControlObject::set(ConfigKey(m_eqRackGroup.arg(deck), "enabled"), 1);
            m_filterWaveformEnableCOs[deck]->set(1);
            m_deckSelectorContainers.at(deck)->setEnabled(true);
        }
        CheckBoxEqOnly->setEnabled(true);
        CheckBoxSingleEqEffect->setEnabled(true);
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
