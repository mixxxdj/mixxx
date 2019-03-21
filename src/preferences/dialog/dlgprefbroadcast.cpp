#include <QtDebug>
#include <QInputDialog>
#include <QMetaMethod>
#include <QMetaProperty>
#include <QAbstractItemDelegate>
#include <QMessageBox>
#include <QHeaderView>

// shout.h checks for WIN32 to see if we are on Windows
#ifdef WIN64
#define WIN32
#endif
// this is needed to define SHOUT_META_* macros used in version guard
#include <shout/shout.h>
#ifdef WIN64
#undef WIN32
#endif

#include "broadcast/defs_broadcast.h"
#include "control/controlproxy.h"
#include "defs_urls.h"
#include "preferences/dialog/dlgprefbroadcast.h"
#include "encoder/encodersettings.h"
#include "util/logger.h"

namespace {
const char* kSettingsGroupHeader = "Settings for %1";
const int kColumnEnabled = 0;
const int kColumnName = 1;
const int kColumnStatus = 2;
const mixxx::Logger kLogger("DlgPrefBroadcast");
}

DlgPrefBroadcast::DlgPrefBroadcast(QWidget *parent,
                                   BroadcastSettingsPointer pBroadcastSettings)
        : DlgPreferencePage(parent),
          m_pBroadcastSettings(pBroadcastSettings),
          m_pSettingsModel(new BroadcastSettingsModel()),
          m_pProfileListSelection(nullptr) {
    setupUi(this);

#ifndef __QTKEYCHAIN__
    // If secure storage is disabled, hide the radio buttons
    // and force the value to false.
    rbPasswordKeychain->setChecked(false);
    rbPasswordCleartext->setChecked(true);

    rbPasswordKeychain->setEnabled(false);
    groupPasswordStorage->setVisible(false);
#endif

#ifndef SHOUT_META_IRC
    stream_IRC_label->setVisible(false);
    stream_IRC->setVisible(false);
#endif

#ifndef SHOUT_META_AIM
    stream_AIM_label->setVisible(false);
    stream_AIM->setVisible(false);
#endif

#ifndef SHOUT_META_ICQ
    stream_ICQ_label->setVisible(false);
    stream_ICQ->setVisible(false);
#endif

    connect(connectionList->horizontalHeader(), SIGNAL(sectionResized(int, int, int)),
            this, SLOT(onSectionResized()));

    updateModel();
    connectionList->setModel(m_pSettingsModel);

    connect(connectionList->selectionModel(),
            SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)),
            this,
            SLOT(connectionListItemSelected(const QModelIndex&)));
    connect(btnRemoveConnection, SIGNAL(clicked(bool)),
            this, SLOT(btnRemoveConnectionClicked()));
    connect(btnRenameConnection, SIGNAL(clicked(bool)),
            this, SLOT(btnRenameConnectionClicked()));
    connect(btnCreateConnection, SIGNAL(clicked(bool)),
            this, SLOT(btnCreateConnectionClicked()));

    connect(btnDisconnectAll, SIGNAL(clicked(bool)),
            this, SLOT(btnDisconnectAllClicked()));

    // Highlight first row
    connectionList->selectRow(0);

    m_pBroadcastEnabled = new ControlProxy(
            BROADCAST_PREF_KEY, "enabled", this);
    m_pBroadcastEnabled->connectValueChanged(this,
            &DlgPrefBroadcast::broadcastEnabledChanged);

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
#ifdef __OPUS__
     comboBoxEncodingFormat->addItem(tr("Opus"), BROADCAST_FORMAT_OPUS);
#endif

     // Encoding channels combobox
     comboBoxEncodingChannels->addItem(tr("Automatic"),
             static_cast<int>(EncoderSettings::ChannelMode::AUTOMATIC));
     comboBoxEncodingChannels->addItem(tr("Mono"),
             static_cast<int>(EncoderSettings::ChannelMode::MONO));
     comboBoxEncodingChannels->addItem(tr("Stereo"),
             static_cast<int>(EncoderSettings::ChannelMode::STEREO));

     connect(checkBoxEnableReconnect, SIGNAL(stateChanged(int)),
             this, SLOT(checkBoxEnableReconnectChanged(int)));

     connect(checkBoxLimitReconnects, SIGNAL(stateChanged(int)),
             this, SLOT(checkBoxLimitReconnectsChanged(int)));

     connect(enableCustomMetadata, SIGNAL(stateChanged(int)),
             this, SLOT(enableCustomMetadataChanged(int)));
}

DlgPrefBroadcast::~DlgPrefBroadcast() {
    delete m_pSettingsModel;
}

void DlgPrefBroadcast::slotResetToDefaults() {
}

void DlgPrefBroadcast::slotUpdate() {
    updateModel();
    connectOnApply->setChecked(false);

    // Force select an item to have the current selection
    // set to a profile pointer belonging to the model
    selectConnectionRow(0);

    // Don't let user modify information if
    // sending is enabled.
    bool enabled = m_pBroadcastEnabled->toBool();
    groupBoxProfileSettings->setEnabled(!enabled);
    btnCreateConnection->setEnabled(!enabled);
    btnRemoveConnection->setEnabled(!enabled);
    btnRenameConnection->setEnabled(!enabled);
    connectOnApply->setEnabled(!enabled);

    btnDisconnectAll->setEnabled(enabled);
}

void DlgPrefBroadcast::applyModel() {
    if (m_pProfileListSelection) {
        setValuesToProfile(m_pProfileListSelection);
    }
    m_pBroadcastSettings->applyModel(m_pSettingsModel);
    updateModel();
}

void DlgPrefBroadcast::slotApply() {
    // Make sure the currently selected connection
    // gets saved as expected
    setValuesToProfile(m_pProfileListSelection);

    // Check for Icecast connections with identical mountpoints on the same host
    QMap<QString, QString> mountpoints;
    for(BroadcastProfilePtr profile : m_pSettingsModel->profiles()) {
        if (profile->getServertype() != BROADCAST_SERVER_ICECAST2) {
            continue;
        }

        QString profileName = profile->getProfileName();
        QString profileMountpoint = profile->getMountpoint();

        for (auto it = mountpoints.constBegin(); it != mountpoints.constEnd(); ++it) {
            if (it.value() == profileMountpoint) {
                QString profileNameWithSameMountpoint = it.key();
                BroadcastProfilePtr profileWithSameMountpoint =
                        m_pSettingsModel->getProfileByName(profileNameWithSameMountpoint);

                if (!profileWithSameMountpoint.isNull()
                    && profileWithSameMountpoint->getHost().toLower()
                    == profile->getHost().toLower()
                    && profileWithSameMountpoint->getPort()
                    == profile->getPort() ) {
                    QMessageBox::warning(
                        this, tr("Action failed"),
                        tr("'%1' has the same Icecast mountpoint as '%2'.\n"
                           "Two source connections to the same server can't have the same mountpoint.")
                        .arg(profileName).arg(profileNameWithSameMountpoint));
                    return;
                }
            }
        }

        mountpoints.insert(profileName, profileMountpoint);
    }

    applyModel();
    bool broadcastingEnabled = m_pBroadcastEnabled->toBool();
    if (!broadcastingEnabled && connectOnApply->isChecked()) {
        m_pBroadcastEnabled->set(true);

        // Reset state of "Connect on Apply" checkbox
        connectOnApply->setChecked(false);
    }

    // Don't let user modify information if
    // sending is enabled.
    bool enabled = m_pBroadcastEnabled->toBool();
    groupBoxProfileSettings->setEnabled(!enabled);
    btnCreateConnection->setEnabled(!enabled);
    btnRemoveConnection->setEnabled(!enabled);
    btnRenameConnection->setEnabled(!enabled);
    connectOnApply->setEnabled(!enabled);

    btnDisconnectAll->setEnabled(enabled);
}

void DlgPrefBroadcast::broadcastEnabledChanged(double value) {
    kLogger.debug() << "broadcastEnabledChanged()" << value;
    bool enabled = (value == 1.0); // 0 and 2 are disabled

    groupBoxProfileSettings->setEnabled(!enabled);
    btnCreateConnection->setEnabled(!enabled);
    btnRemoveConnection->setEnabled(!enabled);
    btnRenameConnection->setEnabled(!enabled);
    connectOnApply->setEnabled(!enabled);

    btnDisconnectAll->setEnabled(enabled);
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

void DlgPrefBroadcast::btnCreateConnectionClicked() {
    if (m_pSettingsModel->rowCount() >= BROADCAST_MAX_CONNECTIONS) {
        QMessageBox::warning(this, tr("Action failed"),
                tr("You can't create more than %1 source connections.")
                .arg(BROADCAST_MAX_CONNECTIONS));
        return;
    }

    int profileNumber = m_pSettingsModel->rowCount();

    // Generate a new profile name based on the current profile count.
    // Try the number above if the generated name is already taken.
    BroadcastProfilePtr existingProfile(nullptr);
    QString newName;
    do {
        profileNumber++;
        newName = tr("Source connection %1").arg(profileNumber);
        existingProfile = m_pSettingsModel->getProfileByName(newName);
    } while(!existingProfile.isNull());

    BroadcastProfilePtr newProfile(new BroadcastProfile(newName));
    if (m_pProfileListSelection) {
        m_pProfileListSelection->copyValuesTo(newProfile);
    }
    m_pSettingsModel->addProfileToModel(newProfile);
    selectConnectionRowByName(newProfile->getProfileName());
}

void DlgPrefBroadcast::connectionListItemSelected(const QModelIndex& selected) {
    setValuesToProfile(m_pProfileListSelection);

    QString selectedName = m_pSettingsModel->data(selected,
            Qt::DisplayRole).toString();
    BroadcastProfilePtr profile =
            m_pSettingsModel->getProfileByName(selectedName);
    if (profile) {
        getValuesFromProfile(profile);
        m_pProfileListSelection = profile;
    }
}

void DlgPrefBroadcast::updateModel() {
    // Resetting the model will clear the current list selection
    // so store the name of the current selection before resetting
    QString selected("");
    if (m_pProfileListSelection) {
        selected = m_pProfileListSelection->getProfileName();
    }

    m_pSettingsModel->resetFromSettings(m_pBroadcastSettings);
    if (!selected.isEmpty()) {
        // Restore previous selection with the name fetched before
        selectConnectionRowByName(selected);
    }
}

void DlgPrefBroadcast::selectConnectionRow(int row) {
    if (row < 0) {
        row = 0;
    }

    const int maxRow = m_pSettingsModel->rowCount() - 1;
    if (row > maxRow) {
        row = maxRow;
    }

    connectionList->selectRow(row);

    // QTableView::selectRow updates the UI but doesn't trigger
    // currentRowChanged in the selection model object, so
    // we must do it manually
    QModelIndex newSelection = m_pSettingsModel->index(row, kColumnName);
    connectionListItemSelected(newSelection);
}

void DlgPrefBroadcast::selectConnectionRowByName(QString rowName) {
    int row = -1;
    for (int i = 0; i < m_pSettingsModel->rowCount(); i++) {
        QModelIndex index = m_pSettingsModel->index(i, kColumnName);
        QVariant value = m_pSettingsModel->data(index, Qt::DisplayRole);
        if (value.toString() == rowName) {
            row = i;
            break;
        }
    }

    if (row > -1) {
        selectConnectionRow(row);
    }
}

void DlgPrefBroadcast::getValuesFromProfile(BroadcastProfilePtr profile) {
    if (!profile) {
        return;
    }

    // Set groupbox header
    QString headerText =
            QString(tr(kSettingsGroupHeader))
            .arg(profile->getProfileName());
    groupBoxProfileSettings->setTitle(headerText);

    rbPasswordCleartext->setChecked(!profile->secureCredentialStorage());
    rbPasswordKeychain->setChecked(profile->secureCredentialStorage());

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

    // Stream IRC
    stream_IRC->setText(profile->getStreamIRC());

    // Stream AIM
    stream_AIM->setText(profile->getStreamAIM());

    // Stream ICQ
    stream_ICQ->setText(profile->getStreamICQ());

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
}

void DlgPrefBroadcast::setValuesToProfile(BroadcastProfilePtr profile) {
    if (!profile)
        return;

    profile->setSecureCredentialStorage(rbPasswordKeychain->isChecked());

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
    profile->setStreamIRC(stream_IRC->text());
    profile->setStreamAIM(stream_AIM->text());
    profile->setStreamICQ(stream_ICQ->text());
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
}

void DlgPrefBroadcast::btnRemoveConnectionClicked() {
    if (m_pSettingsModel->rowCount() < 2) {
        QMessageBox::information(this, tr("Action failed"),
                tr("At least one source connection is required."));
        return;
    }

    if (m_pProfileListSelection) {
        QString profileName = m_pProfileListSelection->getProfileName();
        auto response = QMessageBox::question(this, tr("Confirmation required"),
                    tr("Are you sure you want to delete '%1'?")
                    .arg(profileName), QMessageBox::Yes, QMessageBox::No);

        if (response == QMessageBox::Yes) {
            m_pSettingsModel->deleteProfileFromModel(m_pProfileListSelection);
        }
    }
}

void DlgPrefBroadcast::btnRenameConnectionClicked() {
    if (m_pProfileListSelection) {
        QString profileName = m_pProfileListSelection->getProfileName();

        bool ok = false;
        QString newName =
                QInputDialog::getText(this, tr("Renaming '%1'").arg(profileName),
                        tr("New name for '%1':").arg(profileName),
                        QLineEdit::Normal, profileName, &ok);
        if (ok && newName != profileName) {
            BroadcastProfilePtr existingProfile = m_pSettingsModel->getProfileByName(newName);
            if (!existingProfile) {
                // Requested name not used already
                m_pProfileListSelection->setProfileName(newName);
                getValuesFromProfile(m_pProfileListSelection);
            } else {
                // Requested name different from current name but already used
                QMessageBox::warning(this, tr("Action failed"),
                        tr("Can't rename '%1' to '%2': name already in use")
                        .arg(profileName).arg(newName));
            }
        }
    }
}

void DlgPrefBroadcast::btnDisconnectAllClicked() {
    auto response = QMessageBox::question(this,
            tr("Confirmation required"),
            tr("Are you sure you want to disconnect every active source connection?"),
            QMessageBox::Yes, QMessageBox::No);

    if (response == QMessageBox::Yes) {
        m_pBroadcastEnabled->set(false);
        broadcastEnabledChanged(0.0);
    }
}

void DlgPrefBroadcast::onSectionResized() {
    float width = (float)connectionList->width();

    sender()->blockSignals(true);
    connectionList->setColumnWidth(kColumnEnabled, 100);
    connectionList->setColumnWidth(kColumnName, width * 0.65);
    // The last column is automatically resized to fill
    // the remaining width, thanks to stretchLastSection set to true.
    sender()->blockSignals(false);
}
