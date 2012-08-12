/***************************************************************************
                          dlgprefkey.cpp  -  description
                             -------------------
    begin                : Thu Jun 7 2012
    copyright            : (C) 2012 by Keith Salisbury
    email                : keithsalisbury@gmail.com
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <qlineedit.h>
#include <qfiledialog.h>
#include <qwidget.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <QtCore>
#include <QMessageBox>
#include "track/key_preferences.h"

#include "dlgprefkey.h"
#include "xmlparse.h"
#include "controlobject.h"
//#include "dlgprefbeats.h"
#include "vamp/vampanalyser.h"

using Vamp::Plugin;
using Vamp::PluginHostAdapter;
using Vamp::HostExt::PluginLoader;
using Vamp::HostExt::PluginWrapper;
using Vamp::HostExt::PluginInputDomainAdapter;


#define CONFIG_KEY "[KEY]"
bool firs = true;
DlgPrefKey::DlgPrefKey(QWidget * parent,
                       ConfigObject<ConfigValue> * _config) : QWidget(parent),m_pconfig(_config)
  //Ui::DlgPrefKey()
{
    //config = _config;
    //m_pconfig = _config;
    setupUi(this);
    //m_selectedAnalyser = "qm-tempotracker:0";
    //setupUi(this);

   // populate();
    loadSettings();
    //Connections
  //  connect(plugincombo, SIGNAL(currentIndexChanged(int)),
    //        this, SLOT(pluginSelected(int)));
    connect(banalyserenabled, SIGNAL(stateChanged(int)),
          this, SLOT(analyserEnabled(int)));
    connect(bfastAnalysisEnabled, SIGNAL(stateChanged(int)),
            this, SLOT(fastAnalysisEnabled(int)));
    connect(bfirstLastEnabled, SIGNAL(stateChanged(int)),
            this, SLOT(firstLastEnabled(int)));
   // connect(reset, SIGNAL(clicked(bool)), this, SLOT(setDefaults()));

    connect(breanalyzeEnabled, SIGNAL(stateChanged(int)),
            this, SLOT(reanalyzeEnabled(int)));

    connect(bskipRelevantEnabled, SIGNAL(stateChanged(int)),
            this, SLOT(skipRelevantEnabled(int)));
   // connect(txtMaxBpm, SIGNAL(valueChanged(int)),
      //      this, SLOT(maxBpmRangeChanged(int)));

    //connect(bReanalyse,SIGNAL(stateChanged(int)),
        //    this, SLOT(slotReanalyzeChanged(int)));

}

DlgPrefKey::~DlgPrefKey()
{
}

void DlgPrefKey::loadSettings(){
    //if(m_pconfig->getValueString(
      //  ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYSER_KEY_PLUGIN_ID))==QString("")) {
        //setDefaults();
           // Write to config file so AnalyzerBeats can get the data
        //return;
    //}

   // QString pluginid = m_pconfig->getValueString(
     //   ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYSER_BEAT_PLUGIN_ID));
    //m_selectedAnalyser = pluginid;

    m_banalyserEnabled = static_cast<bool>(m_pconfig->getValueString(
        ConfigKey(KEY_CONFIG_KEY, KEY_DETECTION_ENABLED)).toInt());

    m_bfastAnalysisEnabled = static_cast<bool>(m_pconfig->getValueString(
        ConfigKey(KEY_CONFIG_KEY, KEY_FAST_ANALYSIS)).toInt());

    m_bfirstLastEnabled = static_cast<bool>(m_pconfig->getValueString(
        ConfigKey(KEY_CONFIG_KEY, KEY_FIRST_LAST)).toInt());

    m_breanalyzeEnabled =  static_cast<bool>(m_pconfig->getValueString(
        ConfigKey(KEY_CONFIG_KEY, KEY_REANALYZE)).toInt());

    m_bskipRelevantEnabled = static_cast<bool>(m_pconfig->getValueString(
        ConfigKey(KEY_CONFIG_KEY, KEY_SKIP_RELEVANT)).toInt());
    slotApply();
  //  if (!m_listIdentifier.contains(pluginid)) {
    //    setDefaults();
    //}
   // m_minBpm = m_pconfig->getValueString(ConfigKey(BPM_CONFIG_KEY, BPM_RANGE_START)).toInt();
    //m_maxBpm = m_pconfig->getValueString(ConfigKey(BPM_CONFIG_KEY, BPM_RANGE_END)).toInt();

    slotUpdate();
}

void DlgPrefKey::setDefaults() {
    //if (!m_listIdentifier.contains("qm-tempotracker:0")) {
      //  qDebug() << "DlgPrefBeats: qm-tempotracker Vamp plugin not found";
        //return;
    //}
    //m_selectedAnalyser = "qm-tempotracker:0";
    m_banalyserEnabled = false;
    m_bfastAnalysisEnabled = false;
    m_bfirstLastEnabled = false;
    m_breanalyzeEnabled = false;
    //m_FastAnalysisEnabled = false;
    m_bskipRelevantEnabled = false;

    //m_minBpm = 70;
    //m_maxBpm = 140;
    //slotApply();
    slotUpdate();
}

void  DlgPrefKey::analyserEnabled(int i){
    m_banalyserEnabled = static_cast<bool>(i);
    slotUpdate();
}

void  DlgPrefKey::fastAnalysisEnabled(int i){
    m_bfastAnalysisEnabled = static_cast<bool>(i);
    slotUpdate();
}

void DlgPrefKey::firstLastEnabled(int i){
    m_bfirstLastEnabled = static_cast<bool>(i);
    slotUpdate();
}

void DlgPrefKey::reanalyzeEnabled(int i){
    m_breanalyzeEnabled = static_cast<bool>(i);
    slotUpdate();
}

void DlgPrefKey::skipRelevantEnabled(int i){
    m_bskipRelevantEnabled = static_cast<bool>(i);
    qDebug()<<m_bskipRelevantEnabled;
    slotUpdate();
    }



void DlgPrefKey::slotApply()
{
    //int selected = m_listIdentifier.indexOf(m_selectedAnalyser);
    //if (selected == -1)
     //   return;

    //m_pconfig->set(ConfigKey(
      //  VAMP_CONFIG_KEY, VAMP_ANALYSER_BEAT_LIBRARY), ConfigValue(m_listLibrary[selected]));
    //m_pconfig->set(ConfigKey(
      //  VAMP_CONFIG_KEY, VAMP_ANALYSER_BEAT_PLUGIN_ID), ConfigValue(m_selectedAnalyser));
    m_pconfig->set(ConfigKey(
        KEY_CONFIG_KEY, KEY_DETECTION_ENABLED), ConfigValue(m_banalyserEnabled ? 1 : 0));
    m_pconfig->set(ConfigKey(
        KEY_CONFIG_KEY, KEY_FAST_ANALYSIS), ConfigValue(m_bfastAnalysisEnabled ? 1 : 0));
    m_pconfig->set(ConfigKey(
        KEY_CONFIG_KEY, KEY_FIRST_LAST), ConfigValue(m_bfirstLastEnabled ? 1 : 0));
    m_pconfig->set(ConfigKey(
        KEY_CONFIG_KEY, KEY_REANALYZE), ConfigValue(m_breanalyzeEnabled ? 1 : 0));
    m_pconfig->set(ConfigKey(
        KEY_CONFIG_KEY, KEY_SKIP_RELEVANT), ConfigValue(m_bskipRelevantEnabled ? 1 : 0));

    // m_pconfig->set(ConfigKey(
     //   KEY_CONFIG_KEY, KEY_SKIP_RELEVANT), ConfigValue(1));
    //  m_pconfig->set(ConfigKey(
    //    BPM_CONFIG_KEY, BPM_FAST_ANALYSIS_ENABLED), ConfigValue(m_FastAnalysisEnabled ? 1 : 0));

    //m_pconfig->set(ConfigKey(BPM_CONFIG_KEY, BPM_RANGE_START), ConfigValue(m_minBpm));
    //m_pconfig->set(ConfigKey(BPM_CONFIG_KEY, BPM_RANGE_END), ConfigValue(m_maxBpm));
    m_pconfig->Save();
}

void DlgPrefKey::slotUpdate()
{
    //bfixedtempo->setEnabled(m_banalyserEnabled);
    //boffset->setEnabled((m_banalyserEnabled && m_bfixedtempoEnabled));
    //plugincombo->setEnabled(m_banalyserEnabled);
    banalyserenabled->setChecked(m_banalyserEnabled);
    bfastAnalysisEnabled->setChecked(m_bfastAnalysisEnabled);
    //txtMaxBpm->setEnabled(m_banalyserEnabled && m_bfixedtempoEnabled);
    // txtMinBpm->setEnabled(m_banalyserEnabled && m_bfixedtempoEnabled);
    bfirstLastEnabled->setChecked(m_bfirstLastEnabled);
    breanalyzeEnabled->setChecked(m_breanalyzeEnabled);
    bskipRelevantEnabled->setChecked(m_bskipRelevantEnabled);
    slotApply();

    //if(!m_banalyserEnabled)
      //  return;

    //bfixedtempo->setChecked(m_bfixedtempoEnabled);
    //boffset->setChecked(m_boffsetEnabled);
    //bFastAnalysis->setChecked(m_FastAnalysisEnabled);

    //int comboselected = m_listIdentifier.indexOf(m_selectedAnalyser);
    //if( comboselected==-1){
      //  qDebug()<<"DlgPrefBeats: Plugin not found in slotUpdate()";
        //return;
    //}

    //plugincombo->setCurrentIndex(comboselected);
    //txtMaxBpm->setValue(m_maxBpm);
    //txtMinBpm->setValue(m_minBpm);
    //bReanalyse->setChecked(m_bReanalyze);
}

//void DlgPrefBeats::populate() {
  //  VampAnalyser::initializePluginPaths();
//    QString selectedAnalyser = m_selectedAnalyser;
//    m_listIdentifier.clear();
//    m_listName.clear();
//    m_listLibrary.clear();
//    disconnect(plugincombo, SIGNAL(currentIndexChanged(int)),
//            this, SLOT(pluginSelected(int)));
//    plugincombo->clear();
//    plugincombo->setDuplicatesEnabled(false);
//    connect(plugincombo, SIGNAL(currentIndexChanged(int)),
//            this, SLOT(pluginSelected(int)));
//    VampPluginLoader *loader = VampPluginLoader::getInstance();
//    std::vector<PluginLoader::PluginKey> plugins = loader->listPlugins();
//    qDebug() << "VampPluginLoader::listPlugins() returned" << plugins.size() << "plugins";
//    for (unsigned int iplugin=0; iplugin < plugins.size(); iplugin++) {
//        Plugin *plugin = loader->loadPlugin(plugins[iplugin], 48000);
//        //TODO: find a way to add beat trackers only
//        if (plugin) {
//            Plugin::OutputList outputs = plugin->getOutputDescriptors();
//            for (unsigned int ioutput=0; ioutput < outputs.size(); ioutput++) {
//                QString displayname = QString::fromStdString(plugin->getIdentifier()) + ":"
//                                            + QString::number(ioutput);
//                QString displaynametext = QString::fromStdString(plugin->getName());
//                qDebug() << "Plugin output displayname:" << displayname << displaynametext;
//                bool goodones = ((displayname.contains("mixxxbpmdetection")||
//                                  displayname.contains("qm-tempotracker:0"))||
//                                 displayname.contains("beatroot:0")||
//                                 displayname.contains("marsyas_ibt:0")||
//                                 displayname.contains("aubiotempo:0")
//                                 );

//                bool goodones = ((displaynametext.contains("Beat Track")));
                //validate and add rows to qcombobox

//                m_listVersion << QString::number(plugin->getPluginVersion());
//                m_listMaker << QString::fromStdString(plugin->getMaker());
//                m_listCopyright << QString::fromStdString(plugin->getCopyright());
//                m_listOutput << QString::number(ioutput);
//                m_listDescription << QString::fromStdString(outputs[ioutput].description);

//                if (goodones) {
//                    m_listName << displaynametext;
//                    QString pluginlibrary = QString::fromStdString(plugins[iplugin]).section(":",0,0);
//                    m_listLibrary << pluginlibrary;
//                    QString displayname = QString::fromStdString(plugin->getIdentifier()) + ":"
//                            + QString::number(ioutput);
//                    m_listIdentifier << displayname;
//                    plugincombo->addItem(displaynametext, displayname);
//                }
//            }
//            delete plugin;
//            plugin = 0;
//        }
//    }
//    m_selectedAnalyser = selectedAnalyser;
//}

