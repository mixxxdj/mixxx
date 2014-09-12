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

#define CONFIG_KEY "[Mixer Profile]"
#define ENABLE_INTERNAL_EQ "EnableEQs"

const int kFrequencyUpperLimit = 20050;
const int kFrequencyLowerLimit = 16;

DlgPrefEQ::DlgPrefEQ(QWidget* pParent, EffectsManager* pEffectsManager,
                     ConfigObject<ConfigValue>* pConfig)
        : DlgPreferencePage(pParent),
          m_COTLoFreq(CONFIG_KEY, "LoEQFrequency"),
          m_COTHiFreq(CONFIG_KEY, "HiEQFrequency"),
          m_COTEnableEq(CONFIG_KEY, ENABLE_INTERNAL_EQ),
          m_pConfig(pConfig),
          m_lowEqFreq(0.0),
          m_highEqFreq(0.0),
          m_pEffectsManager(pEffectsManager) {

    // Get the EQ Effect Rack
    m_pEQEffectRack = m_pEffectsManager->getEQEffectRack().data();
    m_eqRackGroup = QString("[EffectRack%1_EffectUnit%2_Effect1]").
            arg(m_pEffectsManager->getEQEffectRackNumber());

    setupUi(this);

    // Connection
    connect(SliderHiEQ, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateHiEQ()));
    connect(SliderHiEQ, SIGNAL(sliderMoved(int)), this, SLOT(slotUpdateHiEQ()));
    connect(SliderHiEQ, SIGNAL(sliderReleased()), this, SLOT(slotUpdateHiEQ()));

    connect(SliderLoEQ, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateLoEQ()));
    connect(SliderLoEQ, SIGNAL(sliderMoved(int)), this, SLOT(slotUpdateLoEQ()));
    connect(SliderLoEQ, SIGNAL(sliderReleased()), this, SLOT(slotUpdateLoEQ()));

    connect(CheckBoxBypass, SIGNAL(stateChanged(int)), this, SLOT(slotBypass(int)));

    connect(CheckBoxShowAllEffects, SIGNAL(stateChanged(int)),
            this, SLOT(slotShowAllEffects()));

    connect(this,
            SIGNAL(effectOnChainSlot(const unsigned int, const unsigned int, QString)),
            m_pEQEffectRack,
            SLOT(slotLoadEffectOnChainSlot(const unsigned int, const unsigned int, QString)));

    // Set to basic view if a previous configuration is missing
    CheckBoxShowAllEffects->setChecked(m_pConfig->getValueString(
            ConfigKey(CONFIG_KEY, "AdvancedView"), QString("no")) == QString("yes"));

    // Add drop down lists for current decks and connect num_decks control
    // to slotAddComboBox
    m_pNumDecks = new ControlObjectSlave("[Master]", "num_decks", this);
    m_pNumDecks->connectValueChanged(SLOT(slotAddComboBox(double)));
    slotAddComboBox(m_pNumDecks->get());

    // Restore the state of Bypass check box
    CheckBoxBypass->setChecked(m_pConfig->getValueString(
            ConfigKey(CONFIG_KEY, ENABLE_INTERNAL_EQ), QString("no")) == QString("no"));
    if (CheckBoxBypass->isChecked()) {
        slotBypass(Qt::Checked);
    } else {
        slotBypass(Qt::Unchecked);
    }

    loadSettings();
    slotUpdate();
    slotApply();
}

DlgPrefEQ::~DlgPrefEQ() {
    qDeleteAll(m_deckEffectSelectors);
    m_deckEffectSelectors.clear();
}

void DlgPrefEQ::slotAddComboBox(double numDecks) {
    while (m_deckEffectSelectors.size() < static_cast<int>(numDecks)) {
        QHBoxLayout* innerHLayout = new QHBoxLayout();
        QLabel* label = new QLabel(QObject::tr("Deck %1").
                            arg(m_deckEffectSelectors.size() + 1), this);

        // Create the drop down list and populate it with the available effects
        QComboBox* box = new QComboBox(this);
        QList<QPair<QString, QString> > availableEffectNames =
                m_pEffectsManager->getAvailableEffectNames().toList();
        for (int i = 0; i < availableEffectNames.size(); ++i) {
            box->addItem(availableEffectNames[i].second);
            box->setItemData(i, QVariant(availableEffectNames[i].first));
        }
        m_deckEffectSelectors.append(box);
        connect(box, SIGNAL(currentIndexChanged(int)),
                this, SLOT(slotEffectChangedOnDeck(int)));

        // Create the drop down list for basic view
        // Add EQ Effects only
        QComboBox* simpleBox = new QComboBox(this);
        QList<QPair<QString, QString> > availableEQEffectNames =
                m_pEffectsManager->getAvailableEQEffectNames().toList();
        for (int i = 0; i < availableEQEffectNames.size(); ++i) {
            simpleBox->addItem(availableEQEffectNames[i].second);
            simpleBox->setItemData(i, QVariant(availableEQEffectNames[i].first));
        }
        m_deckBasicEffectSelectors.append(simpleBox);
        connect(simpleBox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(slotBasicEffectChangedOnDeck(int)));

        // Set the configured effect for box and simpleBox or Butterworth8 EQ
        // if none is configured
        QString configuredEffect;
        int selectedEffectIndex;
        configuredEffect = m_pConfig->getValueString(ConfigKey(CONFIG_KEY,
                QString("EffectForDeck%1").arg(m_deckEffectSelectors.size())),
                QString("Butterworth8 EQ"));
        selectedEffectIndex = box->findText(configuredEffect);
        box->setCurrentIndex(selectedEffectIndex);

        configuredEffect = m_pConfig->getValueString(ConfigKey(CONFIG_KEY,
                QString("BasicEffectForDeck%1").arg(m_deckBasicEffectSelectors.size())),
                QString("Butterworth8 EQ"));
        selectedEffectIndex = simpleBox->findText(configuredEffect);
        simpleBox->setCurrentIndex(selectedEffectIndex);

        // Force the selected effect on the Effect Rack based on user's preference
        bool advancedView = CheckBoxShowAllEffects->isChecked();
        if (advancedView) {
            simpleBox->setVisible(false);
            emit(effectOnChainSlot(m_deckEffectSelectors.size() - 1, 0,
                                   box->itemData(box->currentIndex()).toString()));
        } else {
            box->setVisible(false);
            emit(effectOnChainSlot(m_deckBasicEffectSelectors.size() - 1, 0,
                                   simpleBox->itemData(simpleBox->currentIndex()).toString()));
        }

        // Setup the GUI
        innerHLayout->addWidget(label);
        innerHLayout->addWidget(box);
        innerHLayout->addWidget(simpleBox);
        innerHLayout->addStretch();
        verticalLayout_2->addLayout(innerHLayout);
    }
}

void DlgPrefEQ::slotShowAllEffects() {
    if (!CheckBoxShowAllEffects->isChecked()) {
        m_pConfig->set(ConfigKey(CONFIG_KEY, "AdvancedView"), QString("no"));
        foreach (QComboBox* box, m_deckEffectSelectors) {
            box->setVisible(false);
        }
        foreach (QComboBox* basicBox, m_deckBasicEffectSelectors) {
            basicBox->setVisible(true);
            emit(effectOnChainSlot(m_deckBasicEffectSelectors.indexOf(basicBox),
                                   0, basicBox->itemData(basicBox->currentIndex()).toString()));
        }
    } else {
        m_pConfig->set(ConfigKey(CONFIG_KEY, "AdvancedView"), QString("yes"));
        foreach (QComboBox* box, m_deckEffectSelectors) {
            box->setVisible(true);
            emit(effectOnChainSlot(m_deckEffectSelectors.indexOf(box),
                                   0, box->itemData(box->currentIndex()).toString()));
        }
        foreach (QComboBox* basicBox, m_deckBasicEffectSelectors) {
            basicBox->setVisible(false);
        }
    }
}

void DlgPrefEQ::loadSettings() {
    QString highEqCourse = m_pConfig->getValueString(ConfigKey(CONFIG_KEY, "HiEQFrequency"));
    QString highEqPrecise = m_pConfig->getValueString(ConfigKey(CONFIG_KEY, "HiEQFrequencyPrecise"));
    QString lowEqCourse = m_pConfig->getValueString(ConfigKey(CONFIG_KEY, "LoEQFrequency"));
    QString lowEqPrecise = m_pConfig->getValueString(ConfigKey(CONFIG_KEY, "LoEQFrequencyPrecise"));

    double lowEqFreq = 0.0;
    double highEqFreq = 0.0;

    // Precise takes precedence over course.
    lowEqFreq = lowEqCourse.isEmpty() ? lowEqFreq : lowEqCourse.toDouble();
    lowEqFreq = lowEqPrecise.isEmpty() ? lowEqFreq : lowEqPrecise.toDouble();
    highEqFreq = highEqCourse.isEmpty() ? highEqFreq : highEqCourse.toDouble();
    highEqFreq = highEqPrecise.isEmpty() ? highEqFreq : highEqPrecise.toDouble();

    if (lowEqFreq == 0.0 || highEqFreq == 0.0 || lowEqFreq == highEqFreq) {
        setDefaultShelves();
        lowEqFreq = m_pConfig->getValueString(ConfigKey(CONFIG_KEY, "LoEQFrequencyPrecise")).toDouble();
        highEqFreq = m_pConfig->getValueString(ConfigKey(CONFIG_KEY, "HiEQFrequencyPrecise")).toDouble();
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
            ConfigKey(CONFIG_KEY, ENABLE_INTERNAL_EQ), "yes") == QString("yes")) {
        CheckBoxBypass->setChecked(false);
    }
}

void DlgPrefEQ::setDefaultShelves()
{
    m_pConfig->set(ConfigKey(CONFIG_KEY, "HiEQFrequency"), ConfigValue(2500));
    m_pConfig->set(ConfigKey(CONFIG_KEY, "LoEQFrequency"), ConfigValue(250));
    m_pConfig->set(ConfigKey(CONFIG_KEY, "HiEQFrequencyPrecise"), ConfigValue(2500.0));
    m_pConfig->set(ConfigKey(CONFIG_KEY, "LoEQFrequencyPrecise"), ConfigValue(250.0));
}

void DlgPrefEQ::slotResetToDefaults() {
    setDefaultShelves();
    loadSettings();
    slotUpdate();
    slotApply();
}

void DlgPrefEQ::slotEffectChangedOnDeck(int effectIndex) {
    QComboBox* c = qobject_cast<QComboBox*>(sender());
    // Check if qobject_cast was successful
    if (c) {
        int deckNumber = m_deckEffectSelectors.indexOf(c);
        QString effectId = c->itemData(effectIndex).toString();
        emit(effectOnChainSlot(deckNumber, 0, effectId));

        // Update the configured effect for the current QComboBox
        m_pConfig->set(ConfigKey(CONFIG_KEY, QString("EffectForDeck%1").
                       arg(deckNumber + 1)), ConfigValue(c->currentText()));
    }
}

void DlgPrefEQ::slotBasicEffectChangedOnDeck(int effectIndex) {
    QComboBox* c = qobject_cast<QComboBox*>(sender());
    // Check if qobject_cast was successful
    if (c) {
        int deckNumber = m_deckBasicEffectSelectors.indexOf(c);
        QString effectId = c->itemData(effectIndex).toString();
        emit(effectOnChainSlot(deckNumber, 0, effectId));

        // Update the configured effect for the current QComboBox
        m_pConfig->set(ConfigKey(CONFIG_KEY, QString("BasicEffectForDeck%1").
                       arg(deckNumber + 1)), ConfigValue(c->currentText()));
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
    m_pConfig->set(ConfigKey(CONFIG_KEY, "HiEQFrequency"),
                   ConfigValue(QString::number(static_cast<int>(m_highEqFreq))));
    m_pConfig->set(ConfigKey(CONFIG_KEY, "HiEQFrequencyPrecise"),
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
    m_pConfig->set(ConfigKey(CONFIG_KEY, "LoEQFrequency"),
                   ConfigValue(QString::number(static_cast<int>(m_lowEqFreq))));
    m_pConfig->set(ConfigKey(CONFIG_KEY, "LoEQFrequencyPrecise"),
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
    m_COTLoFreq.slotSet(m_lowEqFreq);
    m_COTHiFreq.slotSet(m_highEqFreq);

    m_COTEnableEq.slotSet((m_pConfig->getValueString(
            ConfigKey(CONFIG_KEY, ENABLE_INTERNAL_EQ), "yes") == QString("yes")));
}

void DlgPrefEQ::slotUpdate() {
    slotUpdateLoEQ();
    slotUpdateHiEQ();
}

void DlgPrefEQ::slotBypass(int state) {
    if (state) {
        m_pConfig->set(ConfigKey(CONFIG_KEY, ENABLE_INTERNAL_EQ), QString("no"));
        // Disable effect processing for all decks by setting the appropriate
        // controls to 0 ("[EffectRackX_EffectUnitDeck_Effect1],enable")
        int deck = 1;
        ControlObjectSlave disableControl;
        foreach(QComboBox* box, m_deckEffectSelectors) {
            disableControl.initialize(ConfigKey(m_eqRackGroup.arg(deck), "enabled"));
            disableControl.set(0);
            deck++;
            box->setEnabled(false);
        }

        foreach(QComboBox* box, m_deckBasicEffectSelectors) {
            box->setEnabled(false);
        }

    } else {
        m_pConfig->set(ConfigKey(CONFIG_KEY, ENABLE_INTERNAL_EQ), QString("yes"));
        // Enable effect processing for all decks by setting the appropriate
        // controls to 1 ("[EffectRackX_EffectUnitDeck_Effect1],enable")
        int deck = 1;
        ControlObjectSlave enableControl;
        foreach(QComboBox* box, m_deckEffectSelectors) {
            enableControl.initialize(ConfigKey(m_eqRackGroup.arg(deck), "enabled"));
            enableControl.set(1);
            deck++;
            box->setEnabled(true);
        }

        foreach(QComboBox* box, m_deckBasicEffectSelectors) {
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
    double result = normValue * (kFrequencyUpperLimit-kFrequencyLowerLimit) +
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
