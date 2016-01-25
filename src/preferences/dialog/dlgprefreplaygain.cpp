#include "preferences/dialog/dlgprefreplaygain.h"

#include "controlobject.h"
#include "util/math.h"

#define kConfigKey "[ReplayGain]"

static const int kReplayGainReferenceLUFS = -18;


DlgPrefReplayGain::DlgPrefReplayGain(QWidget * parent, UserSettingsPointer  _config)
        : DlgPreferencePage(parent),
          config(_config),
          m_replayGainBoost(kConfigKey, "ReplayGainBoost"),
          m_defaultBoost(kConfigKey, "DefaultBoost"),
          m_enabled(kConfigKey, "ReplayGainEnabled") {
    setupUi(this);

    //Connections
    connect(EnableGain, SIGNAL(stateChanged(int)),
            this, SLOT(slotSetRGEnabled()));
    connect(EnableAnalyzer, SIGNAL(stateChanged(int)),
            this, SLOT(slotSetRGAnalyzerEnabled()));
    connect(SliderReplayGainBoost, SIGNAL(valueChanged(int)),
            this, SLOT(slotUpdateReplayGainBoost()));
    connect(SliderReplayGainBoost, SIGNAL(sliderReleased()),
            this, SLOT(slotApply()));
    connect(SliderDefaultBoost, SIGNAL(valueChanged(int)),
            this, SLOT(slotUpdateDefaultBoost()));
    connect(SliderDefaultBoost, SIGNAL(sliderReleased()),
            this, SLOT(slotApply()));

    loadSettings();
}

DlgPrefReplayGain::~DlgPrefReplayGain() {
}

void DlgPrefReplayGain::loadSettings() {
    int iReplayGainBoost = config->getValueString(
            ConfigKey(kConfigKey, "InitialReplayGainBoost"), "0").toInt();
    SliderReplayGainBoost->setValue(iReplayGainBoost);
    setLabelCurrentReplayGainBoost(iReplayGainBoost);


    int iDefaultBoost = config->getValueString(
            ConfigKey(kConfigKey, "InitialDefaultBoost"), "-6").toInt();
    SliderDefaultBoost->setValue(iDefaultBoost);
    LabelCurrentDefaultBoost->setText(
            QString("%1 dB").arg(iDefaultBoost));

    bool gainEnabled = config->getValueString(
            ConfigKey(kConfigKey, "ReplayGainEnabled"), "1").toInt() == 1;
    EnableGain->setChecked(gainEnabled);

    // WARNING: Do not fix the "analyser" spelling here since user config files
    // contain these strings.
    bool analyzerEnabled = config->getValueString(
            ConfigKey(kConfigKey, "ReplayGainAnalyserEnabled"), "1").toInt();
    EnableAnalyzer->setChecked(analyzerEnabled);

    slotUpdate();
    slotUpdateReplayGainBoost();
    slotUpdateDefaultBoost();
}

void DlgPrefReplayGain::slotResetToDefaults() {
    EnableGain->setChecked(true);
    // Turn ReplayGain Analyzer on by default as it does not give appreciable
    // delay on recent hardware (<5 years old).
    EnableAnalyzer->setChecked(true);
    SliderReplayGainBoost->setValue(0);
    setLabelCurrentReplayGainBoost(0);
    SliderDefaultBoost->setValue(-6);
    LabelCurrentDefaultBoost->setText("-6 dB");

    int iDefaultBoost = config->getValueString(
            ConfigKey(kConfigKey, "InitialDefaultBoost"), "-6").toInt();
    SliderDefaultBoost->setValue(iDefaultBoost);
    LabelCurrentDefaultBoost->setText(
            QString("%1 dB").arg(iDefaultBoost));


    slotUpdate();
    slotApply();
}

void DlgPrefReplayGain::slotSetRGEnabled() {
    if (EnableGain->isChecked()) {
        config->set(ConfigKey(kConfigKey,"ReplayGainEnabled"), ConfigValue(1));
    } else {
        config->set(ConfigKey(kConfigKey,"ReplayGainEnabled"), ConfigValue(0));
    }
    slotUpdate();
    slotApply();
}

void DlgPrefReplayGain::slotSetRGAnalyzerEnabled() {
    // WARNING: Do not fix the "analyser" spelling here since user config files
    // contain these strings.
    int enabled = EnableAnalyzer->isChecked() ? 1 : 0;
    config->set(ConfigKey(kConfigKey,"ReplayGainAnalyserEnabled"),
                ConfigValue(enabled));
    slotApply();
}

void DlgPrefReplayGain::slotUpdateReplayGainBoost() {
    int value = SliderReplayGainBoost->value();
    config->set(ConfigKey(kConfigKey, "InitialReplayGainBoost"),
                ConfigValue(value));
    setLabelCurrentReplayGainBoost(value);
    slotApply();
}

void DlgPrefReplayGain::setLabelCurrentReplayGainBoost(int value) {
    LabelCurrentReplayGainBoost->setText(
            QString(tr("%1 LUFS (adjust by %2 dB)")).arg(
                  QString::number(value + kReplayGainReferenceLUFS), QString().sprintf("%+d", value)));
}

void DlgPrefReplayGain::slotUpdateDefaultBoost() {
    int value = SliderDefaultBoost->value();
    config->set(ConfigKey(kConfigKey, "InitialDefaultBoost"),
                ConfigValue(value));
    LabelCurrentDefaultBoost->setText(
            QString("%1 dB").arg(value));
    slotApply();
}

void DlgPrefReplayGain::slotUpdate() {
    if (config->getValueString(
            ConfigKey(kConfigKey,"ReplayGainEnabled")).toInt() == 1) {
        SliderReplayGainBoost->setEnabled(true);
        SliderDefaultBoost->setEnabled(true);
    } else {
        SliderReplayGainBoost->setEnabled(false);
        SliderDefaultBoost->setEnabled(false);
    }
}

void DlgPrefReplayGain::slotApply() {
    double replayGainBoostDb = SliderReplayGainBoost->value();
    m_replayGainBoost.set(db2ratio(replayGainBoostDb));
    double defaultBoostDb = SliderDefaultBoost->value();
    m_defaultBoost.set(db2ratio(defaultBoostDb));
    m_enabled.set(EnableGain->isChecked() ? 1.0 : 0.0);
}
