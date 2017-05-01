#include <QtDebug>

#include "broadcast/defs_broadcast.h"
#include "control/controlproxy.h"
#include "defs_urls.h"
#include "preferences/dialog/dlgprefbroadcast.h"
#include "encoder/encodersettings.h"

DlgPrefBroadcast::DlgPrefBroadcast(QWidget *parent, UserSettingsPointer _config)
        : DlgPreferencePage(parent),
          m_settings(_config) {
    setupUi(this);

    m_pBroadcastEnabled = new ControlProxy(
            BROADCAST_PREF_KEY, "enabled", this);
    m_pBroadcastEnabled->connectValueChanged(
            SLOT(broadcastEnabledChanged(double)));


    // Enable live broadcasting checkbox
    enableLiveBroadcasting->setChecked(
            m_pBroadcastEnabled->toBool());

    //Server type combobox
    comboBoxServerType->addItem(tr("Icecast 2"), BROADCAST_SERVER_ICECAST2);
    comboBoxServerType->addItem(tr("Shoutcast 1"), BROADCAST_SERVER_SHOUTCAST);
    comboBoxServerType->addItem(tr("Icecast 1"), BROADCAST_SERVER_ICECAST1);

    int tmp_index = comboBoxServerType->findData(m_settings.getServertype());
    if (tmp_index < 0) { // Set default if invalid.
        tmp_index = 0;
    }
    comboBoxServerType->setCurrentIndex(tmp_index);

    // Mountpoint
    mountpoint->setText(m_settings.getMountpoint());

    // Host
    host->setText(m_settings.getHost());

    // Port
    QString portString = QString::number(m_settings.getPort());
    port->setText(portString);

    // Login
    login->setText(m_settings.getLogin());

    // Password
    password->setText(m_settings.getPassword());


    // Enable automatic reconnect
    bool enableReconnect = m_settings.getEnableReconnect();
    checkBoxEnableReconnect->setChecked(enableReconnect);
    widgetReconnectControls->setEnabled(enableReconnect);
    connect(checkBoxEnableReconnect, SIGNAL(stateChanged(int)),
            this, SLOT(checkBoxEnableReconnectChanged(int)));


    // Wait until first attempt
    spinBoxFirstDelay->setValue(m_settings.getReconnectFirstDelay());

    // Retry Delay
    spinBoxReconnectPeriod->setValue(m_settings.getReconnectPeriod());

    // Use Maximum Retries
    bool limitConnects = m_settings.getLimitReconnects();
    checkBoxLimitReconnects->setChecked(
            limitConnects);
    spinBoxMaximumRetries->setEnabled(limitConnects);
    connect(checkBoxLimitReconnects, SIGNAL(stateChanged(int)),
            this, SLOT(checkBoxLimitReconnectsChanged(int)));

    // Maximum Retries
    spinBoxMaximumRetries->setValue(m_settings.getMaximumRetries());


    // Stream "public" checkbox
    stream_public->setChecked(m_settings.getStreamPublic());

    // Stream name
    stream_name->setText(m_settings.getStreamName());

    // Stream website
    stream_website->setText(m_settings.getStreamWebsite());

    // Stream description
    stream_desc->setText(m_settings.getStreamDesc());

    // Stream genre
    stream_genre->setText(m_settings.getStreamGenre());

    // Encoding bitrate combobox
    QString kbps_pattern = QString("%1 kbps");
    QList<int> valid_kpbs;
    valid_kpbs << BROADCAST_BITRATE_320KBPS
               << BROADCAST_BITRATE_256KBPS
               << BROADCAST_BITRATE_224KBPS
               << BROADCAST_BITRATE_192KBPS
               << BROADCAST_BITRATE_160KBPS
               << BROADCAST_BITRATE_128KBPS
               << BROADCAST_BITRATE_112KBPS
               << BROADCAST_BITRATE_96KBPS
               << BROADCAST_BITRATE_80KBPS
               << BROADCAST_BITRATE_64KBPS
               << BROADCAST_BITRATE_48KBPS
               << BROADCAST_BITRATE_32KBPS;
    foreach (int kbps, valid_kpbs) {
        comboBoxEncodingBitrate->addItem(
                kbps_pattern.arg(QString::number(kbps)), kbps);
    }

    tmp_index = comboBoxEncodingBitrate->findData(m_settings.getBitrate());
    if (tmp_index < 0) {
        tmp_index = comboBoxEncodingBitrate->findData(BROADCAST_BITRATE_128KBPS);
    }
    comboBoxEncodingBitrate->setCurrentIndex(tmp_index < 0 ? 0 : tmp_index);

    // Encoding format combobox
    comboBoxEncodingFormat->addItem(tr("MP3"), BROADCAST_FORMAT_MP3);
    comboBoxEncodingFormat->addItem(tr("Ogg Vorbis"), BROADCAST_FORMAT_OV);
    tmp_index = comboBoxEncodingFormat->findData(m_settings.getFormat());
    if (tmp_index < 0) {
        // Set default of MP3 if invalid.
        tmp_index = 0;
    }
    comboBoxEncodingFormat->setCurrentIndex(tmp_index);

    // Encoding channels combobox
    comboBoxEncodingChannels->addItem(tr("Automatic"),
        static_cast<int>(EncoderSettings::ChannelMode::AUTOMATIC));
    comboBoxEncodingChannels->addItem(tr("Mono"),
        static_cast<int>(EncoderSettings::ChannelMode::MONO));
    comboBoxEncodingChannels->addItem(tr("Stereo"),
        static_cast<int>(EncoderSettings::ChannelMode::STEREO));
    tmp_index = comboBoxEncodingChannels->findData(m_settings.getChannels());
    if (tmp_index < 0) { // Set default to automatic if invalid.
        tmp_index = 0;
    }
    comboBoxEncodingChannels->setCurrentIndex(tmp_index);

    // Metadata format
    metadata_format->setText(m_settings.getMetadataFormat());

    // Static artist
    custom_artist->setText(m_settings.getCustomArtist());

    // Static title
    custom_title->setText(m_settings.getCustomTitle());

    // "Enable static artist and title" checkbox
    bool enableMetadata = m_settings.getEnableMetadata();
    enableCustomMetadata->setChecked(enableMetadata);
    custom_artist->setEnabled(enableMetadata);
    custom_title->setEnabled(enableMetadata);
    connect(enableCustomMetadata, SIGNAL(stateChanged(int)),
            this, SLOT(enableCustomMetadataChanged(int)));

    // "Enable UTF-8 metadata" checkbox
    // TODO(rryan): allow arbitrary codecs in the future?
    QString charset = m_settings.getMetadataCharset();
    enableUtf8Metadata->setChecked(charset == "UTF-8");

    // OGG "dynamicupdate" checkbox
    ogg_dynamicupdate->setChecked(m_settings.getOggDynamicUpdate());

    slotApply();
}

DlgPrefBroadcast::~DlgPrefBroadcast() {
}

void DlgPrefBroadcast::slotResetToDefaults() {
    // Make sure to keep these values in sync with the constructor.
    enableLiveBroadcasting->setChecked(false);
    comboBoxServerType->setCurrentIndex(0);
    mountpoint->setText(m_settings.getDefaultMountpoint());
    host->setText(m_settings.getDefaultHost());
    int iPort = m_settings.getDefaultPort();
    VERIFY_OR_DEBUG_ASSERT(iPort != 0 && iPort <= 0xffff) {
        port->setText(QString());
    } else {
        port->setText(QString::number(iPort));
    }
    login->setText(m_settings.getDefaultLogin());
    password->setText(m_settings.getDefaultPassword());

    checkBoxEnableReconnect->setChecked(m_settings.getDefaultEnableReconnect());
    widgetReconnectControls->setEnabled(true);
    spinBoxFirstDelay->setValue(m_settings.getDefaultReconnectFirstDelay());
    spinBoxReconnectPeriod->setValue(m_settings.getDefaultReconnectPeriod());
    checkBoxLimitReconnects->setChecked(m_settings.getDefaultLimitReconnects());
    spinBoxMaximumRetries->setValue(m_settings.getDefaultMaximumRetries());
    spinBoxMaximumRetries->setEnabled(true);
    stream_name->setText(m_settings.getDefaultStreamName());
    stream_website->setText(m_settings.getDefaultStreamWebsite());
    stream_desc->setText(m_settings.getDefaultStreamDesc());
    stream_genre->setText(m_settings.getDefaultStreamGenre());
    stream_public->setChecked(m_settings.getDefaultStreamPublic());
    ogg_dynamicupdate->setChecked(m_settings.getDefaultOggDynamicUpdate());
    comboBoxEncodingBitrate->setCurrentIndex(comboBoxEncodingBitrate->findData(
            m_settings.getDefaultBitrate()));
    comboBoxEncodingFormat->setCurrentIndex(0);
    comboBoxEncodingChannels->setCurrentIndex(0);
    enableUtf8Metadata->setChecked(false);
    enableCustomMetadata->setChecked(false);
    metadata_format->setText(m_settings.getDefaultMetadataFormat());
    custom_artist->setText(m_settings.getDefaultCustomArtist());
    custom_title->setText(m_settings.getDefaultCustomTitle());
    custom_artist->setEnabled(false);
    custom_title->setEnabled(false);
}

void DlgPrefBroadcast::slotUpdate() {
    enableLiveBroadcasting->setChecked(m_pBroadcastEnabled->toBool());

    // Don't let user modify information if
    // sending is enabled.
    if(m_pBroadcastEnabled->toBool()) {
        this->setEnabled(false);
    } else {
        this->setEnabled(true);
    }
}

void DlgPrefBroadcast::slotApply()
{
    m_pBroadcastEnabled->set(enableLiveBroadcasting->isChecked());

    // Don't let user modify information if
    // sending is enabled.
    if(m_pBroadcastEnabled->toBool()) {
        this->setEnabled(false);
    } else {
        this->setEnabled(true);
    }

    // Combo boxes, make sure to load their data not their display strings.
    m_settings.setServertype(comboBoxServerType->itemData(
            comboBoxServerType->currentIndex()).toString());
    m_settings.setBitrate(comboBoxEncodingBitrate->itemData(
            comboBoxEncodingBitrate->currentIndex()).toInt());
    m_settings.setFormat(comboBoxEncodingFormat->itemData(
            comboBoxEncodingFormat->currentIndex()).toString());
    m_settings.setChannels(comboBoxEncodingChannels->itemData(
            comboBoxEncodingChannels->currentIndex()).toInt());

    mountpoint->setText(mountpoint->text().trimmed());
    m_settings.setMountPoint(mountpoint->text());
    m_settings.setHost(host->text());
    m_settings.setPort(port->text().toInt());
    m_settings.setLogin(login->text());
    m_settings.setPassword(password->text());
    m_settings.setEnableReconnect(checkBoxEnableReconnect->isChecked());
    m_settings.setReconnectFirstDelay(spinBoxFirstDelay->value());
    m_settings.setReconnectPeriod(spinBoxReconnectPeriod->value());
    m_settings.setLimitReconnects(checkBoxLimitReconnects->isChecked());
    m_settings.setMaximumRetries(spinBoxMaximumRetries->value());
    m_settings.setStreamName(stream_name->text());
    m_settings.setStreamWebsite(stream_website->text());
    m_settings.setStreamDesc(stream_desc->toPlainText());
    m_settings.setStreamGenre(stream_genre->text());
    m_settings.setStreamPublic(stream_public->isChecked());
    m_settings.setOggDynamicUpdate(ogg_dynamicupdate->isChecked());

    QString charset = "";
    if (enableUtf8Metadata->isChecked()) {
        charset = "UTF-8";
    }
    QString current_charset = m_settings.getMetadataCharset();

    // Only allow setting the config value if the current value is either empty
    // or "UTF-8". This way users can customize the charset to something else by
    // setting the value in their mixxx.cfg. Not sure if this will be useful but
    // it's good to leave the option open.
    if (current_charset.length() == 0 || current_charset == "UTF-8") {
        m_settings.setMetadataCharset(charset);
    }

    m_settings.setEnableMetadata(enableCustomMetadata->isChecked());
    m_settings.setCustomArtist(custom_artist->text());
    m_settings.setCustomTitle(custom_title->text());
    m_settings.setMetadataFormat(metadata_format->text());
}

void DlgPrefBroadcast::broadcastEnabledChanged(double value) {
    qDebug() << "DlgPrefBroadcast::broadcastEnabledChanged()" << value;
    bool enabled = value == 1.0; // 0 and 2 are disabled
    this->setEnabled(!enabled);
    enableLiveBroadcasting->setChecked(enabled);

}

void DlgPrefBroadcast::checkBoxEnableReconnectChanged(int value) {
    widgetReconnectControls->setEnabled(value);
}

void DlgPrefBroadcast::checkBoxLimitReconnectsChanged(int value) {
    spinBoxMaximumRetries->setEnabled(value);
}

void DlgPrefBroadcast::enableCustomMetadataChanged(int value) {
    custom_artist->setEnabled(value);
    custom_title->setEnabled(value);
}
