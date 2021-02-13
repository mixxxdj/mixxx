#include "preferences/dialog/dlgprefvinyl.h"

#include <QtDebug>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "defs_urls.h"
#include "mixer/playermanager.h"
#include "moc_dlgprefvinyl.cpp"
#include "util/platform.h"
#include "vinylcontrol/defs_vinylcontrol.h"
#include "vinylcontrol/vinylcontrolmanager.h"

DlgPrefVinyl::DlgPrefVinyl(
        QWidget* parent,
        std::shared_ptr<VinylControlManager> pVCMan,
        UserSettingsPointer _config)
        : DlgPreferencePage(parent),
          m_pVCManager(pVCMan),
          config(_config) {
    m_pNumDecks = new ControlProxy("[Master]", "num_decks", this);
    m_pNumDecks->connectValueChanged(this, &DlgPrefVinyl::slotNumDecksChanged);

    setupUi(this);
    // Create text color for the Troubleshooting link
    createLinkColor();

    delete groupBoxSignalQuality->layout();
    QHBoxLayout *layout = new QHBoxLayout;

    for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
        m_signalWidgets.push_back(new VinylControlSignalWidget());
        VinylControlSignalWidget* widget = m_signalWidgets.back();
        widget->setVinylInput(i);
        widget->setSize(MIXXX_VINYL_SCOPE_SIZE);
        layout->layout()->addWidget(widget);
    }

    groupBoxSignalQuality->setLayout(layout);

    // Add vinyl types
    ComboBoxVinylType1->addItem(MIXXX_VINYL_SERATOCV02VINYLSIDEA);
    ComboBoxVinylType1->addItem(MIXXX_VINYL_SERATOCV02VINYLSIDEB);
    ComboBoxVinylType1->addItem(MIXXX_VINYL_SERATOCD);
    ComboBoxVinylType1->addItem(MIXXX_VINYL_TRAKTORSCRATCHSIDEA);
    ComboBoxVinylType1->addItem(MIXXX_VINYL_TRAKTORSCRATCHSIDEB);
    ComboBoxVinylType1->addItem(MIXXX_VINYL_MIXVIBESDVS);

    ComboBoxVinylType2->addItem(MIXXX_VINYL_SERATOCV02VINYLSIDEA);
    ComboBoxVinylType2->addItem(MIXXX_VINYL_SERATOCV02VINYLSIDEB);
    ComboBoxVinylType2->addItem(MIXXX_VINYL_SERATOCD);
    ComboBoxVinylType2->addItem(MIXXX_VINYL_TRAKTORSCRATCHSIDEA);
    ComboBoxVinylType2->addItem(MIXXX_VINYL_TRAKTORSCRATCHSIDEB);
    ComboBoxVinylType2->addItem(MIXXX_VINYL_MIXVIBESDVS);

    ComboBoxVinylType3->addItem(MIXXX_VINYL_SERATOCV02VINYLSIDEA);
    ComboBoxVinylType3->addItem(MIXXX_VINYL_SERATOCV02VINYLSIDEB);
    ComboBoxVinylType3->addItem(MIXXX_VINYL_SERATOCD);
    ComboBoxVinylType3->addItem(MIXXX_VINYL_TRAKTORSCRATCHSIDEA);
    ComboBoxVinylType3->addItem(MIXXX_VINYL_TRAKTORSCRATCHSIDEB);
    ComboBoxVinylType3->addItem(MIXXX_VINYL_MIXVIBESDVS);

    ComboBoxVinylType4->addItem(MIXXX_VINYL_SERATOCV02VINYLSIDEA);
    ComboBoxVinylType4->addItem(MIXXX_VINYL_SERATOCV02VINYLSIDEB);
    ComboBoxVinylType4->addItem(MIXXX_VINYL_SERATOCD);
    ComboBoxVinylType4->addItem(MIXXX_VINYL_TRAKTORSCRATCHSIDEA);
    ComboBoxVinylType4->addItem(MIXXX_VINYL_TRAKTORSCRATCHSIDEB);
    ComboBoxVinylType4->addItem(MIXXX_VINYL_MIXVIBESDVS);

    ComboBoxVinylSpeed1->addItem(MIXXX_VINYL_SPEED_33);
    ComboBoxVinylSpeed1->addItem(MIXXX_VINYL_SPEED_45);
    ComboBoxVinylSpeed2->addItem(MIXXX_VINYL_SPEED_33);
    ComboBoxVinylSpeed2->addItem(MIXXX_VINYL_SPEED_45);
    ComboBoxVinylSpeed3->addItem(MIXXX_VINYL_SPEED_33);
    ComboBoxVinylSpeed3->addItem(MIXXX_VINYL_SPEED_45);
    ComboBoxVinylSpeed4->addItem(MIXXX_VINYL_SPEED_33);
    ComboBoxVinylSpeed4->addItem(MIXXX_VINYL_SPEED_45);

    LeadinTime1->setSuffix(" s");
    LeadinTime2->setSuffix(" s");
    LeadinTime3->setSuffix(" s");
    LeadinTime4->setSuffix(" s");

    TroubleshootingLink->setText(coloredLinkString(
            m_pLinkColor,
            QStringLiteral("Troubleshooting"),
            MIXXX_MANUAL_VINYL_TROUBLESHOOTING_URL));

    connect(VinylGain, &QSlider::sliderReleased, this, &DlgPrefVinyl::slotVinylGainApply);
    connect(VinylGain,
            QOverload<int>::of(&QSlider::valueChanged),
            this,
            &DlgPrefVinyl::slotUpdateVinylGain);

    // No real point making this a mapper since the combos aren't indexed.
    connect(ComboBoxVinylType1,
            &QComboBox::currentTextChanged,
            this,
            &DlgPrefVinyl::slotVinylType1Changed);
    connect(ComboBoxVinylType2,
            &QComboBox::currentTextChanged,
            this,
            &DlgPrefVinyl::slotVinylType2Changed);
    connect(ComboBoxVinylType3,
            &QComboBox::currentTextChanged,
            this,
            &DlgPrefVinyl::slotVinylType3Changed);
    connect(ComboBoxVinylType4,
            &QComboBox::currentTextChanged,
            this,
            &DlgPrefVinyl::slotVinylType4Changed);

    for (int i = 0; i < kMaxNumberOfDecks; ++i) {
        setDeckWidgetsVisible(i, false);
    }

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
        m_COSpeeds.push_back(new ControlProxy(group, "vinylcontrol_speed_type"));
        setDeckWidgetsVisible(i, true);
    }
}

void DlgPrefVinyl::slotVinylType1Changed(const QString& text) {
    LeadinTime1->setValue(getDefaultLeadIn(text));
}

void DlgPrefVinyl::slotVinylType2Changed(const QString& text) {
    LeadinTime2->setValue(getDefaultLeadIn(text));
}

void DlgPrefVinyl::slotVinylType3Changed(const QString& text) {
    LeadinTime3->setValue(getDefaultLeadIn(text));
}

void DlgPrefVinyl::slotVinylType4Changed(const QString& text) {
    LeadinTime4->setValue(getDefaultLeadIn(text));
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
    ComboBoxVinylType1->setCurrentIndex(0);
    ComboBoxVinylType2->setCurrentIndex(0);
    ComboBoxVinylType3->setCurrentIndex(0);
    ComboBoxVinylType4->setCurrentIndex(0);
    // Default to 33 RPM.
    ComboBoxVinylSpeed1->setCurrentIndex(0);
    ComboBoxVinylSpeed2->setCurrentIndex(0);
    ComboBoxVinylSpeed3->setCurrentIndex(0);
    ComboBoxVinylSpeed4->setCurrentIndex(0);

    LeadinTime1->setValue(MIXXX_VINYL_SERATOCV02VINYLSIDEA_LEADIN);
    LeadinTime2->setValue(MIXXX_VINYL_SERATOCV02VINYLSIDEA_LEADIN);
    LeadinTime3->setValue(MIXXX_VINYL_SERATOCV02VINYLSIDEA_LEADIN);
    LeadinTime4->setValue(MIXXX_VINYL_SERATOCV02VINYLSIDEA_LEADIN);
    SignalQualityEnable->setChecked(true);
    VinylGain->setValue(0);
    slotUpdateVinylGain();
}

void DlgPrefVinyl::slotUpdate() {
    // Set vinyl control types in the comboboxes
    int combo_index =
            ComboBoxVinylType1->findText(config->getValueString(
                    ConfigKey("[Channel1]", "vinylcontrol_vinyl_type")));
    if (combo_index != -1) {
        ComboBoxVinylType1->setCurrentIndex(combo_index);
    }

    combo_index =
            ComboBoxVinylType2->findText(config->getValueString(
                    ConfigKey("[Channel2]", "vinylcontrol_vinyl_type")));
    if (combo_index != -1) {
        ComboBoxVinylType2->setCurrentIndex(combo_index);
    }

    combo_index =
            ComboBoxVinylType3->findText(config->getValueString(
                    ConfigKey("[Channel3]", "vinylcontrol_vinyl_type")));
    if (combo_index != -1) {
        ComboBoxVinylType3->setCurrentIndex(combo_index);
    }

    combo_index =
            ComboBoxVinylType4->findText(config->getValueString(
                    ConfigKey("[Channel4]", "vinylcontrol_vinyl_type")));
    if (combo_index != -1) {
        ComboBoxVinylType4->setCurrentIndex(combo_index);
    }

    combo_index =
            ComboBoxVinylSpeed1->findText(config->getValueString(
                    ConfigKey("[Channel1]", "vinylcontrol_speed_type")));
    if (combo_index != -1) {
        ComboBoxVinylSpeed1->setCurrentIndex(combo_index);
    }

    combo_index =
            ComboBoxVinylSpeed2->findText(config->getValueString(
                    ConfigKey("[Channel2]", "vinylcontrol_speed_type")));
    if (combo_index != -1) {
        ComboBoxVinylSpeed2->setCurrentIndex(combo_index);
    }

    combo_index =
            ComboBoxVinylSpeed3->findText(config->getValueString(
                    ConfigKey("[Channel3]", "vinylcontrol_speed_type")));
    if (combo_index != -1) {
        ComboBoxVinylSpeed3->setCurrentIndex(combo_index);
    }

    combo_index =
            ComboBoxVinylSpeed4->findText(config->getValueString(
                    ConfigKey("[Channel4]", "vinylcontrol_speed_type")));
    if (combo_index != -1) {
        ComboBoxVinylSpeed4->setCurrentIndex(combo_index);
    }

    // set lead-in time
    LeadinTime1->setValue((config->getValue(ConfigKey("[Channel1]", "vinylcontrol_lead_in_time"), "0")).toInt());
    LeadinTime2->setValue((config->getValue(ConfigKey("[Channel2]", "vinylcontrol_lead_in_time"), "0")).toInt());
    LeadinTime3->setValue((config->getValue(ConfigKey("[Channel3]", "vinylcontrol_lead_in_time"), "0")).toInt());
    LeadinTime4->setValue((config->getValue(ConfigKey("[Channel4]", "vinylcontrol_lead_in_time"), "0")).toInt());

    SignalQualityEnable->setChecked(
            (bool)config->getValue<bool>(ConfigKey(VINYL_PREF_KEY, "show_signal_quality")));

    // set vinyl control gain
    const double ratioGain = config->getValue<double>(ConfigKey(VINYL_PREF_KEY, "gain"));
    const double dbGain = ratio2db(ratioGain);
    VinylGain->setValue(static_cast<int>(dbGain + 0.5));
    slotUpdateVinylGain();

    for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
        m_signalWidgets[i]->setVinylActive(m_pVCManager->vinylInputConnected(i));
    }
}

void DlgPrefVinyl::verifyAndSaveLeadInTime(
        QSpinBox* widget, const QString& group, const QString& vinyl_type) {
    QString strLeadIn = widget->text();
    bool isInteger;
    strLeadIn.toInt(&isInteger);
    if (isInteger) {
        config->set(ConfigKey(group, "vinylcontrol_lead_in_time"), strLeadIn);
    } else {
        config->setValue(ConfigKey(group, "vinylcontrol_lead_in_time"),
                         getDefaultLeadIn(vinyl_type));
    }
}

int DlgPrefVinyl::getDefaultLeadIn(const QString& vinyl_type) const {
    if (vinyl_type == MIXXX_VINYL_SERATOCV02VINYLSIDEA) {
        return MIXXX_VINYL_SERATOCV02VINYLSIDEA_LEADIN;
    } else if (vinyl_type == MIXXX_VINYL_SERATOCV02VINYLSIDEB) {
        return MIXXX_VINYL_SERATOCV02VINYLSIDEB_LEADIN;
    } else if (vinyl_type == MIXXX_VINYL_SERATOCD) {
        return MIXXX_VINYL_SERATOCD_LEADIN;
    } else if (vinyl_type == MIXXX_VINYL_TRAKTORSCRATCHSIDEA) {
        return MIXXX_VINYL_TRAKTORSCRATCHSIDEA_LEADIN;
    } else if (vinyl_type == MIXXX_VINYL_TRAKTORSCRATCHSIDEB) {
        return MIXXX_VINYL_TRAKTORSCRATCHSIDEB_LEADIN;
    } else if (vinyl_type == MIXXX_VINYL_MIXVIBESDVS) {
        return MIXXX_VINYL_MIXVIBESDVS_LEADIN;
    }
    qWarning() << "Unknown vinyl type " << vinyl_type;
    return 0;
}

// Update the config object with parameters from dialog
void DlgPrefVinyl::slotApply()
{
    qDebug() << "DlgPrefVinyl::Apply";

    verifyAndSaveLeadInTime(LeadinTime1, "[Channel1]", ComboBoxVinylType1->currentText());
    verifyAndSaveLeadInTime(LeadinTime2, "[Channel2]", ComboBoxVinylType2->currentText());
    verifyAndSaveLeadInTime(LeadinTime3, "[Channel3]", ComboBoxVinylType3->currentText());
    verifyAndSaveLeadInTime(LeadinTime4, "[Channel4]", ComboBoxVinylType4->currentText());

    // Apply updates for everything else...
    VinylTypeSlotApply();
    slotVinylGainApply();

    config->set(ConfigKey(VINYL_PREF_KEY,"show_signal_quality"),
                ConfigValue((int)(SignalQualityEnable->isChecked())));

    m_pVCManager->requestReloadConfig();
    slotUpdate();
}

void DlgPrefVinyl::VinylTypeSlotApply()
{
    config->set(ConfigKey("[Channel1]","vinylcontrol_vinyl_type"),
                ConfigValue(ComboBoxVinylType1->currentText()));
    config->set(ConfigKey("[Channel2]","vinylcontrol_vinyl_type"),
                ConfigValue(ComboBoxVinylType2->currentText()));
    config->set(ConfigKey("[Channel3]","vinylcontrol_vinyl_type"),
                ConfigValue(ComboBoxVinylType3->currentText()));
    config->set(ConfigKey("[Channel4]","vinylcontrol_vinyl_type"),
                ConfigValue(ComboBoxVinylType4->currentText()));
    config->set(ConfigKey("[Channel1]","vinylcontrol_speed_type"),
                ConfigValue(ComboBoxVinylSpeed1->currentText()));
    config->set(ConfigKey("[Channel2]","vinylcontrol_speed_type"),
                ConfigValue(ComboBoxVinylSpeed2->currentText()));
    config->set(ConfigKey("[Channel3]","vinylcontrol_speed_type"),
                ConfigValue(ComboBoxVinylSpeed3->currentText()));
    config->set(ConfigKey("[Channel4]","vinylcontrol_speed_type"),
                ConfigValue(ComboBoxVinylSpeed4->currentText()));

    // Save the vinylcontrol_speed_type in ControlObjects as well so it can be retrieved quickly
    // on the fly. (eg. WSpinny needs to know how fast to spin)

    switch (m_COSpeeds.length()) {
    case 4:
        if (ComboBoxVinylSpeed4->currentText() == MIXXX_VINYL_SPEED_33) {
            m_COSpeeds[3]->set(MIXXX_VINYL_SPEED_33_NUM);
        } else if (ComboBoxVinylSpeed4->currentText() == MIXXX_VINYL_SPEED_45) {
            m_COSpeeds[3]->set(MIXXX_VINYL_SPEED_45_NUM);
        }
        M_FALLTHROUGH_INTENDED;
    case 3:
        if (ComboBoxVinylSpeed3->currentText() == MIXXX_VINYL_SPEED_33) {
            m_COSpeeds[2]->slotSet(MIXXX_VINYL_SPEED_33_NUM);
        } else if (ComboBoxVinylSpeed3->currentText() == MIXXX_VINYL_SPEED_45) {
            m_COSpeeds[2]->slotSet(MIXXX_VINYL_SPEED_45_NUM);
        }
        M_FALLTHROUGH_INTENDED;
    case 2:
        if (ComboBoxVinylSpeed2->currentText() == MIXXX_VINYL_SPEED_33) {
            m_COSpeeds[1]->slotSet(MIXXX_VINYL_SPEED_33_NUM);
        } else if (ComboBoxVinylSpeed2->currentText() == MIXXX_VINYL_SPEED_45) {
            m_COSpeeds[1]->slotSet(MIXXX_VINYL_SPEED_45_NUM);
        }
        M_FALLTHROUGH_INTENDED;
    case 1:
        if (ComboBoxVinylSpeed1->currentText() == MIXXX_VINYL_SPEED_33) {
            m_COSpeeds[0]->slotSet(MIXXX_VINYL_SPEED_33_NUM);
        } else if (ComboBoxVinylSpeed1->currentText() == MIXXX_VINYL_SPEED_45) {
            m_COSpeeds[0]->slotSet(MIXXX_VINYL_SPEED_45_NUM);
        }
        break;
    default:
        qWarning() << "Unexpected number of vinyl speed preference items";
    }
}

void DlgPrefVinyl::slotVinylGainApply() {
    const int dBGain = VinylGain->value();
    qDebug() << "in VinylGainSlotApply()" << "with gain:" << dBGain << "dB";
    // Update the config key...
    const double ratioGain = db2ratio(static_cast<double>(dBGain));
    config->set(ConfigKey(VINYL_PREF_KEY, "gain"), ConfigValue(QString::number(ratioGain)));

    // Update the ControlObject...
    ControlObject::set(ConfigKey(VINYL_PREF_KEY, "gain"), ratioGain);
}

void DlgPrefVinyl::slotUpdateVinylGain() {
    int value = VinylGain->value();
    textLabelPreampCurrent->setText(
            QString("%1 dB").arg(value));
}

QUrl DlgPrefVinyl::helpUrl() const {
    return QUrl(MIXXX_MANUAL_VINYL_URL);
}

void DlgPrefVinyl::setDeckWidgetsVisible(int deck, bool visible) {
    switch(deck) {
    case 0:
        setDeck1WidgetsVisible(visible);
        break;
    case 1:
        setDeck2WidgetsVisible(visible);
        break;
    case 2:
        setDeck3WidgetsVisible(visible);
        break;
    case 3:
        setDeck4WidgetsVisible(visible);
        break;
    default:
        qWarning() << "Tried to set a vinyl preference widget visible that doesn't exist: " << deck;
    }
}

void DlgPrefVinyl::setDeck1WidgetsVisible(bool visible) {
    if (visible) {
        VinylLabel1->show();
        ComboBoxVinylType1->show();
        ComboBoxVinylSpeed1->show();
        if (m_signalWidgets.length() > 0) {
            m_signalWidgets[0]->show();
        }
        LeadinTime1->show();
    } else {
        VinylLabel1->hide();
        ComboBoxVinylType1->hide();
        ComboBoxVinylSpeed1->hide();
        if (m_signalWidgets.length() > 0) {
            m_signalWidgets[0]->hide();
        }
        LeadinTime1->hide();
    }
}

void DlgPrefVinyl::setDeck2WidgetsVisible(bool visible) {
    if (visible) {
        VinylLabel2->show();
        ComboBoxVinylType2->show();
        ComboBoxVinylSpeed2->show();
        if (m_signalWidgets.length() > 1) {
            m_signalWidgets[1]->show();
        }
        LeadinTime2->show();
    } else {
        VinylLabel2->hide();
        ComboBoxVinylType2->hide();
        ComboBoxVinylSpeed2->hide();
        if (m_signalWidgets.length() > 1) {
            m_signalWidgets[1]->hide();
        }
        LeadinTime2->hide();
    }
}

void DlgPrefVinyl::setDeck3WidgetsVisible(bool visible) {
    if (visible) {
        VinylLabel3->show();
        ComboBoxVinylType3->show();
        ComboBoxVinylSpeed3->show();
        if (m_signalWidgets.length() > 2) {
            m_signalWidgets[2]->show();
        }
        LeadinTime3->show();
    } else {
        VinylLabel3->hide();
        ComboBoxVinylType3->hide();
        ComboBoxVinylSpeed3->hide();
        if (m_signalWidgets.length() > 2) {
            m_signalWidgets[2]->hide();
        }
        LeadinTime3->hide();
    }
}

void DlgPrefVinyl::setDeck4WidgetsVisible(bool visible) {
    if (visible) {
        VinylLabel4->show();
        ComboBoxVinylType4->show();
        ComboBoxVinylSpeed4->show();
        if (m_signalWidgets.length() > 3) {
            m_signalWidgets[3]->show();
        }
        LeadinTime4->show();
    } else {
        VinylLabel4->hide();
        ComboBoxVinylType4->hide();
        ComboBoxVinylSpeed4->hide();
        if (m_signalWidgets.length() > 3) {
            m_signalWidgets[3]->hide();
        }
        LeadinTime4->hide();
    }
}
