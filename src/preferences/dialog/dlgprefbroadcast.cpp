#include <QtDebug>
#include <QInputDialog>
#include <QMetaMethod>
#include <QMetaProperty>

#include "broadcast/defs_broadcast.h"
#include "control/controlproxy.h"
#include "defs_urls.h"
#include "preferences/dialog/dlgprefbroadcast.h"
#include "encoder/encodersettings.h"

namespace {
const char* kSettingsGroupHeader = "Settings for profile '%1'";
const char* kUnsavedChangesWarning =
        "Profile '%1' has unsaved changes. Do you want to save them?";
}

DlgPrefBroadcast::DlgPrefBroadcast(QWidget *parent,
                                   UserSettingsPointer _config,
                                   BroadcastSettingsPointer pBroadcastSettings)
        : DlgPreferencePage(parent),
          m_pBroadcastSettings(pBroadcastSettings),
          m_pProfileListSelection(nullptr),
          m_valuesChanged(false) {
    setupUi(this);

    // Should be safe to get the underlying pointer
    profileList->setModel(m_pBroadcastSettings.data());

    connect(btnCreateProfile, SIGNAL(clicked(bool)),
            this, SLOT(btnCreateConnectionClicked(bool)));
    connect(profileList, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(profileListItemSelected(const QModelIndex&)));

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

     // Encoding format combobox
     comboBoxEncodingFormat->addItem(tr("MP3"), BROADCAST_FORMAT_MP3);
     comboBoxEncodingFormat->addItem(tr("Ogg Vorbis"), BROADCAST_FORMAT_OV);

     // Encoding channels combobox
     comboBoxEncodingChannels->addItem(tr("Automatic"),
             static_cast<int>(EncoderSettings::ChannelMode::AUTOMATIC));
     comboBoxEncodingChannels->addItem(tr("Mono"),
             static_cast<int>(EncoderSettings::ChannelMode::MONO));
     comboBoxEncodingChannels->addItem(tr("Stereo"),
             static_cast<int>(EncoderSettings::ChannelMode::STEREO));

     BroadcastProfilePtr pProfile = m_pBroadcastSettings->profileAt(0);
     getValuesFromProfile(pProfile);

     connect(checkBoxEnableReconnect, SIGNAL(stateChanged(int)),
             this, SLOT(checkBoxEnableReconnectChanged(int)));

     connect(checkBoxLimitReconnects, SIGNAL(stateChanged(int)),
             this, SLOT(checkBoxLimitReconnectsChanged(int)));

     connect(enableCustomMetadata, SIGNAL(stateChanged(int)),
             this, SLOT(enableCustomMetadataChanged(int)));

     // Connect each user input of each groupbox in the values groupbox
     // to a local slot which purpose is to determine if changes have been made
     // to the values. Used when selecting another profile without saving
     // the currently selected one.
     enableValueSignals(true);
}

DlgPrefBroadcast::~DlgPrefBroadcast() {
}

void DlgPrefBroadcast::slotResetToDefaults() {
    BroadcastProfile dProfile("dontsave");

    // Make sure to keep these values in sync with the constructor.
    enableLiveBroadcasting->setChecked(false);
    comboBoxServerType->setCurrentIndex(0);
    mountpoint->setText(dProfile.getMountpoint());
    host->setText(dProfile.getHost());
    int iPort = dProfile.getPort();
    VERIFY_OR_DEBUG_ASSERT(iPort != 0 && iPort <= 0xffff) {
        port->setText(QString());
    } else {
        port->setText(QString::number(iPort));
    }
    login->setText(dProfile.getLogin());
    password->setText(dProfile.getPassword());

    checkBoxEnableReconnect->setChecked(dProfile.getEnableReconnect());
    widgetReconnectControls->setEnabled(true);
    spinBoxFirstDelay->setValue(dProfile.getReconnectFirstDelay());
    spinBoxReconnectPeriod->setValue(dProfile.getReconnectPeriod());
    checkBoxLimitReconnects->setChecked(dProfile.getLimitReconnects());
    spinBoxMaximumRetries->setValue(dProfile.getMaximumRetries());
    spinBoxMaximumRetries->setEnabled(true);
    stream_name->setText(dProfile.getStreamName());
    stream_website->setText(dProfile.getStreamWebsite());
    stream_desc->setText(dProfile.getStreamDesc());
    stream_genre->setText(dProfile.getStreamGenre());
    stream_public->setChecked(dProfile.getStreamPublic());
    ogg_dynamicupdate->setChecked(dProfile.getOggDynamicUpdate());
    comboBoxEncodingBitrate->setCurrentIndex(comboBoxEncodingBitrate->findData(
            dProfile.getBitrate()));
    comboBoxEncodingFormat->setCurrentIndex(0);
    comboBoxEncodingChannels->setCurrentIndex(0);
    enableUtf8Metadata->setChecked(false);
    enableCustomMetadata->setChecked(false);
    metadata_format->setText(dProfile.getMetadataFormat());
    custom_artist->setText(dProfile.getCustomArtist());
    custom_title->setText(dProfile.getCustomTitle());
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

    // TODO(Palakis) : keep a local deep copy of the profiles list to
    // apply settings to profiles only when clicking "Apply"
    // Here would be a call to a method of BroadcastSettings that syncs
    // another list with its internal list

    if(m_pProfileListSelection) {
        setValuesToProfile(m_pProfileListSelection);
    }
    m_pBroadcastSettings->saveAll();
    m_valuesChanged = false;
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

void DlgPrefBroadcast::btnCreateConnectionClicked(bool enabled) {
    // TODO(Palakis): Import code from branch broadcast-profile
    bool ok = false;
    QString profileName = QInputDialog::getText(this,
            tr("Create profile"), tr("Profile name:"), QLineEdit::Normal,
            tr("Default profile"), &ok);

    if(ok) {
        BroadcastProfilePtr newProfile =
                    m_pBroadcastSettings->createProfile(profileName);
        if(!newProfile) {
            QMessageBox::warning(this, tr("Profile already exists"),
                                    tr("An Untitled Profile already exists"));
        }
    }
}

void DlgPrefBroadcast::profileListItemSelected(const QModelIndex& index) {
    // TODO(Palakis): reuse this to ask for unsaved changes on apply
    /*if(m_pProfileListSelection) {
        QString title = QObject::tr("Unsaved changes");
        QString msg =
                QString(kUnsavedChangesWarning)
                .arg(m_pProfileListSelection->getProfileName());

        QMessageBox::StandardButton reply =
                QMessageBox::question(this, title, msg,
                        QMessageBox::Yes | QMessageBox::No);
        if(reply == QMessageBox::Yes) {
            // TODO(Palakis) : apply settings to profile in internal list copy
            m_pBroadcastSettings->saveProfile(m_pProfileListSelection);
        }
    }*/
    setValuesToProfile(m_pProfileListSelection);

    QString selectedName = m_pBroadcastSettings->data(index,
            Qt::DisplayRole).toString();
    BroadcastProfilePtr profile =
            m_pBroadcastSettings->getProfileByName(selectedName);
    if(profile) {
        getValuesFromProfile(profile);
        m_pProfileListSelection = profile;
    }
}

void DlgPrefBroadcast::getValuesFromProfile(BroadcastProfilePtr profile) {
    if(!profile)
        return;

    enableValueSignals(false);

    // Set groupbox header
    QString headerText =
            QString(QObject::tr(kSettingsGroupHeader))
            .arg(profile->getProfileName());
    groupBoxProfileSettings->setTitle(headerText);

    // Server type combo list
    int tmp_index = comboBoxServerType->findData(profile->getServertype());
    if (tmp_index < 0) { // Set default if invalid.
        tmp_index = 0;
    }
    comboBoxServerType->setCurrentIndex(tmp_index);

    // Mountpoint
    mountpoint->setText(profile->getMountpoint());

    // Host
    host->setText(profile->getHost());

    // Port
    QString portString = QString::number(profile->getPort());
    port->setText(portString);

    // Login
    login->setText(profile->getLogin());

    // Password
    password->setText(profile->getPassword());

    // Enable automatic reconnect
    bool enableReconnect = profile->getEnableReconnect();
    checkBoxEnableReconnect->setChecked(enableReconnect);
    widgetReconnectControls->setEnabled(enableReconnect);

    // Wait until first attempt
    spinBoxFirstDelay->setValue(profile->getReconnectFirstDelay());

    // Retry Delay
    spinBoxReconnectPeriod->setValue(profile->getReconnectPeriod());

    // Use Maximum Retries
    bool limitConnects = profile->getLimitReconnects();
    checkBoxLimitReconnects->setChecked(
            limitConnects);
    spinBoxMaximumRetries->setEnabled(limitConnects);

    // Maximum Retries
    spinBoxMaximumRetries->setValue(profile->getMaximumRetries());

    // Stream "public" checkbox
    stream_public->setChecked(profile->getStreamPublic());

    // Stream name
    stream_name->setText(profile->getStreamName());

    // Stream website
    stream_website->setText(profile->getStreamWebsite());

    // Stream description
    stream_desc->setText(profile->getStreamDesc());

    // Stream genre
    stream_genre->setText(profile->getStreamGenre());

    // Encoding bitrate combobox
    tmp_index = comboBoxEncodingBitrate->findData(profile->getBitrate());
    if (tmp_index < 0) {
        tmp_index = comboBoxEncodingBitrate->findData(BROADCAST_BITRATE_128KBPS);
    }
    comboBoxEncodingBitrate->setCurrentIndex(tmp_index < 0 ? 0 : tmp_index);

    // Encoding format combobox
    tmp_index = comboBoxEncodingFormat->findData(profile->getFormat());
    if (tmp_index < 0) {
        // Set default of MP3 if invalid.
        tmp_index = 0;
    }
    comboBoxEncodingFormat->setCurrentIndex(tmp_index);

    // Encoding channels combobox
    tmp_index = comboBoxEncodingChannels->findData(profile->getChannels());
    if (tmp_index < 0) { // Set default to automatic if invalid.
        tmp_index = 0;
    }
    comboBoxEncodingChannels->setCurrentIndex(tmp_index);

    // Metadata format
    metadata_format->setText(profile->getMetadataFormat());

    // Static artist
    custom_artist->setText(profile->getCustomArtist());

    // Static title
    custom_title->setText(profile->getCustomTitle());

    // "Enable static artist and title" checkbox
    bool enableMetadata = profile->getEnableMetadata();
    enableCustomMetadata->setChecked(enableMetadata);
    custom_artist->setEnabled(enableMetadata);
    custom_title->setEnabled(enableMetadata);

    // "Enable UTF-8 metadata" checkbox
    // TODO(rryan): allow arbitrary codecs in the future?
    QString charset = profile->getMetadataCharset();
    enableUtf8Metadata->setChecked(charset == "UTF-8");

    // OGG "dynamicupdate" checkbox
    ogg_dynamicupdate->setChecked(profile->getOggDynamicUpdate());

    m_valuesChanged = false;
    enableValueSignals(true);
}

void DlgPrefBroadcast::setValuesToProfile(BroadcastProfilePtr profile) {
    if(!profile)
        return;

    // Combo boxes, make sure to load their data not their display strings.
    profile->setServertype(comboBoxServerType->itemData(
            comboBoxServerType->currentIndex()).toString());
    profile->setBitrate(comboBoxEncodingBitrate->itemData(
            comboBoxEncodingBitrate->currentIndex()).toInt());
    profile->setFormat(comboBoxEncodingFormat->itemData(
            comboBoxEncodingFormat->currentIndex()).toString());
    profile->setChannels(comboBoxEncodingChannels->itemData(
            comboBoxEncodingChannels->currentIndex()).toInt());

    mountpoint->setText(mountpoint->text().trimmed());
    profile->setMountPoint(mountpoint->text());
    profile->setHost(host->text());
    profile->setPort(port->text().toInt());
    profile->setLogin(login->text());
    profile->setPassword(password->text());
    profile->setEnableReconnect(checkBoxEnableReconnect->isChecked());
    profile->setReconnectFirstDelay(spinBoxFirstDelay->value());
    profile->setReconnectPeriod(spinBoxReconnectPeriod->value());
    profile->setLimitReconnects(checkBoxLimitReconnects->isChecked());
    profile->setMaximumRetries(spinBoxMaximumRetries->value());
    profile->setStreamName(stream_name->text());
    profile->setStreamWebsite(stream_website->text());
    profile->setStreamDesc(stream_desc->toPlainText());
    profile->setStreamGenre(stream_genre->text());
    profile->setStreamPublic(stream_public->isChecked());
    profile->setOggDynamicUpdate(ogg_dynamicupdate->isChecked());

    QString charset = "";
    if (enableUtf8Metadata->isChecked()) {
        charset = "UTF-8";
    }
    QString current_charset = profile->getMetadataCharset();

    // Only allow setting the config value if the current value is either empty
    // or "UTF-8". This way users can customize the charset to something else by
    // setting the value in their mixxx.cfg. Not sure if this will be useful but
    // it's good to leave the option open.
    if (current_charset.length() == 0 || current_charset == "UTF-8") {
        profile->setMetadataCharset(charset);
    }

    profile->setEnableMetadata(enableCustomMetadata->isChecked());
    profile->setCustomArtist(custom_artist->text());
    profile->setCustomTitle(custom_title->text());
    profile->setMetadataFormat(metadata_format->text());

    m_valuesChanged = false;
}

void DlgPrefBroadcast::formValueChanged() {
    m_valuesChanged = true;
}

void DlgPrefBroadcast::enableValueSignals(bool enable) {
    QMetaMethod valueChangedSlot = metaObject()->method(
                           metaObject()->indexOfSlot("formValueChanged()"));

    //kLogger.info() << QString("--- ---");

    QList<QGroupBox*> subGroups =
            groupBoxProfileSettings->findChildren<QGroupBox*>();
    for(QGroupBox* subGroup : subGroups) {
        ///kLogger.info() << QString("---");

        QList<QWidget*> childs = subGroup->findChildren<QWidget*>();
        for(QWidget* child : childs) {
            const QMetaObject* metaObj = child->metaObject();
            QMetaProperty userProp = metaObj->userProperty();

            ///kLogger.info() << QString(metaObj->className());

            if(userProp.isValid() && userProp.hasNotifySignal()) {
                if(enable)
                    connect(child, userProp.notifySignal(),
                            this, valueChangedSlot);
                else
                    disconnect(child, userProp.notifySignal(),
                               this, valueChangedSlot);
            }
        }
    }
}
