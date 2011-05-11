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
#include "vamp-hostsdk/PluginLoader.h"
#include "vamp-hostsdk/vamp-hostsdk.h"
#include "vamp-hostsdk/Plugin.h"
#include "controlobject.h"
#include "dlgprefanalysers.h"

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
    m_bShowAll = false;
    m_moreless = "Advanced";
    setupUi(this);

    populate();
    loadSettings();
    //Connections
    connect(plugincombo, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(pluginSelected(QString)));
    connect(reset, SIGNAL(clicked(bool)),     this, SLOT(setDefaults()));
    connect(advanced, SIGNAL(clicked(bool)),  this, SLOT(setAdvanced()));


}

DlgPrefAnalysers::~DlgPrefAnalysers() {
}


void DlgPrefAnalysers::loadSettings(){
    if(m_pconfig->getValueString(ConfigKey(CONFIG_KEY,"AnalyserBeatPluginID"))==QString("")) {
        setDefaults();
    } else {
        QString pluginid = m_pconfig->getValueString(ConfigKey(CONFIG_KEY,"AnalyserBeatPluginID"));
        //m_iselectedAnalyser = plugincombo->findText(pluginid);
        m_selectedAnalyser = pluginid;
        if(m_listIdentifier.indexOf(pluginid)==-1){
            setDefaults();
        }
        slotUpdate();
    }
}

void DlgPrefAnalysers::setDefaults(){

    if(m_listIdentifier.indexOf("qm-tempotracker:0")==-1){
        qDebug()<<"qm-tempotracker Vamp plugin not found";
        return;
    }
    m_selectedAnalyser = "qm-tempotracker:0";
    slotApply();
    slotUpdate();
}

void DlgPrefAnalysers::setAdvanced(){
    m_bShowAll = !m_bShowAll;
    m_moreless = m_bShowAll ? "Basic" : "Advanced";
    advanced->setText(m_moreless);
    populate();
    slotUpdate();
}
void DlgPrefAnalysers::pluginSelected(QString i){
    m_selectedAnalyser = i;
    slotUpdate();

}


void DlgPrefAnalysers::slotUpdate(){
    int comboselected = plugincombo->findText(m_selectedAnalyser);
    if( comboselected==-1)
        return;
    plugincombo->setCurrentIndex(comboselected);
    int selected = m_listIdentifier.indexOf(m_selectedAnalyser);
    name->setText(tr("Name")+": "+m_listName[selected]);
    version->setText(tr("Version")+": "+m_listVersion[selected]);
    maker->setText(tr("Maker")+": "+m_listMaker[selected]);
    copyright->setText(tr("Copyright")+": "+m_listCopyright[selected]);
    output->setText(tr("Output")+": "+m_listOutput[selected]);
    description->setText(tr("Description")+": "+m_listDescription[selected]);
}

void DlgPrefAnalysers::slotApply(){
    //if(m_iselectedAnalyser>m_listIdentifier.size())
    //    return;
    int selected = m_listIdentifier.indexOf(m_selectedAnalyser);
    if (selected == -1)
        return;
    m_pconfig->set(ConfigKey(CONFIG_KEY,"AnalyserBeatLibrary"), ConfigValue(m_listLibrary[selected]));
    m_pconfig->set(ConfigKey(CONFIG_KEY,"AnalyserBeatPluginID"), ConfigValue(m_selectedAnalyser));
    m_pconfig->Save();
    m_bShowAll = false;
}

void DlgPrefAnalysers::populate() {
    QString selectedAnalyser = m_selectedAnalyser;
    m_listIdentifier.clear();
    m_listName.clear();
    m_listVersion.clear();
    m_listMaker.clear();
    m_listCopyright.clear();
    m_listOutput.clear();
    m_listDescription.clear();
    m_listLibrary.clear();
    disconnect(plugincombo, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(pluginSelected(QString)));
    plugincombo->clear();
    plugincombo->setDuplicatesEnabled(false);
    connect(plugincombo, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(pluginSelected(QString)));
    PluginLoader *loader = PluginLoader::getInstance();
    std::vector<PluginLoader::PluginKey> plugins = loader->listPlugins();
    for (unsigned int iplugin=0; iplugin < plugins.size(); iplugin++) {
        Plugin *plugin = loader->loadPlugin(plugins[iplugin], 48000);
        //TODO: find a way to add beat trackers only
        if (plugin) {
            Plugin::OutputList outputs = plugin->getOutputDescriptors();
            for (unsigned int ioutput=0; ioutput < outputs.size(); ioutput++) {

                //validate and add rows to qcombobox

                m_listName << QString::fromStdString(plugin->getName());
                m_listVersion << QString::number(plugin->getPluginVersion());
                m_listMaker << QString::fromStdString(plugin->getMaker());
                m_listCopyright << QString::fromStdString(plugin->getCopyright());
                m_listOutput << QString::number(ioutput);
                m_listDescription << QString::fromStdString(outputs[ioutput].description);
                QString pluginlibrary = QString::fromStdString(loader->getLibraryPathForPlugin(plugins[iplugin]));
                m_listLibrary << pluginlibrary;
                QString displayname = QString::fromStdString(plugin->getIdentifier()) + ":"
                        + QString::number(ioutput);
                m_listIdentifier << displayname;
                bool goodones = ((displayname.contains("mixxxbpmdetection")||
                        displayname.contains("qm-barbeattracker:0")||
                        displayname.contains("qm-tempotracker:0"))&&pluginlibrary.contains("mixxxminimal"));
                if (m_bShowAll||goodones){
                    plugincombo->addItem(displayname, displayname);
                }
            }
            delete plugin;
            plugin = 0;
        }
    }
    m_selectedAnalyser = selectedAnalyser;
}

