#include "preferences/dialog/dlgprefreplaygain.h"

#include "controlobject.h"
#include "util/math.h"

#define kConfigKey "[ReplayGain]"

static const int kReplayGainReferenceLUFS = -18;


DlgPrefReplayGain::DlgPrefReplayGain(QWidget * parent, UserSettingsPointer  _config)
        : DlgPreferencePage(parent),
          m_pConfig(_config),
          m_replayGainBoost(kConfigKey, "ReplayGainBoost"),
          m_defaultBoost(kConfigKey, "DefaultBoost"),
          m_enabled(kConfigKey, "ReplayGainEnabled") {
    setupUi(this);

    m_analysisButtonGroup.addButton(radioButtonRG1);
    m_analysisButtonGroup.addButton(radioButtonRG2);
    m_analysisButtonGroup.addButton(radioButtonDisable);

    connect(EnableGain, SIGNAL(stateChanged(int)),
            this, SLOT(slotSetRGEnabled()));
    connect(&m_analysisButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotSetRGAnalyzerChanged()));
    connect(SliderReplayGainBoost, SIGNAL(valueChanged(int)),
            this, SLOT(slotUpdateReplayGainBoost()));
    connect(SliderReplayGainBoost, SIGNAL(sliderReleased()),
            this, SLOT(slotApply()));
    connect(SliderDefaultBoost, SIGNAL(valueChanged(int)),
            this, SLOT(slotUpdateDefaultBoost()));
    connect(SliderDefaultBoost, SIGNAL(sliderReleased()),
            this, SLOT(slotApply()));
    connect(checkBoxReanalyze, SIGNAL(stateChanged(int)),
            this, SLOT(slotSetReanalyze()));

    loadSettings();
}

DlgPrefReplayGain::~DlgPrefReplayGain() {
}

void DlgPrefReplayGain::loadSettings() {
    int iReplayGainBoost = m_pConfig->getValueString(
            ConfigKey(kConfigKey, "InitialReplayGainBoost"), "0").toInt();
    SliderReplayGainBoost->setValue(iReplayGainBoost);
    setLabelCurrentReplayGainBoost(iReplayGainBoost);


    int iDefaultBoost = m_pConfig->getValueString(
            ConfigKey(kConfigKey, "InitialDefaultBoost"), "-6").toInt();
    SliderDefaultBoost->setValue(iDefaultBoost);
    LabelCurrentDefaultBoost->setText(
            QString("%1 dB").arg(iDefaultBoost));

    bool gainEnabled = m_pConfig->getValueString(
            ConfigKey(kConfigKey, "ReplayGainEnabled"), "1").toInt() == 1;
    EnableGain->setChecked(gainEnabled);

    // WARNING: Do not fix the "analyser" spelling here since user config files
    // contain these strings.
    bool analyzerEnabled = m_pConfig->getValueString(
            ConfigKey(kConfigKey, "ReplayGainAnalyserEnabled"), "1").toInt();

    int version = m_pConfig->getValueString(
            ConfigKey("[ReplayGain]", "ReplayGainVersion")).toInt();

    if (!analyzerEnabled) {
        radioButtonDisable->setChecked(true);
    } else if (version == 1) {
        radioButtonRG1->setChecked(true);
    } else {
        radioButtonRG2->setChecked(true);
    }

    checkBoxReanalyze->setEnabled(analyzerEnabled);

    bool reanalyse = m_pConfig->getValueString(
            ConfigKey("[ReplayGain]", "ReplayGainReanalyze")).toInt();
    checkBoxReanalyze->setChecked(reanalyse);

    slotUpdate();
    slotUpdateReplayGainBoost();
    slotUpdateDefaultBoost();
}

void DlgPrefReplayGain::slotResetToDefaults() {
    EnableGain->setChecked(true);
    // Turn ReplayGain Analyzer on by default as it does not give appreciable
    // delay on recent hardware (<5 years old).
    radioButtonRG2->setChecked(true);
    checkBoxReanalyze->setChecked(false);
    checkBoxReanalyze->setEnabled(true);
    SliderReplayGainBoost->setValue(0);
    setLabelCurrentReplayGainBoost(0);
    SliderDefaultBoost->setValue(-6);
    LabelCurrentDefaultBoost->setText("-6 dB");

    int iDefaultBoost = m_pConfig->getValueString(
            ConfigKey(kConfigKey, "InitialDefaultBoost"), "-6").toInt();
    SliderDefaultBoost->setValue(iDefaultBoost);
    LabelCurrentDefaultBoost->setText(
            QString("%1 dB").arg(iDefaultBoost));


    slotUpdate();
    slotApply();
}

void DlgPrefReplayGain::slotSetRGEnabled() {
    if (EnableGain->isChecked()) {
        m_pConfig->set(ConfigKey(kConfigKey,"ReplayGainEnabled"), ConfigValue(1));
    } else {
        m_pConfig->set(ConfigKey(kConfigKey,"ReplayGainEnabled"), ConfigValue(0));
    }
    slotUpdate();
    slotApply();
}

void DlgPrefReplayGain::slotSetRGAnalyzerChanged() {
    bool rg1 = radioButtonRG1->isChecked();
    bool disabled = radioButtonDisable->isChecked();

    // WARNING: Do not fix the "analyser" spelling here since user config files
    // contain these strings.
    m_pConfig->set(ConfigKey(kConfigKey,"ReplayGainAnalyserEnabled"),
                ConfigValue(!disabled));
    checkBoxReanalyze->setEnabled(!disabled);
    m_pConfig->set(ConfigKey(kConfigKey,"ReplayGainVersion"),
                ConfigValue(rg1 ? 1 : 2));

    slotApply();
}

void DlgPrefReplayGain::slotUpdateReplayGainBoost() {
    int value = SliderReplayGainBoost->value();
    m_pConfig->set(ConfigKey(kConfigKey, "InitialReplayGainBoost"),
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
    m_pConfig->set(ConfigKey(kConfigKey, "InitialDefaultBoost"),
                ConfigValue(value));
    LabelCurrentDefaultBoost->setText(
            QString("%1 dB").arg(value));
    slotApply();
}

void DlgPrefReplayGain::slotUpdate() {
    if (m_pConfig->getValueString(
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

void DlgPrefReplayGain::slotSetReanalyze() {
    bool checked = checkBoxReanalyze->isChecked();
    m_pConfig->set(ConfigKey(kConfigKey,"ReplayGainReanalyze"),
                ConfigValue(checked));
    slotApply();
}

