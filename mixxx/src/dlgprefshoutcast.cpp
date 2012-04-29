/***************************************************************************
                          dlgprefshoutcast.cpp  -  description
                             -------------------
    begin                : Thu Jun 19 2007
    copyright            : (C) 2008 by Wesley Stessens
                           (C) 2008 by Albert Santoni
                           (C) 2007 by John Sully
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dlgprefshoutcast.h"
#include "defs_urls.h"
#include <QtDebug>
#include <QtCore>
#include <QtGui>

DlgPrefShoutcast::DlgPrefShoutcast(QWidget *parent, ConfigObject<ConfigValue> *_config)
        : QWidget(parent) {
    m_pConfig = _config;
    int tmp_index = 0;  //Used for finding the index of an element by name in a combobox.
    QString tmp_string;
    setupUi(this);

    m_pUpdateShoutcastFromPrefs = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(SHOUTCAST_PREF_KEY, "update_from_prefs")));

    //Connections
    //connect(PushButtonBrowse, SIGNAL(clicked()),	this,	SLOT(slotBrowseSave()));
    //connect(LineEditRecPath,  SIGNAL(returnPressed()),  this,	SLOT(slotApply()));

    /*
    connect(comboBoxEncoding, SIGNAL(activated(int)),	this,	SLOT(slotRecordPathChange()));
    connect(SliderQuality,    SIGNAL(valueChanged(int)), this,	SLOT(slotSliderQuality()));
    connect(SliderQuality,    SIGNAL(sliderMoved(int)),	this,	SLOT(slotSliderQuality()));
    connect(SliderQuality,    SIGNAL(sliderReleased()), this,	SLOT(slotSliderQuality()));
    */
    //connect(CheckBoxRecord,    SIGNAL(stateChanged(int)), this, SLOT(slotApply()));


    //Enable live broadcasting checkbox
    enableLiveBroadcasting->setChecked((bool)m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"enabled")).toInt());

    //Server type combobox
    tmp_index = comboBoxServerType->findText(m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"servertype")));
    if (tmp_index < 0) //Set default if invalid.
        tmp_index = 0;
    comboBoxServerType->setCurrentIndex(tmp_index);

    //Mountpoint
    mountpoint->setText(m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"mountpoint")));

    //Host
    host->setText(m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"host")));

    //Port
    tmp_string = m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"port"));
    if (tmp_string.isEmpty())
        tmp_string = QString(SHOUTCAST_DEFAULT_PORT);
    port->setText(tmp_string);

    //Login
    login->setText(m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"login")));

    //Password
    password->setText(m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"password")));

    //Stream name
    stream_name->setText(m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"stream_name")));

    //Stream website
    tmp_string = m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"stream_website"));
    if (tmp_string.isEmpty())
        tmp_string = MIXXX_WEBSITE_URL;
    stream_website->setText(tmp_string);

    //Stream description
    stream_desc->setText(m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"stream_desc")));

    //Stream genre
    tmp_string = m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"stream_genre"));
    if (tmp_string.isEmpty())
        tmp_string = "Live Mix";
    stream_genre->setText(tmp_string);

    //Stream "public" checkbox
    stream_public->setChecked((bool)m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"stream_public")).toInt());

    //Encoding bitrate combobox
    tmp_index = comboBoxEncodingBitrate->findText(m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"bitrate")));
    if (tmp_index < 0) //Set default if invalid.
        tmp_index = 5; // 128 kbps
    comboBoxEncodingBitrate->setCurrentIndex(tmp_index);

    //Encoding format combobox
    tmp_index = comboBoxEncodingFormat->findText(m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"format")));
    if (tmp_index < 0) //Set default if invalid.
        tmp_index = 0;
    comboBoxEncodingFormat->setCurrentIndex(tmp_index);

    //Encoding channels combobox
    tmp_index = comboBoxEncodingChannels->findText(m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"channels")));
    if (tmp_index < 0) //Set default if invalid.
        tmp_index = 0;
    comboBoxEncodingChannels->setCurrentIndex(tmp_index);

    //"Enable custom metadata" checkbox
    enableCustomMetadata->setChecked((bool)m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"enable_metadata")).toInt());

    //Custom artist
    custom_artist->setText(m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"custom_artist")));

    //Custom title
    custom_title->setText(m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"custom_title")));


    slotApply();

    //recordControl->queueFromThread(RECORD_OFF); //make sure a corrupt config file won't cause us to record constantly

}

DlgPrefShoutcast::~DlgPrefShoutcast()
{
    delete m_pUpdateShoutcastFromPrefs;
}

void DlgPrefShoutcast::slotUpdate()
{
    enableLiveBroadcasting->setChecked((bool)m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"enabled")).toInt());
}

void DlgPrefShoutcast::slotApply()
{
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "enabled"),       ConfigValue(enableLiveBroadcasting->isChecked()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "servertype"),    ConfigValue(comboBoxServerType->currentText()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "mountpoint"),    ConfigValue(mountpoint->text()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "host"),          ConfigValue(host->text()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "port"),          ConfigValue(port->text()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "login"),         ConfigValue(login->text()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "password"),      ConfigValue(password->text()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "stream_name"),   ConfigValue(stream_name->text()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "stream_website"),ConfigValue(stream_website->text()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "stream_desc"),   ConfigValue(stream_desc->toPlainText()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "stream_genre"),  ConfigValue(stream_genre->text()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "stream_public"), ConfigValue(stream_public->isChecked()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "bitrate"),       ConfigValue(comboBoxEncodingBitrate->currentText()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "format"),        ConfigValue(comboBoxEncodingFormat->currentText()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "channels"),      ConfigValue(comboBoxEncodingChannels->currentText()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY,"enable_metadata"),ConfigValue(enableCustomMetadata->isChecked()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "custom_artist"), ConfigValue(custom_artist->text()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "custom_title"),  ConfigValue(custom_title->text()));

    //Tell the EngineShoutcast object to update with these values by toggling this control object.
    m_pUpdateShoutcastFromPrefs->slotSet(1.0f);
}
