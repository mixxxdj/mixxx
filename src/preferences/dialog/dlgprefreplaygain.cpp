#include "preferences/dialog/dlgprefreplaygain.h"

#include "control/controlobject.h"
#include "moc_dlgprefreplaygain.cpp"
#include "util/math.h"

namespace {
const char* kConfigKey = "[ReplayGain]";
const char* kReplayGainBoost = "ReplayGainBoost";
const char* kDefaultBoost = "DefaultBoost";
const char* kReplayGainEnabled = "ReplayGainEnabled";
const int kReplayGainReferenceLUFS = -18;
} // anonymous namespace

DlgPrefReplayGain::DlgPrefReplayGain(QWidget* parent, UserSettingsPointer pConfig)
        : DlgPreferencePage(parent),
          m_rgSettings(pConfig),
          m_replayGainBoost(kConfigKey, kReplayGainBoost),
          m_defaultBoost(kConfigKey, kDefaultBoost),
          m_enabled(kConfigKey, kReplayGainEnabled) {
    setupUi(this);

    m_analysisButtonGroup.addButton(radioButtonRG1);
    m_analysisButtonGroup.addButton(radioButtonRG2);
    m_analysisButtonGroup.addButton(radioButtonDisable);

    connect(EnableGain, &QCheckBox::stateChanged, this, &DlgPrefReplayGain::slotSetRGEnabled);
    connect(&m_analysisButtonGroup,
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            &QButtonGroup::idClicked,
#else
            QOverload<int>::of(&QButtonGroup::buttonClicked),
#endif
            this,
            &DlgPrefReplayGain::slotSetRGAnalyzerChanged);
    connect(SliderReplayGainBoost,
            &QAbstractSlider::valueChanged,
            this,
            &DlgPrefReplayGain::slotUpdateReplayGainBoost);
    connect(SliderReplayGainBoost,
            &QAbstractSlider::sliderReleased,
            this,
            &DlgPrefReplayGain::slotApply);
    connect(SliderDefaultBoost,
            &QAbstractSlider::valueChanged,
            this,
            &DlgPrefReplayGain::slotUpdateDefaultBoost);
    connect(SliderDefaultBoost,
            &QAbstractSlider::sliderReleased,
            this,
            &DlgPrefReplayGain::slotApply);
    connect(checkBoxReanalyze,
            &QCheckBox::stateChanged,
            this,
            &DlgPrefReplayGain::slotSetReanalyze);

    loadSettings();
}

DlgPrefReplayGain::~DlgPrefReplayGain() {
}

void DlgPrefReplayGain::loadSettings() {
    int iReplayGainBoost = m_rgSettings.getInitialReplayGainBoost();
    SliderReplayGainBoost->setValue(iReplayGainBoost);
    setLabelCurrentReplayGainBoost(iReplayGainBoost);


    int iDefaultBoost = m_rgSettings.getInitialDefaultBoost();
    SliderDefaultBoost->setValue(iDefaultBoost);
    LabelCurrentDefaultBoost->setText(
            QString("%1 dB").arg(iDefaultBoost));

    bool gainEnabled = m_rgSettings.getReplayGainEnabled();
    EnableGain->setChecked(gainEnabled);

    bool analyzerEnabled = m_rgSettings.getReplayGainAnalyzerEnabled();
    int version = m_rgSettings.getReplayGainAnalyzerVersion();

    if (!analyzerEnabled) {
        radioButtonDisable->setChecked(true);
    } else if (version == 1) {
        radioButtonRG1->setChecked(true);
    } else {
        radioButtonRG2->setChecked(true);
    }

    checkBoxReanalyze->setEnabled(analyzerEnabled);

    bool reanalyse = m_rgSettings.getReplayGainReanalyze();
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

    int iDefaultBoost = m_rgSettings.getInitialDefaultBoost();
    SliderDefaultBoost->setValue(iDefaultBoost);
    LabelCurrentDefaultBoost->setText(
            QString("%1 dB").arg(iDefaultBoost));


    slotUpdate();
    slotApply();
}

void DlgPrefReplayGain::slotSetRGEnabled() {
    m_rgSettings.setReplayGainEnabled(EnableGain->isChecked());
    slotUpdate();
    slotApply();
}

bool DlgPrefReplayGain::isReplayGainAnalyzerEnabled() const {
    return !radioButtonDisable->isChecked();
}

int DlgPrefReplayGain::getReplayGainVersion() const {
    if (radioButtonRG1->isChecked()) {
        return 1;
    }
    return 2;
}

void DlgPrefReplayGain::slotSetRGAnalyzerChanged() {
    m_rgSettings.setReplayGainAnalyzerEnabled(isReplayGainAnalyzerEnabled());
    checkBoxReanalyze->setEnabled(isReplayGainAnalyzerEnabled());
    m_rgSettings.setReplayGainAnalyzerVersion(getReplayGainVersion());
    slotApply();
}

void DlgPrefReplayGain::slotUpdateReplayGainBoost() {
    int value = SliderReplayGainBoost->value();
    m_rgSettings.setInitialReplayGainBoost(value);
    setLabelCurrentReplayGainBoost(value);
    slotApply();
}

void DlgPrefReplayGain::setLabelCurrentReplayGainBoost(int value) {
    LabelCurrentReplayGainBoost->setText(
        QString(tr("%1 LUFS (adjust by %2 dB)")).arg(
              QString::number(value + kReplayGainReferenceLUFS),
              (value < 0) ? QString() : (QString("+") + QString::number(value))
        )
    );
}

void DlgPrefReplayGain::slotUpdateDefaultBoost() {
    int value = SliderDefaultBoost->value();
    m_rgSettings.setInitialDefaultBoost(value);
    LabelCurrentDefaultBoost->setText(
            QString("%1 dB").arg(value));
    slotApply();
}

void DlgPrefReplayGain::slotUpdate() {
    if (m_rgSettings.getReplayGainEnabled()) {
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
    m_rgSettings.setReplayGainReanalyze(checked);
    slotApply();
}
