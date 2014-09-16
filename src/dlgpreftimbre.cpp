#include <qcheckbox.h>
#include <vamp-hostsdk/vamp-hostsdk.h>

#include "dlgpreftimbre.h"
#include "track/timbre_preferences.h"
#include "vamp/vampanalyser.h"
#include "ui_dlgpreftimbredlg.h"

using Vamp::Plugin;
using Vamp::PluginHostAdapter;
using Vamp::HostExt::PluginLoader;
using Vamp::HostExt::PluginWrapper;
using Vamp::HostExt::PluginInputDomainAdapter;

DlgPrefTimbre::DlgPrefTimbre(QWidget *parent, ConfigObject<ConfigValue> *pConfig)
        : DlgPreferencePage(parent),
        Ui::DlgPrefTimbreDlg(),
        m_pConfig(pConfig),
        m_bAnalyserEnabled(false) {
    setupUi(this);

    populate();
    loadSettings();

    connect(comboBoxPlugin, SIGNAL(currentIndexChanged(int)),
            this, SLOT(pluginSelected(int)));
    connect(checkBoxAnalyserEnabled, SIGNAL(stateChanged(int)),
            this, SLOT(analyserEnabled(int)));
}

DlgPrefTimbre::~DlgPrefTimbre() {
}

void DlgPrefTimbre::slotApply() {
    int selected = m_listIdentifier.indexOf(m_selectedAnalyser);
    if (selected == -1)
        return;

    m_pConfig->set(
        ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYSER_TIMBRE_LIBRARY),
        ConfigValue(m_listLibrary[selected]));
    m_pConfig->set(
        ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYSER_TIMBRE_PLUGIN_ID),
        ConfigValue(m_selectedAnalyser));
    m_pConfig->set(
        ConfigKey(TIMBRE_CONFIG_KEY, TIMBRE_ANALYSIS_ENABLED),
        ConfigValue(m_bAnalyserEnabled ? 1 : 0));
    m_pConfig->Save();
}

void DlgPrefTimbre::slotUpdate() {
    comboBoxPlugin->setEnabled(m_bAnalyserEnabled);
    checkBoxAnalyserEnabled->setChecked(m_bAnalyserEnabled);
    slotApply();
    if (!m_bAnalyserEnabled) {
        return;
    }

    int comboSelected = m_listIdentifier.indexOf(m_selectedAnalyser);
    if (comboSelected == -1) {
        qDebug() << "DlgPrefTimbre: Plugin not found in slotUpdate()";
        return;
    }
    comboBoxPlugin->setCurrentIndex(comboSelected);
}

void DlgPrefTimbre::pluginSelected(int i) {
    if (i==-1) {
        return;
    }
    m_selectedAnalyser = m_listIdentifier[i];
    slotUpdate();
}

void DlgPrefTimbre::analyserEnabled(int i) {
    m_bAnalyserEnabled = static_cast<bool>(i);
    slotUpdate();
}

void DlgPrefTimbre::setDefaults() {
    qDebug() << "DlgPrefTimbre::setDefaults";
    m_bAnalyserEnabled = true;
    m_selectedAnalyser = VAMP_ANALYSER_TIMBRE_DEFAULT_PLUGIN_ID;
    if (!m_listIdentifier.contains(m_selectedAnalyser)) {
        qDebug() << "DlgPrefTimbre: qm-similarity Vamp plugin not found";
        m_bAnalyserEnabled = false;
    }
    slotUpdate();
}

void DlgPrefTimbre::populate() {
    VampAnalyser::initializePluginPaths();
    m_listIdentifier.clear();
    m_listName.clear();
    m_listLibrary.clear();
    comboBoxPlugin->clear();
    comboBoxPlugin->setDuplicatesEnabled(false);
    VampPluginLoader *loader = VampPluginLoader::getInstance();
    std::vector<PluginLoader::PluginKey> plugins = loader->listPlugins();
    qDebug() << "VampPluginLoader::listPlugins() returned" << plugins.size()
             << "plugins";
    for (unsigned int iPlugin=0; iPlugin < plugins.size(); iPlugin++) {
        Plugin *plugin = loader->loadPlugin(plugins[iPlugin], 48000);
        if (plugin) {
            Plugin::OutputList outputs = plugin->getOutputDescriptors();
            for (unsigned int iOutput=0; iOutput < outputs.size(); iOutput++) {
                QString displayName = QString::fromStdString(plugin->getIdentifier())
                    + ":" + QString::number(iOutput);
                QString displayNameText = QString::fromStdString(plugin->getName());
                bool isTimbreDetector = displayName.contains(
                    VAMP_ANALYSER_TIMBRE_DEFAULT_PLUGIN_ID);
                if (isTimbreDetector) {
                    m_listName << displayNameText;
                    QString pluginlibrary = QString::fromStdString(
                        plugins[iPlugin]).section(":",0,0);
                    m_listLibrary << pluginlibrary;
                    m_listIdentifier << displayName;
                    comboBoxPlugin->addItem(displayNameText, displayName);
                }
            }
            delete plugin;
            plugin = NULL;
        }
    }
}

void DlgPrefTimbre::loadSettings() {
    qDebug() << "DlgPrefTimbre::loadSettings";
    if(m_pConfig->getValueString(
        ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYSER_TIMBRE_PLUGIN_ID))==QString("")) {
        setDefaults();
        slotApply();    // Write to config file so AnalyzerTimbre can get the data
        return;
    }
    QString pluginid = m_pConfig->getValueString(
        ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYSER_TIMBRE_PLUGIN_ID));
    m_selectedAnalyser = pluginid;

    m_bAnalyserEnabled = static_cast<bool>(m_pConfig->getValueString(
        ConfigKey(TIMBRE_CONFIG_KEY, TIMBRE_ANALYSIS_ENABLED)).toInt());

    if (!m_listIdentifier.contains(pluginid)) {
        setDefaults();
    }
    slotUpdate();
}
