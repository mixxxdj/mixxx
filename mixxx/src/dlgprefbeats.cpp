/*
 *  Created on: 28/apr/2011
 *      Author: vittorio
 */

#include <qlineedit.h>
#include <qwidget.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <QVector>
#include <QList>
#include <QtCore>
#include <vamp-hostsdk/vamp-hostsdk.h>

#include "track/beat_preferences.h"
#include "controlobject.h"
#include "dlgprefbeats.h"
#include "vamp/vampanalyser.h"

using Vamp::Plugin;
using Vamp::PluginHostAdapter;
using Vamp::HostExt::PluginLoader;
using Vamp::HostExt::PluginWrapper;
using Vamp::HostExt::PluginInputDomainAdapter;

DlgPrefBeats::DlgPrefBeats(QWidget *parent, ConfigObject<ConfigValue> *_config)
        : QWidget(parent),
          m_selectedAnalyser() {
    m_pconfig = _config;
    setupUi(this);

    populate();
    loadSettings();
    //Connections
    connect(plugincombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(pluginSelected(int)));
    connect(banalyserenabled, SIGNAL(stateChanged(int)),
            this, SLOT(analyserEnabled(int)));
    connect(bfixedtempo, SIGNAL(stateChanged(int)),
            this, SLOT(fixedtempoEnabled(int)));
    connect(boffset, SIGNAL(stateChanged(int)),
            this, SLOT(offsetEnabled(int)));
    connect(reset, SIGNAL(clicked(bool)),     this, SLOT(setDefaults()));

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

void DlgPrefBeats::loadSettings(){
    if(m_pconfig->getValueString(
        ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYSER_BEAT_PLUGIN_ID))==QString("")) {
        setDefaults();
        slotApply();    // Write to config file so AnalyzerBeats can get the data
        return;
    }

    QString pluginid = m_pconfig->getValueString(
        ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYSER_BEAT_PLUGIN_ID));
    m_selectedAnalyser = pluginid;

    m_banalyserEnabled = static_cast<bool>(m_pconfig->getValueString(
        ConfigKey(BPM_CONFIG_KEY, BPM_DETECTION_ENABLED)).toInt());

    m_bfixedtempoEnabled = static_cast<bool>(m_pconfig->getValueString(
        ConfigKey(BPM_CONFIG_KEY, BPM_FIXED_TEMPO_ASSUMPTION)).toInt());

    m_boffsetEnabled = static_cast<bool>(m_pconfig->getValueString(
        ConfigKey(BPM_CONFIG_KEY, BPM_FIXED_TEMPO_OFFSET_CORRECTION)).toInt());

    m_bReanalyze =  static_cast<bool>(m_pconfig->getValueString(
        ConfigKey(BPM_CONFIG_KEY, BPM_REANALYZE_WHEN_SETTINGS_CHANGE)).toInt());

    m_FastAnalysisEnabled = static_cast<bool>(m_pconfig->getValueString(
        ConfigKey(BPM_CONFIG_KEY, BPM_FAST_ANALYSIS_ENABLED)).toInt());

    if (!m_listIdentifier.contains(pluginid)) {
        setDefaults();
    }
    m_minBpm = m_pconfig->getValueString(ConfigKey(BPM_CONFIG_KEY, BPM_RANGE_START)).toInt();
    m_maxBpm = m_pconfig->getValueString(ConfigKey(BPM_CONFIG_KEY, BPM_RANGE_END)).toInt();

    slotUpdate();
}

void DlgPrefBeats::setDefaults() {
    if (m_listIdentifier.size()==0) {
        qDebug() << "DlgPrefBeats:No Vamp plugin not found";
        return;
    }
    if (m_listIdentifier.contains("qm-tempotracker:0")) {
        m_selectedAnalyser = "qm-tempotracker:0";
    } else {
        // the first one will always be the soundtouch one. defined in
        // vamp-plugins/libmain.cpp
        m_selectedAnalyser = m_listIdentifier.at(0);
    }
    m_banalyserEnabled = true;
    m_bfixedtempoEnabled = true;
    m_boffsetEnabled = true;
    m_FastAnalysisEnabled = false;
    m_bReanalyze = false;

    m_minBpm = 70;
    m_maxBpm = 140;
    //slotApply();
    slotUpdate();
}

void DlgPrefBeats::pluginSelected(int i){
    if (i==-1)
        return;
    m_selectedAnalyser = m_listIdentifier[i];
    slotUpdate();
}

void  DlgPrefBeats::analyserEnabled(int i){
    m_banalyserEnabled = static_cast<bool>(i);
    slotUpdate();
}

void  DlgPrefBeats::fixedtempoEnabled(int i){
    m_bfixedtempoEnabled = static_cast<bool>(i);
    slotUpdate();
}

void DlgPrefBeats::offsetEnabled(int i){
    m_boffsetEnabled = static_cast<bool>(i);
    slotUpdate();
}

void DlgPrefBeats::minBpmRangeChanged(int value){
    m_minBpm = value;
    slotUpdate();
}

void DlgPrefBeats::maxBpmRangeChanged(int value){
    m_maxBpm = value;
    slotUpdate();
}

void DlgPrefBeats::slotUpdate(){
    bfixedtempo->setEnabled(m_banalyserEnabled);
    boffset->setEnabled((m_banalyserEnabled && m_bfixedtempoEnabled));
    plugincombo->setEnabled(m_banalyserEnabled);
    banalyserenabled->setChecked(m_banalyserEnabled);
    bFastAnalysis->setEnabled(m_banalyserEnabled);
    txtMaxBpm->setEnabled(m_banalyserEnabled && m_bfixedtempoEnabled);
    txtMinBpm->setEnabled(m_banalyserEnabled && m_bfixedtempoEnabled);
    bReanalyse->setEnabled(m_banalyserEnabled);

    if (!m_banalyserEnabled) {
        return;
    }

    if (m_selectedAnalyser != "qm-tempotracker:0") {
        bfixedtempo->setEnabled(false);
        boffset->setEnabled(false);
    }

    bfixedtempo->setChecked(m_bfixedtempoEnabled);
    boffset->setChecked(m_boffsetEnabled);
    bFastAnalysis->setChecked(m_FastAnalysisEnabled);

    int comboselected = m_listIdentifier.indexOf(m_selectedAnalyser);
    if ( comboselected==-1) {
        qDebug()<<"DlgPrefBeats: Plugin("<<m_selectedAnalyser<<") not found in slotUpdate()";
        return;
    }

    plugincombo->setCurrentIndex(comboselected);
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
    int selected = m_listIdentifier.indexOf(m_selectedAnalyser);
    if (selected == -1)
        return;

    m_pconfig->set(ConfigKey(
        VAMP_CONFIG_KEY, VAMP_ANALYSER_BEAT_LIBRARY), ConfigValue(m_listLibrary[selected]));
    m_pconfig->set(ConfigKey(
        VAMP_CONFIG_KEY, VAMP_ANALYSER_BEAT_PLUGIN_ID), ConfigValue(m_selectedAnalyser));
    m_pconfig->set(ConfigKey(
        BPM_CONFIG_KEY, BPM_DETECTION_ENABLED), ConfigValue(m_banalyserEnabled ? 1 : 0));
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
    m_pconfig->Save();
}

void DlgPrefBeats::populate() {
    VampAnalyser::initializePluginPaths();
    m_listIdentifier.clear();
    m_listName.clear();
    m_listLibrary.clear();
    disconnect(plugincombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(pluginSelected(int)));
    plugincombo->clear();
    plugincombo->setDuplicatesEnabled(false);
    connect(plugincombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(pluginSelected(int)));
    VampPluginLoader *loader = VampPluginLoader::getInstance();
    std::vector<PluginLoader::PluginKey> plugins = loader->listPlugins();
    qDebug() << "VampPluginLoader::listPlugins() returned" << plugins.size() << "plugins";
    for (unsigned int iplugin=0; iplugin < plugins.size(); iplugin++) {
        Plugin *plugin = loader->loadPlugin(plugins[iplugin], 48000);
        //TODO: find a way to add beat trackers only
        if (plugin) {
            Plugin::OutputList outputs = plugin->getOutputDescriptors();
            for (unsigned int ioutput=0; ioutput < outputs.size(); ioutput++) {
                QString displayname = QString::fromStdString(plugin->getIdentifier()) + ":"
                                            + QString::number(ioutput);
                QString displaynametext = QString::fromStdString(plugin->getName());
                qDebug() << "Plugin output displayname:" << displayname << displaynametext;
                bool goodones = ((displayname.contains("mixxxbpmdetection")||
                                  displayname.contains("qm-tempotracker:0"))||
                                 displayname.contains("beatroot:0")||
                                 displayname.contains("marsyas_ibt:0")||
                                 displayname.contains("aubiotempo:0")
                                 );
                if (goodones) {
                    m_listName << displaynametext;
                    QString pluginlibrary = QString::fromStdString(plugins[iplugin]).section(":",0,0);
                    m_listLibrary << pluginlibrary;
                    QString displayname = QString::fromStdString(plugin->getIdentifier()) + ":"
                            + QString::number(ioutput);
                    m_listIdentifier << displayname;
                    plugincombo->addItem(displaynametext, displayname);
                }
            }
            delete plugin;
            plugin = 0;
        }
    }
    // m_selectedAnalyser = selectedAnalyser;
}

