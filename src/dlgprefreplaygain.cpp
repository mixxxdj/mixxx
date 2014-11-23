#include "dlgprefreplaygain.h"

#include "controlobject.h"
#include "util/math.h"

#define kConfigKey "[ReplayGain]"


DlgPrefReplayGain::DlgPrefReplayGain(QWidget * parent, ConfigObject<ConfigValue> * _config)
        :  DlgPreferencePage(parent),
           config(_config),
           m_COTReplayGainBoost(kConfigKey, "ReplayGainBoost"),
           m_COTEnabled(kConfigKey, "ReplayGainEnabled") {
    setupUi(this);

    //Connections
    connect(EnableGain,      SIGNAL(stateChanged(int)), this, SLOT(slotSetRGEnabled()));
    connect(EnableAnalyser,  SIGNAL(stateChanged(int)), this, SLOT(slotSetRGAnalyserEnabled()));
    connect(SliderBoost,     SIGNAL(valueChanged(int)), this, SLOT(slotUpdateBoost()));
    connect(SliderBoost,     SIGNAL(sliderReleased()),  this, SLOT(slotApply()));

    loadSettings();
}

DlgPrefReplayGain::~DlgPrefReplayGain() {
}

void DlgPrefReplayGain::loadSettings() {
    if(config->getValueString(ConfigKey(kConfigKey,"ReplayGainEnabled"))==QString("")) {
        slotResetToDefaults();
    } else {
        int iReplayGainBoost =
                config->getValueString(ConfigKey(kConfigKey, "InitialReplayGainBoost")).toInt();
        SliderBoost->setValue(iReplayGainBoost);
        lcddB->display(iReplayGainBoost);

        bool gainEnabled =
                config->getValueString(ConfigKey(kConfigKey, "ReplayGainEnabled")).toInt() == 1;
        EnableGain->setChecked(gainEnabled);

        bool analyserEnabled =
                config->getValueString(ConfigKey(kConfigKey, "ReplayGainAnalyserEnabled")).toInt();
        EnableAnalyser->setChecked(analyserEnabled);
    }
    slotUpdate();
    slotUpdateBoost();
}

void DlgPrefReplayGain::slotResetToDefaults() {
    EnableGain->setChecked(true);
    // Turn ReplayGain Analyser on by default as it does not give appreciable
    // delay on recent hardware (<5 years old).
    EnableAnalyser->setChecked(true);
    SliderBoost->setValue(6);
    lcddB->display(6);
    slotUpdate();
    slotApply();
}

void DlgPrefReplayGain::slotSetRGEnabled() {
    if (EnableGain->isChecked()) {
        config->set(ConfigKey(kConfigKey,"ReplayGainEnabled"), ConfigValue(1));
    } else {
        config->set(ConfigKey(kConfigKey,"ReplayGainEnabled"), ConfigValue(0));
        config->set(ConfigKey(kConfigKey,"ReplayGainAnalyserEnabled"),
                    ConfigValue(0));
    }

    slotUpdate();
    slotApply();
}

void DlgPrefReplayGain::slotSetRGAnalyserEnabled() {
    int enabled = EnableAnalyser->isChecked() ? 1 : 0;
    config->set(ConfigKey(kConfigKey,"ReplayGainAnalyserEnabled"),
                ConfigValue(enabled));
    slotApply();
}

void DlgPrefReplayGain::slotUpdateBoost() {
    config->set(ConfigKey(kConfigKey, "InitialReplayGainBoost"),
                ConfigValue(SliderBoost->value()));
    slotApply();
}

void DlgPrefReplayGain::slotUpdate() {
    if (config->getValueString(
            ConfigKey(kConfigKey,"ReplayGainEnabled")).toInt()==1) {
        EnableAnalyser->setEnabled(true);
        SliderBoost->setEnabled(true);
    } else {
        EnableAnalyser->setChecked(false);
        EnableAnalyser->setEnabled(false);
        SliderBoost->setEnabled(false);
    }
}

void DlgPrefReplayGain::slotApply() {
    double replayGainBoostDb = SliderBoost->value();
    m_COTReplayGainBoost.set(db2ratio(replayGainBoostDb));
    int iRGenabled = 0;
    if (EnableGain->isChecked()) {
        iRGenabled = 1;
    }
    m_COTEnabled.set(iRGenabled);
}
