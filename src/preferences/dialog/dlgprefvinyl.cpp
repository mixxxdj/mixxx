#include "preferences/dialog/dlgprefvinyl.h"

#include <QtDebug>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "control/pollingcontrolproxy.h"
#include "defs_urls.h"
#include "mixer/playermanager.h"
#include "moc_dlgprefvinyl.cpp"
#include "util/defs.h"
#include "vinylcontrol/defs_vinylcontrol.h"
#include "vinylcontrol/vinylcontrolmanager.h"
#include "vinylcontrol/vinylcontrolsignalwidget.h"

DlgPrefVinyl::DlgPrefVinyl(
        QWidget* parent,
        std::shared_ptr<VinylControlManager> pVCMan,
        UserSettingsPointer config)
        : DlgPreferencePage(parent),
          m_pVCManager(pVCMan),
          config(config) {
    m_pNumDecks = new ControlProxy(QStringLiteral("[App]"), QStringLiteral("num_decks"), this);
    m_pNumDecks->connectValueChanged(this, &DlgPrefVinyl::slotNumDecksChanged);

    setupUi(this);
    // Create text color for the Troubleshooting link
    createLinkColor();

    // Add per-deck vinyl selectors
    m_vcLabels = {VinylLabel1,
            VinylLabel2,
            VinylLabel3,
            VinylLabel4};

    m_vcTypeBoxes = {ComboBoxVinylType1,
            ComboBoxVinylType2,
            ComboBoxVinylType3,
            ComboBoxVinylType4};
    for (auto* box : std::as_const(m_vcTypeBoxes)) {
        box->addItem(kVinylAlgoriddimA);
        box->addItem(kVinylAlgoriddimB);
        box->addItem(kVinylMixVibes7Inch);
        box->addItem(kVinylMixVibesDvs);
        box->addItem(kVinylPioneerA);
        box->addItem(kVinylPioneerB);
        box->addItem(kVinylSeratoCd);
        box->addItem(kVinylSeratoCv02SideA);
        box->addItem(kVinylSeratoCv02SideB);
        box->addItem(kVinylTraktorScratchMk1SideA);
        box->addItem(kVinylTraktorScratchMk1SideB);
        box->addItem(kVinylTraktorScratchMk2Cd);
        box->addItem(kVinylTraktorScratchMk2SideA);
        box->addItem(kVinylTraktorScratchMk2SideB);

        // Use Serato as initial constructor value. Gets overwritten by config in slotUpdate()
        box->setCurrentText(kVinylSeratoCv02SideA);
        connect(box,
                &QComboBox::currentTextChanged,
                this,
                &DlgPrefVinyl::slotVinylTypeChanged);
    }

    m_vcSpeedBoxes = {ComboBoxVinylSpeed1,
            ComboBoxVinylSpeed2,
            ComboBoxVinylSpeed3,
            ComboBoxVinylSpeed4};
    for (auto* box : std::as_const(m_vcSpeedBoxes)) {
        box->addItem(kVinylSpeed33);
        box->addItem(kVinylSpeed45);
    }

    m_vcPitchEstimatorBoxes = {ComboBoxPitchEstimator1,
            ComboBoxPitchEstimator2,
            ComboBoxPitchEstimator3,
            ComboBoxPitchEstimator4};
    for (auto* box : std::as_const(m_vcPitchEstimatorBoxes)) {
        box->addItem(kVinylPitchFilterKalman);
        box->addItem(kVinylPitchFilterLegacy);
    }

    m_vcLeadInBoxes = {LeadinTime1, LeadinTime2, LeadinTime3, LeadinTime4};
    for (auto* box : std::as_const(m_vcLeadInBoxes)) {
        box->setSuffix(" s");
    }

    DEBUG_ASSERT(m_vcLabels.length() == kMaximumVinylControlInputs);
    DEBUG_ASSERT(m_vcTypeBoxes.length() == kMaximumVinylControlInputs);
    DEBUG_ASSERT(m_vcSpeedBoxes.length() == kMaximumVinylControlInputs);
    DEBUG_ASSERT(m_vcPitchEstimatorBoxes.length() == kMaximumVinylControlInputs);
    DEBUG_ASSERT(m_vcLeadInBoxes.length() == kMaximumVinylControlInputs);

    for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
        VinylControlSignalWidget* widget = new VinylControlSignalWidget();
        m_signalWidgets.push_back(widget);
        widget->setVinylInput(i);
        widget->setSize(kVinylScopeSize);
        signalQualityLayout->addWidget(widget);
    }

    TroubleshootingLink->setText(coloredLinkString(
            m_pLinkColor,
            // QStringLiteral("Troubleshooting") fails to compile on Fedora 36 with GCC 12.0.x
            "Troubleshooting",
            MIXXX_MANUAL_VINYL_TROUBLESHOOTING_URL));

    connect(SliderVinylGain, &QSlider::sliderReleased, this, &DlgPrefVinyl::slotVinylGainApply);
    connect(SliderVinylGain,
            QOverload<int>::of(&QSlider::valueChanged),
            this,
            &DlgPrefVinyl::slotUpdateVinylGain);

    for (int i = 0; i < kMaxNumberOfDecks; ++i) {
        setDeckWidgetsVisible(i, false);
    }

    setScrollSafeGuardForAllInputWidgets(this);

    slotNumDecksChanged(m_pNumDecks->get());
}

DlgPrefVinyl::~DlgPrefVinyl() {
    qDeleteAll(m_COSpeeds);
    qDeleteAll(m_signalWidgets);
}

void DlgPrefVinyl::slotNumDecksChanged(double dNumDecks) {
    int num_decks = static_cast<int>(dNumDecks);

    if (num_decks < 0 || num_decks > kMaxNumberOfDecks) {
        return;
    }

    for (int i = m_COSpeeds.length(); i < num_decks; ++i) {
        QString group = PlayerManager::groupForDeck(i);
        m_COSpeeds.push_back(new PollingControlProxy(group, "vinylcontrol_speed_type"));
        setDeckWidgetsVisible(i, true);
    }
}

void DlgPrefVinyl::slotVinylTypeChanged(const QString& text) {
    QComboBox* c = qobject_cast<QComboBox*>(sender());
    int i = m_vcTypeBoxes.indexOf(c);
    m_vcLeadInBoxes[i]->setValue(getDefaultLeadIn(text));
}

/// Performs any necessary actions that need to happen when the prefs dialog is opened.
void DlgPrefVinyl::slotShow() {
    if (m_pVCManager) {
        for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
            m_pVCManager->addSignalQualityListener(m_signalWidgets[i]);
        }
    }

    // (Re)Initialize the signal quality indicators
    for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
        m_signalWidgets[i]->resetWidget();
    }
}

/** Performs any necessary actions that need to happen when the prefs dialog is closed. */
void DlgPrefVinyl::slotHide() {
    if (m_pVCManager) {
        for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
            m_pVCManager->removeSignalQualityListener(m_signalWidgets[i]);
        }
    }
}

void DlgPrefVinyl::slotResetToDefaults() {
    // Default to Serato Side A.
    for (auto* pBox : std::as_const(m_vcTypeBoxes)) {
        pBox->setCurrentText(kVinylSeratoCv02SideA);
    }

    // Default to 33 RPM.
    for (auto* pBox : std::as_const(m_vcSpeedBoxes)) {
        pBox->setCurrentText(kVinylSpeed33);
    }

    for (auto* pBox : std::as_const(m_vcLeadInBoxes)) {
        pBox->setValue(kVinylSeratoCv02SideALeadIn);
    }

    SignalQualityEnable->setChecked(true);
    SliderVinylGain->setValue(0);
    slotUpdateVinylGain();
}

void DlgPrefVinyl::slotUpdate() {
    // set vinyl control gain
    const double ratioGain = config->getValue<double>(ConfigKey(kVinylPrefKey, "gain"));
    const double dbGain = ratio2db(ratioGain);
    SliderVinylGain->setValue(static_cast<int>(dbGain + 0.5));
    slotUpdateVinylGain();

    SignalQualityEnable->setChecked(
            (bool)config->getValue<bool>(ConfigKey(kVinylPrefKey, "show_signal_quality")));

    for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
        QString group = PlayerManager::groupForDeck(i);

        // Set vinyl control types in the comboboxes
        int comboIndex =
                m_vcTypeBoxes[i]->findText(config->getValueString(
                        ConfigKey(group, "vinylcontrol_vinyl_type")));
        if (comboIndex != -1) {
            m_vcTypeBoxes[i]->setCurrentIndex(comboIndex);
        }

        // set speed
        comboIndex =
                m_vcSpeedBoxes[i]->findText(config->getValueString(
                        ConfigKey(group, "vinylcontrol_speed_type")));
        if (comboIndex != -1) {
            m_vcSpeedBoxes[i]->setCurrentIndex(comboIndex);
        }

        // Set pitch filter types
        comboIndex =
                m_vcPitchEstimatorBoxes[i]->findText(config->getValueString(
                        ConfigKey(group, "vinylcontrol_pitch_estimator_type")));
        if (comboIndex != -1) {
            m_vcPitchEstimatorBoxes[i]->setCurrentIndex(comboIndex);
        }

        // set lead-in time
        int leadIn = config->getValue(
                ConfigKey(group, "vinylcontrol_lead_in_time"),
                getDefaultLeadIn(m_vcTypeBoxes[i]->currentText()));
        m_vcLeadInBoxes[i]->setValue(leadIn);

        m_signalWidgets[i]->setVinylActive(m_pVCManager->vinylInputConnected(i));
    }
}

void DlgPrefVinyl::verifyAndSaveLeadInTime(
        int deck, const QString& group) {
    bool isInteger;
    int iLeadIn = m_vcLeadInBoxes[deck]->cleanText().toInt(&isInteger);
    if (!isInteger || iLeadIn < 0) {
        iLeadIn = getDefaultLeadIn(m_vcTypeBoxes[deck]->currentText());
    }
    config->setValue(ConfigKey(group, "vinylcontrol_lead_in_time"), iLeadIn);
}

int DlgPrefVinyl::getDefaultLeadIn(const QString& vinyl_type) const {
    if (vinyl_type == kVinylAlgoriddimA) {
        return kVinylAlgoriddimALeadIn;
    } else if (vinyl_type == kVinylAlgoriddimB) {
        return kVinylAlgoriddimBLeadIn;
    } else if (vinyl_type == kVinylMixVibes7Inch) {
        return kVinylMixVibes7InchLeadIn;
    } else if (vinyl_type == kVinylMixVibesDvs) {
        return kVinylMixVibesDvsLeadIn;
    } else if (vinyl_type == kVinylPioneerA) {
        return kVinylPioneerALeadIn;
    } else if (vinyl_type == kVinylPioneerB) {
        return kVinylPioneerBLeadIn;
    } else if (vinyl_type == kVinylSeratoCd) {
        return kVinylSeratoCdLeadIn;
    } else if (vinyl_type == kVinylSeratoCv02SideA) {
        return kVinylSeratoCv02SideALeadIn;
    } else if (vinyl_type == kVinylSeratoCv02SideB) {
        return kVinylSeratoCv02SideBLeadIn;
    } else if (vinyl_type == kVinylTraktorScratchMk1SideA) {
        return kVinylTraktorScratchMk1SideALeadIn;
    } else if (vinyl_type == kVinylTraktorScratchMk1SideB) {
        return kVinylTraktorScratchMk1SideBLeadIn;
    } else if (vinyl_type == kVinylTraktorScratchMk2Cd) {
        return kVinylTraktorScratchMk2CdLeadIn;
    } else if (vinyl_type == kVinylTraktorScratchMk2SideA) {
        return kVinylTraktorScratchMk2SideALeadIn;
    } else if (vinyl_type == kVinylTraktorScratchMk2SideB) {
        return kVinylTraktorScratchMk2SideBLeadIn;
    }

    qWarning() << "Unknown vinyl type " << vinyl_type;
    return 0;
}

// Update the config object with parameters from dialog
void DlgPrefVinyl::slotApply() {
    slotVinylGainApply();

    // Save vinyl types, speed and lead-in, activate signal wdgets
    for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
        QString group = PlayerManager::groupForDeck(i);

        config->set(ConfigKey(group, "vinylcontrol_vinyl_type"),
                ConfigValue(m_vcTypeBoxes[i]->currentText()));
        config->set(ConfigKey(group, "vinylcontrol_speed_type"),
                ConfigValue(m_vcSpeedBoxes[i]->currentText()));
        config->set(ConfigKey(group, "vinylcontrol_pitch_estimator_type"),
                ConfigValue(m_vcPitchEstimatorBoxes[i]->currentText()));
        verifyAndSaveLeadInTime(i, group);

        m_signalWidgets[i]->setVinylActive(m_pVCManager->vinylInputConnected(i));
    }

    // Save the vinylcontrol_speed_type in ControlObjects as well so it can be
    // retrieved quickly on the fly. (eg. WSpinny needs to know how fast to spin)
    for (auto* co : std::as_const(m_COSpeeds)) {
        int i = m_COSpeeds.indexOf(co);
        double speed = m_vcSpeedBoxes[i]->currentText() == kVinylSpeed33
                ? kVinylSpeed33Num
                : kVinylSpeed45Num;
        co->set(speed);
    }

    config->set(ConfigKey(kVinylPrefKey, "show_signal_quality"),
            ConfigValue((int)(SignalQualityEnable->isChecked())));

    m_pVCManager->requestReloadConfig();
    slotUpdate();
}

void DlgPrefVinyl::slotVinylGainApply() {
    const int dBGain = SliderVinylGain->value();
    qDebug() << "in VinylGainSlotApply()" << "with gain:" << dBGain << "dB";
    // Update the config key...
    const double ratioGain = db2ratio(static_cast<double>(dBGain));
    config->set(ConfigKey(kVinylPrefKey, "gain"), ConfigValue(QString::number(ratioGain)));

    // Update the ControlObject...
    ControlObject::set(ConfigKey(kVinylPrefKey, "gain"), ratioGain);
}

void DlgPrefVinyl::slotUpdateVinylGain() {
    int value = SliderVinylGain->value();
    textLabelPreampCurrent->setText(
            QString("%1 dB").arg(value));
}

QUrl DlgPrefVinyl::helpUrl() const {
    return QUrl(MIXXX_MANUAL_VINYL_URL);
}

void DlgPrefVinyl::setDeckWidgetsVisible(int deck, bool visible) {
    if (deck < 0 || deck > 3) {
        qWarning() << "Tried to set a vinyl preference widget visible that doesn't exist: " << deck;
    }

    if (visible) {
        m_vcLabels[deck]->show();
        m_vcTypeBoxes[deck]->show();
        m_vcSpeedBoxes[deck]->show();
        m_vcPitchEstimatorBoxes[deck]->show();
        m_vcLeadInBoxes[deck]->show();
        m_signalWidgets[deck]->show();
    } else {
        m_vcLabels[deck]->hide();
        m_vcTypeBoxes[deck]->hide();
        m_vcSpeedBoxes[deck]->hide();
        m_vcPitchEstimatorBoxes[deck]->hide();
        m_vcLeadInBoxes[deck]->hide();
        m_signalWidgets[deck]->hide();
    }
}
