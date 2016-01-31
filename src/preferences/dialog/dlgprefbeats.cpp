#include "preferences/dialog/dlgprefbeats.h"

#include "analyzer/analyzerbeats.h"
#include "controlobject.h"
#include "track/beat_preferences.h"

DlgPrefBeats::DlgPrefBeats(QWidget *parent, UserSettingsPointer _config)
        : DlgPreferencePage(parent),
          m_pconfig(_config),
          m_minBpm(0),
          m_maxBpm(0),
          m_banalyzerEnabled(false),
          m_bfixedtempoEnabled(false),
          m_boffsetEnabled(false),
          m_FastAnalysisEnabled(false),
          m_bReanalyze(false) {
    setupUi(this);

    m_availablePlugins = AnalyzerBeats::availablePlugins();
    for (const AnalyzerPluginInfo& info : m_availablePlugins) {
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
}

DlgPrefBeats::~DlgPrefBeats() {
}

void DlgPrefBeats::loadSettings() {
    QString beatPluginId = m_pconfig->getValueString(
        ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYZER_BEAT_PLUGIN_ID));
    if (beatPluginId.isEmpty()) {
        slotResetToDefaults();
        slotApply();    // Write to config file so AnalyzerBeats can get the data
        return;
    }

    m_selectedAnalyzerId = beatPluginId;

    m_banalyzerEnabled = static_cast<bool>(m_pconfig->getValueString(
        ConfigKey(BPM_CONFIG_KEY, BPM_DETECTION_ENABLED)).toInt());

    m_bfixedtempoEnabled = static_cast<bool>(m_pconfig->getValueString(
        ConfigKey(BPM_CONFIG_KEY, BPM_FIXED_TEMPO_ASSUMPTION)).toInt());

    m_boffsetEnabled = static_cast<bool>(m_pconfig->getValueString(
        ConfigKey(BPM_CONFIG_KEY, BPM_FIXED_TEMPO_OFFSET_CORRECTION)).toInt());

    m_bReanalyze =  static_cast<bool>(m_pconfig->getValueString(
        ConfigKey(BPM_CONFIG_KEY, BPM_REANALYZE_WHEN_SETTINGS_CHANGE)).toInt());

    m_FastAnalysisEnabled = static_cast<bool>(m_pconfig->getValueString(
        ConfigKey(BPM_CONFIG_KEY, BPM_FAST_ANALYSIS_ENABLED)).toInt());

    m_minBpm = m_pconfig->getValueString(ConfigKey(BPM_CONFIG_KEY, BPM_RANGE_START)).toInt();
    m_maxBpm = m_pconfig->getValueString(ConfigKey(BPM_CONFIG_KEY, BPM_RANGE_END)).toInt();

    slotUpdate();
}

void DlgPrefBeats::slotResetToDefaults() {
    // TODO(rryan): Select QM Beat Tracker here.
    m_selectedAnalyzerId = "qm-tempotracker";
    m_banalyzerEnabled = true;
    m_bfixedtempoEnabled = true;
    m_boffsetEnabled = true;
    m_FastAnalysisEnabled = false;
    m_bReanalyze = false;
    m_minBpm = 70;
    m_maxBpm = 140;
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
    bFastAnalysis->setEnabled(m_banalyzerEnabled);
    txtMaxBpm->setEnabled(m_banalyzerEnabled && m_bfixedtempoEnabled);
    txtMinBpm->setEnabled(m_banalyzerEnabled && m_bfixedtempoEnabled);
    bReanalyse->setEnabled(m_banalyzerEnabled);

    if (!m_banalyzerEnabled) {
        return;
    }

    // TODO(rryan)
    if (m_selectedAnalyzerId != "qm-tempotracker") {
        bfixedtempo->setEnabled(false);
        boffset->setEnabled(false);
    }

    bfixedtempo->setChecked(m_bfixedtempoEnabled);
    boffset->setChecked(m_boffsetEnabled);
    bFastAnalysis->setChecked(m_FastAnalysisEnabled);

    for (int i = 0; i < m_availablePlugins.size(); ++i) {
        const auto& info = m_availablePlugins.at(i);
        if (info.id == m_selectedAnalyzerId) {
            plugincombo->setCurrentIndex(i);
            break;
        }
    }

    txtMaxBpm->setValue(m_maxBpm);
    txtMinBpm->setValue(m_minBpm);
    bReanalyse->setChecked(m_bReanalyze);
}

void DlgPrefBeats::slotReanalyzeChanged(int value) {
    m_bReanalyze = static_cast<bool>(value);
    slotUpdate();
}

void DlgPrefBeats::fastAnalysisEnabled(int i) {
    m_FastAnalysisEnabled = static_cast<bool>(i);
    slotUpdate();
}

void DlgPrefBeats::slotApply() {
    // TODO(rryan)
    // m_pconfig->set(ConfigKey(
    //     VAMP_CONFIG_KEY, VAMP_ANALYZER_BEAT_LIBRARY), ConfigValue(m_listLibrary[selected]));
    m_pconfig->set(ConfigKey(
        VAMP_CONFIG_KEY, VAMP_ANALYZER_BEAT_PLUGIN_ID), ConfigValue(m_selectedAnalyzerId));
    m_pconfig->set(ConfigKey(
        BPM_CONFIG_KEY, BPM_DETECTION_ENABLED), ConfigValue(m_banalyzerEnabled ? 1 : 0));
    m_pconfig->set(ConfigKey(
        BPM_CONFIG_KEY, BPM_FIXED_TEMPO_ASSUMPTION), ConfigValue(m_bfixedtempoEnabled ? 1 : 0));
    m_pconfig->set(ConfigKey(
        BPM_CONFIG_KEY, BPM_FIXED_TEMPO_OFFSET_CORRECTION), ConfigValue(m_boffsetEnabled ? 1 : 0));
    m_pconfig->set(ConfigKey(
        BPM_CONFIG_KEY, BPM_REANALYZE_WHEN_SETTINGS_CHANGE), ConfigValue(m_bReanalyze ? 1 : 0));
    m_pconfig->set(ConfigKey(
        BPM_CONFIG_KEY, BPM_FAST_ANALYSIS_ENABLED), ConfigValue(m_FastAnalysisEnabled ? 1 : 0));

    m_pconfig->set(ConfigKey(BPM_CONFIG_KEY, BPM_RANGE_START), ConfigValue(m_minBpm));
    m_pconfig->set(ConfigKey(BPM_CONFIG_KEY, BPM_RANGE_END), ConfigValue(m_maxBpm));
}
