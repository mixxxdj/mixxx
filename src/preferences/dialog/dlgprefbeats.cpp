#include "preferences/dialog/dlgprefbeats.h"

#include "analyzer/analyzerbeats.h"
#include "control/controlobject.h"
#include "defs_urls.h"
#include "moc_dlgprefbeats.cpp"

DlgPrefBeats::DlgPrefBeats(QWidget* parent, UserSettingsPointer pConfig)
        : DlgPreferencePage(parent),
          m_bpmSettings(pConfig),
          m_minBpm(m_bpmSettings.getBpmRangeStartDefault()),
          m_maxBpm(m_bpmSettings.getBpmRangeEndDefault()),
          m_bAnalyzerEnabled(m_bpmSettings.getBpmDetectionEnabledDefault()),
          m_bFixedTempoEnabled(m_bpmSettings.getFixedTempoAssumptionDefault()),
          m_bOffsetEnabled(m_bpmSettings.getFixedTempoOffsetCorrectionDefault()),
          m_bFastAnalysisEnabled(m_bpmSettings.getFastAnalysisDefault()),
          m_bReanalyze(m_bpmSettings.getReanalyzeWhenSettingsChangeDefault()),
          m_bReanalyzeImported(m_bpmSettings.getReanalyzeImportedDefault()) {
    setupUi(this);

    m_availablePlugins = AnalyzerBeats::availablePlugins();
    for (const auto& info : qAsConst(m_availablePlugins)) {
        comboBoxBeatPlugin->addItem(info.name, info.id);
    }

    loadSettings();

    // Connections
    connect(comboBoxBeatPlugin,
            QOverload<int>::of(&QComboBox::activated),
            this,
            &DlgPrefBeats::pluginSelected);
    connect(checkBoxAnalyzerEnabled,
            &QCheckBox::stateChanged,
            this,
            &DlgPrefBeats::analyzerEnabled);
    connect(checkBoxFixedTempo,
            &QCheckBox::stateChanged,
            this,
            &DlgPrefBeats::fixedtempoEnabled);
    connect(checkBoxOffsetCorr,
            &QCheckBox::stateChanged,
            this,
            &DlgPrefBeats::offsetEnabled);
    connect(checkBoxFastAnalysis,
            &QCheckBox::stateChanged,
            this,
            &DlgPrefBeats::fastAnalysisEnabled);
    connect(txtMinBpm,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &DlgPrefBeats::minBpmRangeChanged);
    connect(txtMaxBpm,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &DlgPrefBeats::maxBpmRangeChanged);
    connect(checkBoxReanalyse,
            &QCheckBox::stateChanged,
            this,
            &DlgPrefBeats::slotReanalyzeChanged);
    connect(checkBoxReanalyseImported,
            &QCheckBox::stateChanged,
            this,
            &DlgPrefBeats::slotReanalyzeImportedChanged);
}

DlgPrefBeats::~DlgPrefBeats() {
}

QUrl DlgPrefBeats::helpUrl() const {
    return QUrl(MIXXX_MANUAL_BEATS_URL);
}

void DlgPrefBeats::loadSettings() {
    m_selectedAnalyzerId = m_bpmSettings.getBeatPluginId();
    m_bAnalyzerEnabled = m_bpmSettings.getBpmDetectionEnabled();
    m_bFixedTempoEnabled = m_bpmSettings.getFixedTempoAssumption();
    m_bOffsetEnabled = m_bpmSettings.getFixedTempoOffsetCorrection();
    m_bReanalyze =  m_bpmSettings.getReanalyzeWhenSettingsChange();
    m_bFastAnalysisEnabled = m_bpmSettings.getFastAnalysis();

    // TODO(rryan): Above range enabled is not exposed?
    m_minBpm = m_bpmSettings.getBpmRangeStart();
    m_maxBpm = m_bpmSettings.getBpmRangeEnd();

    slotUpdate();
}

void DlgPrefBeats::slotResetToDefaults() {
    if (m_availablePlugins.size() > 0) {
        m_selectedAnalyzerId = m_availablePlugins[0].id;
    }
    m_bAnalyzerEnabled = m_bpmSettings.getBpmDetectionEnabledDefault();
    m_bFixedTempoEnabled = m_bpmSettings.getFixedTempoAssumptionDefault();
    m_bOffsetEnabled = m_bpmSettings.getFixedTempoOffsetCorrectionDefault();
    m_bFastAnalysisEnabled = m_bpmSettings.getFastAnalysisDefault();
    m_bReanalyze = m_bpmSettings.getReanalyzeWhenSettingsChangeDefault();
    // TODO(rryan): Above range enabled is not exposed?
    m_minBpm = m_bpmSettings.getBpmRangeStartDefault();
    m_maxBpm = m_bpmSettings.getBpmRangeEndDefault();
    slotUpdate();
}

void DlgPrefBeats::pluginSelected(int i) {
    if (i == -1) {
        return;
    }
    m_selectedAnalyzerId = m_availablePlugins[i].id;
    slotUpdate();
}

void DlgPrefBeats::analyzerEnabled(int i) {
    m_bAnalyzerEnabled = static_cast<bool>(i);
    slotUpdate();
}

void DlgPrefBeats::fixedtempoEnabled(int i) {
    m_bFixedTempoEnabled = static_cast<bool>(i);
    slotUpdate();
}

void DlgPrefBeats::offsetEnabled(int i) {
    m_bOffsetEnabled = static_cast<bool>(i);
    slotUpdate();
}

void DlgPrefBeats::minBpmRangeChanged(int value) {
    m_minBpm = value;
    slotUpdate();
}

void DlgPrefBeats::maxBpmRangeChanged(int value) {
    m_maxBpm = value;
    slotUpdate();
}

void DlgPrefBeats::slotUpdate() {
    checkBoxFixedTempo->setEnabled(m_bAnalyzerEnabled);
    checkBoxOffsetCorr->setEnabled((m_bAnalyzerEnabled && m_bFixedTempoEnabled));
    comboBoxBeatPlugin->setEnabled(m_bAnalyzerEnabled);
    checkBoxAnalyzerEnabled->setChecked(m_bAnalyzerEnabled);
    // Fast analysis cannot be combined with non-constant tempo beatgrids.
    checkBoxFastAnalysis->setEnabled(m_bAnalyzerEnabled && m_bFixedTempoEnabled);
    txtMaxBpm->setEnabled(m_bAnalyzerEnabled && m_bFixedTempoEnabled);
    txtMinBpm->setEnabled(m_bAnalyzerEnabled && m_bFixedTempoEnabled);
    checkBoxReanalyse->setEnabled(m_bAnalyzerEnabled);
    checkBoxReanalyseImported->setEnabled(m_bReanalyzeImported);

    if (!m_bAnalyzerEnabled) {
        return;
    }

    if (m_availablePlugins.size() > 0) {
        bool found = false;
        for (int i = 0; i < m_availablePlugins.size(); ++i) {
            const auto& info = m_availablePlugins.at(i);
            if (info.id == m_selectedAnalyzerId) {
                found = true;
                comboBoxBeatPlugin->setCurrentIndex(i);
                if (!m_availablePlugins[i].constantTempoSupported) {
                    checkBoxFixedTempo->setEnabled(false);
                    checkBoxOffsetCorr->setEnabled(false);
                }
                break;
            }
        }
        if (!found) {
            comboBoxBeatPlugin->setCurrentIndex(0);
            m_selectedAnalyzerId = m_availablePlugins[0].id;
        }
    }

    checkBoxFixedTempo->setChecked(m_bFixedTempoEnabled);
    checkBoxOffsetCorr->setChecked(m_bOffsetEnabled);
    // Fast analysis cannot be combined with non-constant tempo beatgrids.
    checkBoxFastAnalysis->setChecked(m_bFastAnalysisEnabled && m_bFixedTempoEnabled);

    txtMaxBpm->setValue(m_maxBpm);
    txtMinBpm->setValue(m_minBpm);
    checkBoxReanalyse->setChecked(m_bReanalyze);
}

void DlgPrefBeats::slotReanalyzeChanged(int value) {
    m_bReanalyze = static_cast<bool>(value);
    slotUpdate();
}

void DlgPrefBeats::slotReanalyzeImportedChanged(int value) {
    m_bReanalyzeImported = static_cast<bool>(value);
    slotUpdate();
}

void DlgPrefBeats::fastAnalysisEnabled(int i) {
    m_bFastAnalysisEnabled = static_cast<bool>(i);
    slotUpdate();
}

void DlgPrefBeats::slotApply() {
    m_bpmSettings.setBeatPluginId(m_selectedAnalyzerId);
    m_bpmSettings.setBpmDetectionEnabled(m_bAnalyzerEnabled);
    m_bpmSettings.setFixedTempoAssumption(m_bFixedTempoEnabled);
    m_bpmSettings.setFixedTempoOffsetCorrection(m_bOffsetEnabled);
    m_bpmSettings.setReanalyzeWhenSettingsChange(m_bReanalyze);
    m_bpmSettings.setReanalyzeImported(m_bReanalyzeImported);
    m_bpmSettings.setFastAnalysis(m_bFastAnalysisEnabled);
    m_bpmSettings.setBpmRangeStart(m_minBpm);
    m_bpmSettings.setBpmRangeEnd(m_maxBpm);
}
