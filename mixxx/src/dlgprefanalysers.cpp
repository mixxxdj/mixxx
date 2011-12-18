/*
 * dlgprefanalysers.cpp
 *
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
#include <QVector>
#include <QList>
#include <QtCore>
#include <vamp-hostsdk/vamp-hostsdk.h>
#include "controlobject.h"
#include "dlgprefanalysers.h"
#include "vamp/vampanalyser.h"

#define CONFIG_KEY "[Vamp]"

using Vamp::Plugin;
using Vamp::PluginHostAdapter;
using Vamp::HostExt::PluginLoader;
using Vamp::HostExt::PluginWrapper;
using Vamp::HostExt::PluginInputDomainAdapter;


DlgPrefAnalysers::DlgPrefAnalysers(QWidget *parent, ConfigObject<ConfigValue> *_config): QWidget(parent)
, Ui::DlgAnalysersDlg()
{
    m_pconfig = _config;
    m_selectedAnalyser = "qm-tempotracker:0";
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


}

DlgPrefAnalysers::~DlgPrefAnalysers() {
}


void DlgPrefAnalysers::loadSettings(){
    if(m_pconfig->getValueString(ConfigKey(CONFIG_KEY,"AnalyserBeatPluginID"))==QString("")) {
        setDefaults();
    } else {
        QString pluginid = m_pconfig->getValueString(ConfigKey(CONFIG_KEY,"AnalyserBeatPluginID"));
        m_selectedAnalyser = pluginid;
        int i = m_pconfig->getValueString(ConfigKey(CONFIG_KEY,"AnalyserBeatEnabled")).toInt();
        m_banalyserEnabled = static_cast<bool>(i);

        i = m_pconfig->getValueString(ConfigKey(CONFIG_KEY,"AnalyserBeatFixedTempo")).toInt();
        m_bfixedtempoEnabled = static_cast<bool>(i);

        i = m_pconfig->getValueString(ConfigKey(CONFIG_KEY,"AnalyserBeatOffset")).toInt();
        m_boffsetEnabled = static_cast<bool>(i);

        if(m_listIdentifier.indexOf(pluginid)==-1){
            setDefaults();
        }
        slotUpdate();
    }
}

void DlgPrefAnalysers::setDefaults(){

    if(m_listIdentifier.indexOf("qm-tempotracker:0")==-1){
        qDebug()<<"DlgPrefAnalysers: qm-tempotracker Vamp plugin not found";
        return;
    }
    m_selectedAnalyser = "qm-tempotracker:0";
    m_banalyserEnabled = true;
    m_bfixedtempoEnabled = true;
    m_boffsetEnabled = true;
    //slotApply();
    slotUpdate();
}

void DlgPrefAnalysers::pluginSelected(int i){
    if (i==-1)
        return;
    m_selectedAnalyser = m_listIdentifier[i];
    slotUpdate();

}

void  DlgPrefAnalysers::analyserEnabled(int i){
    m_banalyserEnabled = static_cast<bool>(i);
    slotUpdate();

}

void  DlgPrefAnalysers::fixedtempoEnabled(int i){
    m_bfixedtempoEnabled = static_cast<bool>(i);
    slotUpdate();

}

void DlgPrefAnalysers::offsetEnabled(int i){
    m_boffsetEnabled = static_cast<bool>(i);
    slotUpdate();

}


void DlgPrefAnalysers::slotUpdate(){

    bfixedtempo->setEnabled(m_banalyserEnabled);
    boffset->setEnabled((m_banalyserEnabled && m_bfixedtempoEnabled));
    plugincombo->setEnabled(m_banalyserEnabled);
    banalyserenabled->setChecked(m_banalyserEnabled);
    if(!m_banalyserEnabled)
        return;


    bfixedtempo->setChecked(m_bfixedtempoEnabled);
    boffset->setChecked(m_boffsetEnabled);

    int comboselected = m_listIdentifier.indexOf(m_selectedAnalyser);
    if( comboselected==-1){
        qDebug()<<"DlgPrefAnalysers: Plugin not found in slotUpdate()";
        return;
    }

    plugincombo->setCurrentIndex(comboselected);


}

void DlgPrefAnalysers::slotApply(){

    int selected = m_listIdentifier.indexOf(m_selectedAnalyser);
    if (selected == -1)
        return;

    if (m_banalyserEnabled)
        m_pconfig->set(ConfigKey(CONFIG_KEY,"AnalyserBeatEnabled"), ConfigValue(1));
    else
        m_pconfig->set(ConfigKey(CONFIG_KEY,"AnalyserBeatEnabled"), ConfigValue(0));

    if (m_bfixedtempoEnabled)
        m_pconfig->set(ConfigKey(CONFIG_KEY,"AnalyserBeatFixedTempo"), ConfigValue(1));
    else
        m_pconfig->set(ConfigKey(CONFIG_KEY,"AnalyserBeatFixedTempo"), ConfigValue(0));


    if (m_boffsetEnabled)
        m_pconfig->set(ConfigKey(CONFIG_KEY,"AnalyserBeatOffset"), ConfigValue(1));
    else
        m_pconfig->set(ConfigKey(CONFIG_KEY,"AnalyserBeatOffset"), ConfigValue(0));

    m_pconfig->set(ConfigKey(CONFIG_KEY,"AnalyserBeatLibrary"), ConfigValue(m_listLibrary[selected]));
    m_pconfig->set(ConfigKey(CONFIG_KEY,"AnalyserBeatPluginID"), ConfigValue(m_selectedAnalyser));
    m_pconfig->Save();
}

void DlgPrefAnalysers::populate() {
    VampAnalyser::initializePluginPaths();
    QString selectedAnalyser = m_selectedAnalyser;
    m_listIdentifier.clear();
    m_listName.clear();
    m_listLibrary.clear();
    disconnect(plugincombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(pluginSelected(int)));
    plugincombo->clear();
    plugincombo->setDuplicatesEnabled(false);
    connect(plugincombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(pluginSelected(int)));
    PluginLoader *loader = PluginLoader::getInstance();
    std::vector<PluginLoader::PluginKey> plugins = loader->listPlugins();
    for (unsigned int iplugin=0; iplugin < plugins.size(); iplugin++) {
        Plugin *plugin = loader->loadPlugin(plugins[iplugin], 48000);
        //TODO: find a way to add beat trackers only
        if (plugin) {
            Plugin::OutputList outputs = plugin->getOutputDescriptors();
            for (unsigned int ioutput=0; ioutput < outputs.size(); ioutput++) {
                QString displayname = QString::fromStdString(plugin->getIdentifier()) + ":"
                                            + QString::number(ioutput);
                QString displaynametext = QString::fromStdString(plugin->getName());
                bool goodones = ((displayname.contains("mixxxbpmdetection")||
                                        displayname.contains("qm-tempotracker:0"))||
                                        displayname.contains("beatroot:0")||
                                        displayname.contains("marsyas_ibt:0")||
                                        displayname.contains("aubiotempo:0")
                                        );

//                bool goodones = ((displaynametext.contains("Beat Track")));
                //validate and add rows to qcombobox

//                m_listVersion << QString::number(plugin->getPluginVersion());
//                m_listMaker << QString::fromStdString(plugin->getMaker());
//                m_listCopyright << QString::fromStdString(plugin->getCopyright());
//                m_listOutput << QString::number(ioutput);
//                m_listDescription << QString::fromStdString(outputs[ioutput].description);





                if (goodones){
                    m_listName << displaynametext;
                    QString pluginlibrary = QString::fromStdString(loader->getLibraryPathForPlugin(plugins[iplugin]));
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
    m_selectedAnalyser = selectedAnalyser;
}

