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

#include <QtDebug>

#include "defs_urls.h"
#include "preferences/dialog/dlgprefshoutcast.h"
#include "shoutcast/defs_shoutcast.h"
#include "controlobjectslave.h"

const char* kDefaultMetadataFormat = "$artist - $title";

DlgPrefShoutcast::DlgPrefShoutcast(QWidget *parent, UserSettingsPointer _config)
        : DlgPreferencePage(parent),
          m_pConfig(_config) {
    setupUi(this);

    m_pShoutcastEnabled = new ControlObjectSlave(
            SHOUTCAST_PREF_KEY, "enabled", this);
    m_pShoutcastEnabled->connectValueChanged(
            SLOT(shoutcastEnabledChanged(double)));


    // Enable live broadcasting checkbox
    enableLiveBroadcasting->setChecked(
            m_pShoutcastEnabled->toBool());

    //Server type combobox
    comboBoxServerType->addItem(tr("Icecast 2"), SHOUTCAST_SERVER_ICECAST2);
    comboBoxServerType->addItem(tr("Shoutcast 1"), SHOUTCAST_SERVER_SHOUTCAST);
    comboBoxServerType->addItem(tr("Icecast 1"), SHOUTCAST_SERVER_ICECAST1);

    int tmp_index = comboBoxServerType->findData(m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY,"servertype")));
    if (tmp_index < 0) //Set default if invalid.
        tmp_index = 0;
    comboBoxServerType->setCurrentIndex(tmp_index);

    // Mountpoint
    mountpoint->setText(m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY,"mountpoint")));

    // Host
    host->setText(m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY,"host")));

    // Port
    QString tmp_string = m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY,"port"));
    if (tmp_string.isEmpty())
        tmp_string = QString(SHOUTCAST_DEFAULT_PORT);
    port->setText(tmp_string);

    // Login
    login->setText(m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY,"login")));

    // Password
    password->setText(m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY,"password")));

    // Stream name
    stream_name->setText(m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY,"stream_name")));

    // Stream website
    tmp_string = m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY,"stream_website"));
    if (tmp_string.isEmpty())
        tmp_string = MIXXX_WEBSITE_URL;
    stream_website->setText(tmp_string);

    // Stream description
    tmp_string = m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY,"stream_desc"));
    if (tmp_string.isEmpty())
        tmp_string = tr("This stream is online for testing purposes!");
    stream_desc->setText(tmp_string);

    // Stream genre
    tmp_string = m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY,"stream_genre"));
    if (tmp_string.isEmpty())
        tmp_string = tr("Live Mix");
    stream_genre->setText(tmp_string);

    // Stream "public" checkbox
    stream_public->setChecked((bool)m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY,"stream_public")).toInt());

    // OGG "dynamicupdate" checkbox
    ogg_dynamicupdate->setChecked((bool)m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY,"ogg_dynamicupdate")).toInt());

    // Encoding bitrate combobox
    QString kbps_pattern = QString("%1 kbps");
    QList<int> valid_kpbs;
    valid_kpbs << SHOUTCAST_BITRATE_320KBPS
               << SHOUTCAST_BITRATE_256KBPS
               << SHOUTCAST_BITRATE_224KBPS
               << SHOUTCAST_BITRATE_192KBPS
               << SHOUTCAST_BITRATE_160KBPS
               << SHOUTCAST_BITRATE_128KBPS
               << SHOUTCAST_BITRATE_112KBPS
               << SHOUTCAST_BITRATE_96KBPS
               << SHOUTCAST_BITRATE_80KBPS
               << SHOUTCAST_BITRATE_64KBPS
               << SHOUTCAST_BITRATE_48KBPS
               << SHOUTCAST_BITRATE_32KBPS;
    foreach (int kbps, valid_kpbs) {
        comboBoxEncodingBitrate->addItem(
            kbps_pattern.arg(QString::number(kbps)), kbps);
    }

    tmp_index = comboBoxEncodingBitrate->findData(m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY, "bitrate")).toInt());
    if (tmp_index < 0) {
        tmp_index = comboBoxEncodingBitrate->findData(SHOUTCAST_BITRATE_128KBPS);
    }
    comboBoxEncodingBitrate->setCurrentIndex(tmp_index < 0 ? 0 : tmp_index);

    // Encoding format combobox
    comboBoxEncodingFormat->addItem(tr("MP3"), SHOUTCAST_FORMAT_MP3);
    comboBoxEncodingFormat->addItem(tr("Ogg Vorbis"), SHOUTCAST_FORMAT_OV);
    tmp_index = comboBoxEncodingFormat->findData(m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY, "format")));
    if (tmp_index < 0) {
        // Set default of MP3 if invalid.
        tmp_index = 0;
    }
    comboBoxEncodingFormat->setCurrentIndex(tmp_index);

    // Encoding channels combobox
    comboBoxEncodingChannels->addItem(tr("Stereo"), SHOUTCAST_CHANNELS_STEREO);
    tmp_index = comboBoxEncodingChannels->findData(m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY, "channels")));
    if (tmp_index < 0) //Set default to stereo if invalid.
        tmp_index = 0;
    comboBoxEncodingChannels->setCurrentIndex(tmp_index);

    // "Enable UTF-8 metadata" checkbox
    // TODO(rryan): allow arbitrary codecs in the future?
    QString charset = m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY, "metadata_charset"));
    enableUtf8Metadata->setChecked(charset == "UTF-8");

    // "Enable custom metadata" checkbox
    enableCustomMetadata->setChecked((bool)m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY,"enable_metadata")).toInt());

    //Custom artist
    custom_artist->setText(m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY,"custom_artist")));

    //Custom title
    custom_title->setText(m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY,"custom_title")));

    //Metadata format
    tmp_string = m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY,"metadata_format"));
    if (tmp_string.isEmpty())
        // No tr() here, see https://bugs.launchpad.net/mixxx/+bug/1419500
        tmp_string = kDefaultMetadataFormat;
    metadata_format->setText(tmp_string);

    slotApply();
}

DlgPrefShoutcast::~DlgPrefShoutcast() {
}

void DlgPrefShoutcast::slotResetToDefaults() {
    // Make sure to keep these values in sync with the constructor.
    enableLiveBroadcasting->setChecked(false);
    comboBoxServerType->setCurrentIndex(0);
    mountpoint->setText("");
    host->setText("");
    port->setText("8000");
    login->setText("");
    password->setText("");
    stream_name->setText("");
    stream_website->setText(MIXXX_WEBSITE_URL);
    stream_desc->setText(tr("This stream is online for testing purposes!"));
    stream_genre->setText(tr("Live Mix"));
    stream_public->setChecked(false);
    ogg_dynamicupdate->setChecked(false);
    comboBoxEncodingBitrate->setCurrentIndex(comboBoxEncodingBitrate->findData(
            SHOUTCAST_BITRATE_128KBPS));
    comboBoxEncodingFormat->setCurrentIndex(0);
    comboBoxEncodingChannels->setCurrentIndex(0);
    enableUtf8Metadata->setChecked(false);
    enableCustomMetadata->setChecked(false);
    // No tr() here, see https://bugs.launchpad.net/mixxx/+bug/1419500
    metadata_format->setText(kDefaultMetadataFormat);
    custom_artist->setText("");
    custom_title->setText("");
}

void DlgPrefShoutcast::slotUpdate() {
    enableLiveBroadcasting->setChecked(m_pShoutcastEnabled->toBool());

    // Don't let user modify information if
    // sending is enabled.
    if(m_pShoutcastEnabled->toBool()) {
        this->setEnabled(false);
    } else {
        this->setEnabled(true);
    }
}

void DlgPrefShoutcast::slotApply()
{
    m_pShoutcastEnabled->set(enableLiveBroadcasting->isChecked());

    // Don't let user modify information if
    // sending is enabled.
    if(m_pShoutcastEnabled->toBool()) {
        this->setEnabled(false);
    } else {
        this->setEnabled(true);
    }

    // Combo boxes, make sure to load their data not their display strings.
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "servertype"),
            ConfigValue(comboBoxServerType->itemData(
                            comboBoxServerType->currentIndex()).toString()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "bitrate"),
            ConfigValue(comboBoxEncodingBitrate->itemData(
                            comboBoxEncodingBitrate->currentIndex()).toString()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "format"),
            ConfigValue(comboBoxEncodingFormat->itemData(
                            comboBoxEncodingFormat->currentIndex()).toString()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "channels"),
            ConfigValue(comboBoxEncodingChannels->itemData(
                            comboBoxEncodingChannels->currentIndex()).toString()));

    mountpoint->setText(mountpoint->text().trimmed());
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "mountpoint"),
            ConfigValue(mountpoint->text()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "host"),
            ConfigValue(host->text()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "port"),
            ConfigValue(port->text()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "login"),
            ConfigValue(login->text()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "password"),
            ConfigValue(password->text()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "stream_name"),
            ConfigValue(stream_name->text()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "stream_website"),
            ConfigValue(stream_website->text()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "stream_desc"),
            ConfigValue(stream_desc->toPlainText()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "stream_genre"),
            ConfigValue(stream_genre->text()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "stream_public"),
            ConfigValue(stream_public->isChecked()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "ogg_dynamicupdate"),
            ConfigValue(ogg_dynamicupdate->isChecked()));

    QString charset = "";
    if (enableUtf8Metadata->isChecked()) {
        charset = "UTF-8";
    }
    QString current_charset = m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY, "metadata_charset"));

    // Only allow setting the config value if the current value is either empty
    // or "UTF-8". This way users can customize the charset to something else by
    // setting the value in their mixxx.cfg. Not sure if this will be useful but
    // it's good to leave the option open.
    if (current_charset.length() == 0 || current_charset == "UTF-8") {
        m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "metadata_charset"), ConfigValue(charset));
    }

    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "enable_metadata"),ConfigValue(enableCustomMetadata->isChecked()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "custom_artist"), ConfigValue(custom_artist->text()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "custom_title"),  ConfigValue(custom_title->text()));
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "metadata_format"), ConfigValue(metadata_format->text()));
}

void DlgPrefShoutcast::shoutcastEnabledChanged(double value) {
    qDebug() << "DlgPrefShoutcast::shoutcastEnabledChanged()" << value;
    bool enabled = value == 1.0; // 0 and 2 are disabled
    this->setEnabled(!enabled);
    enableLiveBroadcasting->setChecked(enabled);

}
