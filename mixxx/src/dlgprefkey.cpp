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
#include "vamp/vampanalyser.h"
#include "vamp/vamppluginloader.h"

using Vamp::Plugin;
using Vamp::PluginHostAdapter;
using Vamp::HostExt::PluginLoader;
using Vamp::HostExt::PluginWrapper;
using Vamp::HostExt::PluginInputDomainAdapter;

DlgPrefKey::DlgPrefKey(QWidget* parent, ConfigObject<ConfigValue>* _config)
        : QWidget(parent),
          Ui::DlgPrefKeyDlg(),
          m_pconfig(_config),
          m_bAnalyserEnabled(false),
          m_bFastAnalysisEnabled(false),
          m_bReanalyzeEnabled(false) {
    setupUi(this);

    m_keyLineEdits.insert(mixxx::track::io::key::C_MAJOR, c_major_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::D_FLAT_MAJOR, d_flat_major_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::D_MAJOR, d_major_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::E_FLAT_MAJOR, e_flat_major_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::E_MAJOR, e_major_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::F_MAJOR, f_major_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::F_SHARP_MAJOR, f_sharp_major_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::G_MAJOR, g_major_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::A_FLAT_MAJOR, a_flat_major_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::A_MAJOR, a_major_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::B_FLAT_MAJOR, b_flat_major_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::B_MAJOR, b_major_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::C_MINOR, c_minor_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::C_SHARP_MINOR, c_sharp_minor_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::D_MINOR, d_minor_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::E_FLAT_MINOR, e_flat_minor_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::E_MINOR, e_minor_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::F_MINOR, f_minor_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::F_SHARP_MINOR, f_sharp_minor_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::G_MINOR, g_minor_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::G_SHARP_MINOR, g_sharp_minor_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::A_MINOR, a_minor_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::B_FLAT_MINOR, b_flat_minor_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::B_MINOR, b_minor_edit);

    populate();
    loadSettings();

    // Connections
    connect(plugincombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(pluginSelected(int)));
    connect(banalyserenabled, SIGNAL(stateChanged(int)),
            this, SLOT(analyserEnabled(int)));
    connect(bfastAnalysisEnabled, SIGNAL(stateChanged(int)),
            this, SLOT(fastAnalysisEnabled(int)));
    // connect(reset, SIGNAL(clicked(bool)), this, SLOT(setDefaults()));
    connect(breanalyzeEnabled, SIGNAL(stateChanged(int)),
            this, SLOT(reanalyzeEnabled(int)));

    connect(radioNotationOpenKey, SIGNAL(toggled(bool)),
            this, SLOT(setNotationOpenKey(bool)));
    connect(radioNotationLancelot, SIGNAL(toggled(bool)),
            this, SLOT(setNotationLancelot(bool)));
    connect(radioNotationCustom, SIGNAL(toggled(bool)),
            this, SLOT(setNotationCustom(bool)));
}

DlgPrefKey::~DlgPrefKey() {
}

void DlgPrefKey::loadSettings(){
    qDebug() << "DlgPrefKey::loadSettings";

    qDebug() << "Key plugin ID:" << m_pconfig->getValueString(
        ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYSER_KEY_PLUGIN_ID));

    if(m_pconfig->getValueString(
        ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYSER_KEY_PLUGIN_ID)) == "") {
        setDefaults();
        slotApply(); // Write to config file so AnalyserKey can get the data
        return;
    }

   QString pluginid = m_pconfig->getValueString(
       ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYSER_KEY_PLUGIN_ID));
    m_selectedAnalyser = pluginid;

    m_bAnalyserEnabled = static_cast<bool>(m_pconfig->getValueString(
        ConfigKey(KEY_CONFIG_KEY, KEY_DETECTION_ENABLED)).toInt());

    m_bFastAnalysisEnabled = static_cast<bool>(m_pconfig->getValueString(
        ConfigKey(KEY_CONFIG_KEY, KEY_FAST_ANALYSIS)).toInt());

    m_bReanalyzeEnabled = static_cast<bool>(m_pconfig->getValueString(
        ConfigKey(KEY_CONFIG_KEY, KEY_REANALYZE_WHEN_SETTINGS_CHANGE)).toInt());

    QString notation = m_pconfig->getValueString(
        ConfigKey(KEY_CONFIG_KEY, KEY_NOTATION));
    if (notation == KEY_NOTATION_OPEN_KEY) {
        radioNotationOpenKey->setChecked(true);
        setNotationOpenKey(true);
    } else if (notation == KEY_NOTATION_LANCELOT) {
        radioNotationLancelot->setChecked(true);
        setNotationLancelot(true);
    } else if (notation == KEY_NOTATION_CUSTOM) {
        radioNotationCustom->setChecked(true);
        for (QMap<mixxx::track::io::key::ChromaticKey, QLineEdit*>::const_iterator it =
                     m_keyLineEdits.begin();
             it != m_keyLineEdits.end(); ++it) {
            it.value()->setText(m_pconfig->getValueString(
                ConfigKey(KEY_CONFIG_KEY, KEY_NOTATION_CUSTOM_PREFIX +
                          QString::number(it.key()))));
        }
        setNotationCustom(true);
    } else {
        radioNotationOpenKey->setChecked(true);
        setNotationOpenKey(true);
    }

    qDebug() << "loadSettings" << pluginid << m_bAnalyserEnabled << m_bFastAnalysisEnabled << m_bReanalyzeEnabled << notation;
    if (!m_listIdentifier.contains(pluginid)) {
        setDefaults();
    }
    slotUpdate();
}

void DlgPrefKey::setDefaults() {
    qDebug() << "DlgPrefKey::setDefaults";
    m_bAnalyserEnabled = true;
    m_bFastAnalysisEnabled = false;
    m_bReanalyzeEnabled = false;
    m_selectedAnalyser = VAMP_ANALYSER_KEY_DEFAULT_PLUGIN_ID;
    if (!m_listIdentifier.contains(m_selectedAnalyser)) {
        qDebug() << "DlgPrefKey: qm-keydetector Vamp plugin not found";
        m_bAnalyserEnabled = false;
    }

    radioNotationOpenKey->setChecked(true);
    setNotationOpenKey(true);

    //slotApply();
    slotUpdate();
}

void DlgPrefKey::pluginSelected(int i){
    if (i==-1)
        return;
    m_selectedAnalyser = m_listIdentifier[i];
    slotUpdate();
}

void  DlgPrefKey::analyserEnabled(int i){
    m_bAnalyserEnabled = static_cast<bool>(i);
    slotUpdate();
}

void  DlgPrefKey::fastAnalysisEnabled(int i){
    m_bFastAnalysisEnabled = static_cast<bool>(i);
    slotUpdate();
}

void DlgPrefKey::reanalyzeEnabled(int i){
    m_bReanalyzeEnabled = static_cast<bool>(i);
    slotUpdate();
}

void DlgPrefKey::slotApply() {
    qDebug() << "DlgPrefKey::slotApply";;
    int selected = m_listIdentifier.indexOf(m_selectedAnalyser);
    if (selected == -1)
        return;

    m_pconfig->set(
        ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYSER_KEY_LIBRARY),
        ConfigValue(m_listLibrary[selected]));
    m_pconfig->set(
        ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYSER_KEY_PLUGIN_ID),
        ConfigValue(m_selectedAnalyser));
    m_pconfig->set(
        ConfigKey(KEY_CONFIG_KEY, KEY_DETECTION_ENABLED),
        ConfigValue(m_bAnalyserEnabled ? 1 : 0));
    m_pconfig->set(
        ConfigKey(KEY_CONFIG_KEY, KEY_FAST_ANALYSIS),
        ConfigValue(m_bFastAnalysisEnabled ? 1 : 0));
    m_pconfig->set(
        ConfigKey(KEY_CONFIG_KEY, KEY_REANALYZE_WHEN_SETTINGS_CHANGE),
        ConfigValue(m_bReanalyzeEnabled ? 1 : 0));

    QMap<mixxx::track::io::key::ChromaticKey, QString> notation;
    if (radioNotationLancelot->isChecked()) {
        m_pconfig->set(
            ConfigKey(KEY_CONFIG_KEY, KEY_NOTATION),
            ConfigValue(KEY_NOTATION_LANCELOT));

        // This is just a handy way to iterate the keys. We don't use the QLineEdits.
        for (QMap<mixxx::track::io::key::ChromaticKey, QLineEdit*>::const_iterator it =
                     m_keyLineEdits.begin();
             it != m_keyLineEdits.end(); ++it) {
            notation[it.key()] = KeyUtils::keyToString(
                it.key(), KeyUtils::LANCELOT);
        }
    } else if (radioNotationCustom->isChecked()) {
        m_pconfig->set(
            ConfigKey(KEY_CONFIG_KEY, KEY_NOTATION),
            ConfigValue(KEY_NOTATION_CUSTOM));

        for (QMap<mixxx::track::io::key::ChromaticKey, QLineEdit*>::const_iterator it =
                     m_keyLineEdits.begin();
             it != m_keyLineEdits.end(); ++it) {
            notation[it.key()] = it.value()->text();
            m_pconfig->set(
                ConfigKey(KEY_CONFIG_KEY, KEY_NOTATION_CUSTOM_PREFIX +
                          QString::number(it.key())),
                ConfigValue(it.value()->text()));
        }
    } else {
        // Either OpenKey was chosen or somehow no radio button was chosen.
        m_pconfig->set(
            ConfigKey(KEY_CONFIG_KEY, KEY_NOTATION),
            ConfigValue(KEY_NOTATION_OPEN_KEY));

        // This is just a handy way to iterate the keys. We don't use the QLineEdits.
        for (QMap<mixxx::track::io::key::ChromaticKey, QLineEdit*>::const_iterator it =
                     m_keyLineEdits.begin();
             it != m_keyLineEdits.end(); ++it) {
            notation[it.key()] = KeyUtils::keyToString(
                it.key(), KeyUtils::OPEN_KEY);
        }
    }

    KeyUtils::setNotation(notation);
    // TODO(rryan): Re-draw the whole GUI somehow to repaint every notation
    // change?

    m_pconfig->Save();
}

void DlgPrefKey::slotUpdate() {
    qDebug() << "DlgPrefKey::slotUpdate";
    plugincombo->setEnabled(m_bAnalyserEnabled);
    banalyserenabled->setChecked(m_bAnalyserEnabled);
    bfastAnalysisEnabled->setChecked(m_bFastAnalysisEnabled);
    breanalyzeEnabled->setChecked(m_bReanalyzeEnabled);
    slotApply();

    if (!m_bAnalyserEnabled)
        return;

    int comboselected = m_listIdentifier.indexOf(m_selectedAnalyser);
    if (comboselected == -1){
        qDebug() << "DlgPrefKey: Plugin not found in slotUpdate()";
        return;
    }
    plugincombo->setCurrentIndex(comboselected);
}

void DlgPrefKey::populate() {
   VampAnalyser::initializePluginPaths();
   m_listIdentifier.clear();
   m_listName.clear();
   m_listLibrary.clear();
   plugincombo->clear();
   plugincombo->setDuplicatesEnabled(false);
   VampPluginLoader *loader = VampPluginLoader::getInstance();
   std::vector<PluginLoader::PluginKey> plugins = loader->listPlugins();
   qDebug() << "VampPluginLoader::listPlugins() returned" << plugins.size() << "plugins";
   for (unsigned int iplugin=0; iplugin < plugins.size(); iplugin++) {
       // TODO(XXX): WTF, 48000
       Plugin *plugin = loader->loadPlugin(plugins[iplugin], 48000);
       //TODO: find a way to add key detectors only
       if (plugin) {
           Plugin::OutputList outputs = plugin->getOutputDescriptors();
           for (unsigned int ioutput=0; ioutput < outputs.size(); ioutput++) {
               QString displayname = QString::fromStdString(plugin->getIdentifier()) + ":"
                                           + QString::number(ioutput);
               QString displaynametext = QString::fromStdString(plugin->getName());
               qDebug() << "Plugin output displayname:" << displayname << displaynametext;
               bool goodones = displayname.contains(VAMP_ANALYSER_KEY_DEFAULT_PLUGIN_ID);

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
}

void DlgPrefKey::setNotationCustom(bool active) {
    if (!active) {
        return;
    }

    for (QMap<mixxx::track::io::key::ChromaticKey, QLineEdit*>::const_iterator it =
                 m_keyLineEdits.begin();
         it != m_keyLineEdits.end(); ++it) {
        it.value()->setEnabled(true);
    }
    slotUpdate();
}

void DlgPrefKey::setNotationOpenKey(bool active) {
    if (!active) {
        return;
    }

    for (QMap<mixxx::track::io::key::ChromaticKey, QLineEdit*>::const_iterator it =
                 m_keyLineEdits.begin();
         it != m_keyLineEdits.end(); ++it) {
        it.value()->setText(KeyUtils::keyToString(
            it.key(), KeyUtils::OPEN_KEY));
        it.value()->setEnabled(false);
    }
    slotUpdate();
}

void DlgPrefKey::setNotationLancelot(bool active) {
    if (!active) {
        return;
    }

    for (QMap<mixxx::track::io::key::ChromaticKey, QLineEdit*>::const_iterator it =
                 m_keyLineEdits.begin();
         it != m_keyLineEdits.end(); ++it) {
        it.value()->setText(KeyUtils::keyToString(
            it.key(), KeyUtils::LANCELOT));
        it.value()->setEnabled(false);
    }
    slotUpdate();
}
