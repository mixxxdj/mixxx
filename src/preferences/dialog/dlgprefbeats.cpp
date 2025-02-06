#include "preferences/dialog/dlgprefbeats.h"

#include "analyzer/analyzerbeats.h"
#include "defs_urls.h"
#include "moc_dlgprefbeats.cpp"

DlgPrefBeats::DlgPrefBeats(QWidget* parent, UserSettingsPointer pConfig)
        : DlgPreferencePage(parent),
          m_bpmSettings(pConfig),
          m_bAnalyzerEnabled(m_bpmSettings.getBpmDetectionEnabledDefault()),
          m_bFixedTempoEnabled(m_bpmSettings.getFixedTempoAssumptionDefault()),
          m_bFastAnalysisEnabled(m_bpmSettings.getFastAnalysisDefault()),
          m_bReanalyze(m_bpmSettings.getReanalyzeWhenSettingsChangeDefault()),
          m_bReanalyzeImported(m_bpmSettings.getReanalyzeImportedDefault()) {
    setupUi(this);

    m_availablePlugins = AnalyzerBeats::availablePlugins();
    for (const auto& info : std::as_const(m_availablePlugins)) {
        comboBoxBeatPlugin->addItem(info.name(), info.id());
    }

    loadSettings();

    // Connections
    connect(comboBoxBeatPlugin,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefBeats::pluginSelected);
    connect(checkBoxAnalyzerEnabled,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefBeats::analyzerEnabled);
    connect(checkBoxFixedTempo,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefBeats::fixedtempoEnabled);
    connect(checkBoxFastAnalysis,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefBeats::fastAnalysisEnabled);
    connect(checkBoxReanalyze,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefBeats::slotReanalyzeChanged);
    connect(checkBoxReanalyzeImported,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefBeats::slotReanalyzeImportedChanged);

    setScrollSafeGuard(comboBoxBeatPlugin);
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
    m_bReanalyze =  m_bpmSettings.getReanalyzeWhenSettingsChange();
    m_bReanalyzeImported = m_bpmSettings.getReanalyzeImported();
    m_bFastAnalysisEnabled = m_bpmSettings.getFastAnalysis();

    slotUpdate();
}

void DlgPrefBeats::slotResetToDefaults() {
    if (m_availablePlugins.size() > 0) {
        m_selectedAnalyzerId = m_availablePlugins[0].id();
    }
    m_bAnalyzerEnabled = m_bpmSettings.getBpmDetectionEnabledDefault();
    m_bFixedTempoEnabled = m_bpmSettings.getFixedTempoAssumptionDefault();
    m_bFastAnalysisEnabled = m_bpmSettings.getFastAnalysisDefault();
    m_bReanalyze = m_bpmSettings.getReanalyzeWhenSettingsChangeDefault();
    m_bReanalyzeImported = m_bpmSettings.getReanalyzeImportedDefault();

    slotUpdate();
}

void DlgPrefBeats::pluginSelected(int i) {
    if (i == -1) {
        return;
    }
    m_selectedAnalyzerId = m_availablePlugins[i].id();
    slotUpdate();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void DlgPrefBeats::analyzerEnabled(Qt::CheckState state) {
    m_bAnalyzerEnabled = (state == Qt::Checked);
#else
void DlgPrefBeats::analyzerEnabled(int i) {
    m_bAnalyzerEnabled = static_cast<bool>(i);
#endif
    slotUpdate();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void DlgPrefBeats::fixedtempoEnabled(Qt::CheckState state) {
    m_bFixedTempoEnabled = (state == Qt::Checked);
#else
void DlgPrefBeats::fixedtempoEnabled(int i) {
    m_bFixedTempoEnabled = static_cast<bool>(i);
#endif
    slotUpdate();
}

void DlgPrefBeats::slotUpdate() {
    checkBoxFixedTempo->setEnabled(m_bAnalyzerEnabled);
    comboBoxBeatPlugin->setEnabled(m_bAnalyzerEnabled);
    checkBoxAnalyzerEnabled->setChecked(m_bAnalyzerEnabled);
    // Fast analysis cannot be combined with non-constant tempo beatgrids.
    checkBoxFastAnalysis->setEnabled(m_bAnalyzerEnabled && m_bFixedTempoEnabled);
    checkBoxReanalyze->setEnabled(m_bAnalyzerEnabled);
    checkBoxReanalyzeImported->setEnabled(m_bAnalyzerEnabled);

    if (!m_bAnalyzerEnabled) {
        return;
    }

    if (m_availablePlugins.size() > 0) {
        bool found = false;
        for (int i = 0; i < m_availablePlugins.size(); ++i) {
            const auto& info = m_availablePlugins.at(i);
            if (info.id() == m_selectedAnalyzerId) {
                found = true;
                comboBoxBeatPlugin->setCurrentIndex(i);
                if (!m_availablePlugins[i].isConstantTempoSupported()) {
                    checkBoxFixedTempo->setEnabled(false);
                }
                break;
            }
        }
        if (!found) {
            comboBoxBeatPlugin->setCurrentIndex(0);
            m_selectedAnalyzerId = m_availablePlugins[0].id();
        }
    }

    checkBoxFixedTempo->setChecked(m_bFixedTempoEnabled);
    // Fast analysis cannot be combined with non-constant tempo beatgrids.
    checkBoxFastAnalysis->setChecked(m_bFastAnalysisEnabled && m_bFixedTempoEnabled);

    checkBoxReanalyze->setChecked(m_bReanalyze);
    checkBoxReanalyzeImported->setChecked(m_bReanalyzeImported);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void DlgPrefBeats::slotReanalyzeChanged(Qt::CheckState state) {
    m_bReanalyze = (state == Qt::Checked);
#else
void DlgPrefBeats::slotReanalyzeChanged(int value) {
    m_bReanalyze = static_cast<bool>(value);
#endif
    slotUpdate();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void DlgPrefBeats::slotReanalyzeImportedChanged(Qt::CheckState state) {
    m_bReanalyzeImported = (state == Qt::Checked);
#else
void DlgPrefBeats::slotReanalyzeImportedChanged(int value) {
    m_bReanalyzeImported = static_cast<bool>(value);
#endif
    slotUpdate();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void DlgPrefBeats::fastAnalysisEnabled(Qt::CheckState state) {
    m_bFastAnalysisEnabled = (state == Qt::Checked);
#else
void DlgPrefBeats::fastAnalysisEnabled(int i) {
    m_bFastAnalysisEnabled = static_cast<bool>(i);
#endif
    slotUpdate();
}

void DlgPrefBeats::slotApply() {
    m_bpmSettings.setBeatPluginId(m_selectedAnalyzerId);
    m_bpmSettings.setBpmDetectionEnabled(m_bAnalyzerEnabled);
    m_bpmSettings.setFixedTempoAssumption(m_bFixedTempoEnabled);
    m_bpmSettings.setReanalyzeWhenSettingsChange(m_bReanalyze);
    m_bpmSettings.setReanalyzeImported(m_bReanalyzeImported);
    m_bpmSettings.setFastAnalysis(m_bFastAnalysisEnabled);
}
