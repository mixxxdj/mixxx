#include "preferences/dialog/dlgprefbeats.h"

#include "analyzer/analyzerbeats.h"
#include "control/controlobject.h"
#include "defs_urls.h"

DlgPrefBeats::DlgPrefBeats(QWidget* parent, UserSettingsPointer pConfig)
        : DlgPreferencePage(parent),
          m_bpmSettings(pConfig),
          m_minBpm(m_bpmSettings.getBpmRangeStartDefault()),
          m_maxBpm(m_bpmSettings.getBpmRangeEndDefault()),
          m_banalyzerEnabled(m_bpmSettings.getBpmDetectionEnabledDefault()),
          m_bfixedtempoEnabled(m_bpmSettings.getFixedTempoAssumptionDefault()),
          m_boffsetEnabled(m_bpmSettings.getFixedTempoOffsetCorrectionDefault()),
          m_FastAnalysisEnabled(m_bpmSettings.getFastAnalysisDefault()),
          m_bReanalyze(m_bpmSettings.getReanalyzeWhenSettingsChangeDefault()),
          m_bReanalyzeImported(m_bpmSettings.getReanalyzeImportedDefault()) {
    setupUi(this);

    m_availablePlugins = AnalyzerBeats::availablePlugins();
    for (const auto& info : m_availablePlugins) {
        plugincombo->addItem(info.name, info.id);
    }

    loadSettings();

    // Connections
    connect(plugincombo, SIGNAL(activated(int)),
            this, SLOT(pluginSelected(int)));
    connect(banalyzerenabled, SIGNAL(stateChanged(int)),
            this, SLOT(analyzerEnabled(int)));
    connect(bfixedtempo, SIGNAL(stateChanged(int)),
            this, SLOT(fixedtempoEnabled(int)));
    connect(boffset, SIGNAL(stateChanged(int)),
            this, SLOT(offsetEnabled(int)));

    connect(bFastAnalysis, SIGNAL(stateChanged(int)),
            this, SLOT(fastAnalysisEnabled(int)));

    connect(txtMinBpm, SIGNAL(valueChanged(int)),
            this, SLOT(minBpmRangeChanged(int)));
    connect(txtMaxBpm, SIGNAL(valueChanged(int)),
            this, SLOT(maxBpmRangeChanged(int)));

    connect(bReanalyse,SIGNAL(stateChanged(int)),
            this, SLOT(slotReanalyzeChanged(int)));
    connect(bReanalyzeImported,
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
    m_banalyzerEnabled = m_bpmSettings.getBpmDetectionEnabled();
    m_bfixedtempoEnabled = m_bpmSettings.getFixedTempoAssumption();
    m_boffsetEnabled = m_bpmSettings.getFixedTempoOffsetCorrection();
    m_bReanalyze =  m_bpmSettings.getReanalyzeWhenSettingsChange();
    m_FastAnalysisEnabled = m_bpmSettings.getFastAnalysis();

    // TODO(rryan): Above range enabled is not exposed?
    m_minBpm = m_bpmSettings.getBpmRangeStart();
    m_maxBpm = m_bpmSettings.getBpmRangeEnd();

    slotUpdate();
}

void DlgPrefBeats::slotResetToDefaults() {
    if (m_availablePlugins.size() > 0) {
        m_selectedAnalyzerId = m_availablePlugins[0].id;
    }
    m_banalyzerEnabled = m_bpmSettings.getBpmDetectionEnabledDefault();
    m_bfixedtempoEnabled = m_bpmSettings.getFixedTempoAssumptionDefault();
    m_boffsetEnabled = m_bpmSettings.getFixedTempoOffsetCorrectionDefault();
    m_FastAnalysisEnabled = m_bpmSettings.getFastAnalysisDefault();
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
    m_banalyzerEnabled = static_cast<bool>(i);
    slotUpdate();
}

void DlgPrefBeats::fixedtempoEnabled(int i) {
    m_bfixedtempoEnabled = static_cast<bool>(i);
    slotUpdate();
}

void DlgPrefBeats::offsetEnabled(int i) {
    m_boffsetEnabled = static_cast<bool>(i);
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
    bfixedtempo->setEnabled(m_banalyzerEnabled);
    boffset->setEnabled((m_banalyzerEnabled && m_bfixedtempoEnabled));
    plugincombo->setEnabled(m_banalyzerEnabled);
    banalyzerenabled->setChecked(m_banalyzerEnabled);
    // Fast analysis cannot be combined with non-constant tempo beatgrids.
    bFastAnalysis->setEnabled(m_banalyzerEnabled && m_bfixedtempoEnabled);
    txtMaxBpm->setEnabled(m_banalyzerEnabled && m_bfixedtempoEnabled);
    txtMinBpm->setEnabled(m_banalyzerEnabled && m_bfixedtempoEnabled);
    bReanalyse->setEnabled(m_banalyzerEnabled);
    bReanalyzeImported->setEnabled(m_bReanalyzeImported);

    if (!m_banalyzerEnabled) {
        return;
    }

    if (m_availablePlugins.size() > 0) {
        bool found = false;
        for (int i = 0; i < m_availablePlugins.size(); ++i) {
            const auto& info = m_availablePlugins.at(i);
            if (info.id == m_selectedAnalyzerId) {
                found = true;
                plugincombo->setCurrentIndex(i);
                if (!m_availablePlugins[i].constantTempoSupported) {
                    bfixedtempo->setEnabled(false);
                    boffset->setEnabled(false);
                }
                break;
            }
        }
        if (!found) {
            plugincombo->setCurrentIndex(0);
            m_selectedAnalyzerId = m_availablePlugins[0].id;
        }
    }

    bfixedtempo->setChecked(m_bfixedtempoEnabled);
    boffset->setChecked(m_boffsetEnabled);
    // Fast analysis cannot be combined with non-constant tempo beatgrids.
    bFastAnalysis->setChecked(m_FastAnalysisEnabled && m_bfixedtempoEnabled);

    txtMaxBpm->setValue(m_maxBpm);
    txtMinBpm->setValue(m_minBpm);
    bReanalyse->setChecked(m_bReanalyze);
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
    m_FastAnalysisEnabled = static_cast<bool>(i);
    slotUpdate();
}

void DlgPrefBeats::slotApply() {
    m_bpmSettings.setBeatPluginId(m_selectedAnalyzerId);
    m_bpmSettings.setBpmDetectionEnabled(m_banalyzerEnabled);
    m_bpmSettings.setFixedTempoAssumption(m_bfixedtempoEnabled);
    m_bpmSettings.setFixedTempoOffsetCorrection(m_boffsetEnabled);
    m_bpmSettings.setReanalyzeWhenSettingsChange(m_bReanalyze);
    m_bpmSettings.setReanalyzeImported(m_bReanalyzeImported);
    m_bpmSettings.setFastAnalysis(m_FastAnalysisEnabled);
    m_bpmSettings.setBpmRangeStart(m_minBpm);
    m_bpmSettings.setBpmRangeEnd(m_maxBpm);
}
