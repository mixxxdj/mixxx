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
    m_iselectedAnalyser = 0;
    setupUi(this);

    populate();

    //Connections
    connect(plugincombo, SIGNAL(currentIndexChanged(int)),
                this, SLOT(pluginSelected(int)));
    connect(reset, SIGNAL(clicked(bool)),     this, SLOT(setDefaults()));
    loadSettings();

}

DlgPrefAnalysers::~DlgPrefAnalysers() {
}


void DlgPrefAnalysers::loadSettings(){
    if(m_pconfig->getValueString(ConfigKey(CONFIG_KEY,"AnalyserBeatPluginID"))==QString("")) {
           setDefaults();
       } else {
            QString pluginid = m_pconfig->getValueString(ConfigKey(CONFIG_KEY,"AnalyserBeatPluginID"));
           m_iselectedAnalyser = plugincombo->findText(pluginid);

       }
   qDebug()<<"Selected analyser number"<<m_iselectedAnalyser;
   //slotApply();
   slotUpdate();
}

void DlgPrefAnalysers::setDefaults(){
qDebug()<<"Set to default";
m_iselectedAnalyser = plugincombo->findText("qm-barbeattracker:0");
slotApply();
}


void DlgPrefAnalysers::pluginSelected(int i){
    m_iselectedAnalyser = i;
    if(m_listName.isEmpty())
        return;
    // fill everything


    slotUpdate();
}


void DlgPrefAnalysers::slotUpdate(){
        name->setText(tr("Name")+": "+m_listName[m_iselectedAnalyser]);
        version->setText(tr("Version")+": "+m_listVersion[m_iselectedAnalyser]);
        maker->setText(tr("Maker")+": "+m_listMaker[m_iselectedAnalyser]);
        copyright->setText(tr("Copyright")+": "+m_listCopyright[m_iselectedAnalyser]);
        output->setText(tr("Output")+": "+m_listOutput[m_iselectedAnalyser]);
        description->setText(tr("Description")+": "+m_listDescription[m_iselectedAnalyser]);
}

void DlgPrefAnalysers::slotApply(){
   plugincombo->setCurrentIndex(m_iselectedAnalyser);
   m_pconfig->set(ConfigKey(CONFIG_KEY,"AnalyserBeatLibrary"), ConfigValue(m_listLibrary[m_iselectedAnalyser]));
   QString key = m_listIdentifier[m_iselectedAnalyser] + ":" + m_listOutput[m_iselectedAnalyser];
   m_pconfig->set(ConfigKey(CONFIG_KEY,"AnalyserBeatPluginID"), ConfigValue(key));
   m_pconfig->Save();
}

void DlgPrefAnalysers::populate() {
    m_listIdentifier.clear();
    m_listName.clear();
    m_listVersion.clear();
    m_listMaker.clear();
    m_listCopyright.clear();
    m_listOutput.clear();
    m_listDescription.clear();
    m_listLibrary.clear();

    plugincombo->clear();
    plugincombo->setDuplicatesEnabled(false);

    PluginLoader *loader = PluginLoader::getInstance();
    std::vector<PluginLoader::PluginKey> plugins = loader->listPlugins();
    for (unsigned int iplugin=0; iplugin < plugins.size(); iplugin++) {
        Plugin *plugin = loader->loadPlugin(plugins[iplugin], 48000);
        //TODO: find a way to add beat trackers only
        if (plugin) {
            Plugin::OutputList outputs = plugin->getOutputDescriptors();
            for (unsigned int ioutput=0; ioutput < outputs.size(); ioutput++) {

                //validate and add rows to qcombobox
                m_listIdentifier << QString::fromStdString(plugin->getIdentifier());
                m_listName << QString::fromStdString(plugin->getName());
                m_listVersion << QString::number(plugin->getPluginVersion());
                m_listMaker << QString::fromStdString(plugin->getMaker());
                m_listCopyright << QString::fromStdString(plugin->getCopyright());
                m_listOutput << QString::number(ioutput);
                m_listDescription << QString::fromStdString(outputs[ioutput].description);
                m_listLibrary
                        << QString::fromStdString(loader->getLibraryPathForPlugin(plugins[iplugin]));
                QString displayname = QString::fromStdString(plugin->getIdentifier()) + ":"
                        + QString::number(ioutput);
                plugincombo->addItem(displayname, displayname);
            }
            delete plugin;
            plugin = 0;
        }
    }
}

