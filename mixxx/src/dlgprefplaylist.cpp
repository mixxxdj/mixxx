/***************************************************************************
                          dlgprefplaylist.cpp  -  description
                             -------------------
    begin                : Thu Apr 17 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "dlgprefplaylist.h"
#include "library/promotracksfeature.h"
#include "soundsourceproxy.h"
//#include "plugindownloader.h"
#include <QtCore>
#include <QtGui>

#define MIXXX_ADDONS_URL "http://www.mixxx.org/wiki/doku.php/add-ons"

DlgPrefPlaylist::DlgPrefPlaylist(QWidget * parent, ConfigObject<ConfigValue> * _config)
        :  QWidget(parent) {
    config = _config;
    setupUi(this);
    slotUpdate();
    checkbox_ID3_sync->setVisible(false);

    /*
    m_pPluginDownloader = new PluginDownloader(this);

    //Disable the M4A button if the plugin is present on disk.
    setupM4AButton();

    //Disable M4A Button after download completes successfully.
    connect(m_pPluginDownloader, SIGNAL(downloadFinished()),
            this, SLOT(slotM4ADownloadFinished()));

    connect(m_pPluginDownloader, SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(slotM4ADownloadProgress(qint64, qint64)));
    */

    // Connection
    connect(PushButtonBrowsePlaylist, SIGNAL(clicked()),       this,      SLOT(slotBrowseDir()));
    connect(LineEditSongfiles,        SIGNAL(returnPressed()), this,      SLOT(slotApply()));
    //connect(pushButtonM4A, SIGNAL(clicked()), this, SLOT(slotM4ACheck()));
    connect(pushButtonExtraPlugins, SIGNAL(clicked()), this, SLOT(slotExtraPlugins()));

    if (!PromoTracksFeature::isSupported(config))
    {
        groupBoxBundledSongs->hide();
    }

    // plugins are loaded in src/main.cpp way early in boot so this is safe
    // here, doesn't need done at every slotUpdate
    QStringList plugins(SoundSourceProxy::supportedFileExtensionsByPlugins());
    if (plugins.length() > 0) {
        pluginsLabel->setText(plugins.join(", "));
    }
}

DlgPrefPlaylist::~DlgPrefPlaylist()
{
}

void DlgPrefPlaylist::slotExtraPlugins()
{
    QDesktopServices::openUrl(QUrl(MIXXX_ADDONS_URL));
}

/*
void DlgPrefPlaylist::slotM4ADownloadProgress(qint64 bytesReceived,
                                              qint64 bytesTotal)
{
    pushButtonM4A->setText(QString("%1\%").arg(100*(float)bytesReceived/bytesTotal, 0, 'g', 1));
}
void DlgPrefPlaylist::slotM4ADownloadFinished()
{
    //Disable the button after the download is finished.
    //We force it to be disabled because on Linux, gdebi-gtk
    //needs to be finished running before we know whether or not
    //the plugin is actually installed. :(
    setupM4AButton(true);
}

void DlgPrefPlaylist::setupM4AButton(bool forceInstalled)
{
    //If the M4A plugin is present on disk, disable the button
    if (m_pPluginDownloader->checkForM4APlugin() || forceInstalled) {
        pushButtonM4A->setChecked(true);
        pushButtonM4A->setEnabled(false);
        pushButtonM4A->setText(tr("Installed"));
    }
}

void DlgPrefPlaylist::slotM4ACheck()
{
    qDebug() << "slotM4ACheck";

#ifdef __LINUX__
    QFile version("/proc/version");
    bool isUbuntu = true;
    if (version.open(QIODevice::ReadOnly))
    {
        QByteArray rawLine = version.readAll();
        QString versionString(rawLine);
        if (versionString.contains("Ubuntu", Qt::CaseInsensitive))
        {
            isUbuntu = true;
        }
    }
    else {
        isUbuntu = false;
    }

    if (!isUbuntu)
    {
        QMessageBox::information(this, tr("M4A Playback Plugin"),
                                tr("The M4A playback plugin is currently"
                                "unavailable for your Linux distribution."
                                "Please download and compile Mixxx from "
                                "source to enable M4A playback."));
    }

#endif

    if (!m_pPluginDownloader->checkForM4APlugin())
    {
        m_pPluginDownloader->downloadM4APlugin();
    }
}*/

void DlgPrefPlaylist::slotUpdate()
{
    // Song path
    LineEditSongfiles->setText(config->getValueString(ConfigKey("[Playlist]","Directory")));
    //Bundled songs stat tracking
    checkBoxPromoStats->setChecked((bool)config->getValueString(ConfigKey("[Promo]","StatTracking")).toInt());
    checkBox_library_scan->setChecked((bool)config->getValueString(ConfigKey("[Library]","RescanOnStartup")).toInt());
    checkbox_ID3_sync->setChecked((bool)config->getValueString(ConfigKey("[Library]","WriteAudioTags")).toInt());
    checkBox_use_relative_path->setChecked((bool)config->getValueString(ConfigKey("[Library]","UseRelativePathOnExport")).toInt());
    checkBox_show_rhythmbox->setChecked((bool)config->getValueString(ConfigKey("[Library]","ShowRhythmboxLibrary"),"1").toInt());
    checkBox_show_itunes->setChecked((bool)config->getValueString(ConfigKey("[Library]","ShowITunesLibrary"),"1").toInt());
    checkBox_show_traktor->setChecked((bool)config->getValueString(ConfigKey("[Library]","ShowTraktorLibrary"),"1").toInt());
}

void DlgPrefPlaylist::slotBrowseDir()
{
    QString fd = QFileDialog::getExistingDirectory(this, tr("Choose music library directory"),
                                                   config->getValueString(ConfigKey("[Playlist]","Directory")));
    if (fd != "")
    {
        LineEditSongfiles->setText(fd);
    }
}

void DlgPrefPlaylist::slotApply()
{

    config->set(ConfigKey("[Promo]","StatTracking"),
                ConfigValue((int)checkBoxPromoStats->isChecked()));

    config->set(ConfigKey("[Library]","RescanOnStartup"),
                ConfigValue((int)checkBox_library_scan->isChecked()));

    config->set(ConfigKey("[Library]","WriteAudioTags"),
                ConfigValue((int)checkbox_ID3_sync->isChecked()));

    config->set(ConfigKey("[Library]","UseRelativePathOnExport"),
                ConfigValue((int)checkBox_use_relative_path->isChecked()));

    config->set(ConfigKey("[Library]","ShowRhythmboxLibrary"),
                ConfigValue((int)checkBox_show_rhythmbox->isChecked()));

    config->set(ConfigKey("[Library]","ShowITunesLibrary"),
                ConfigValue((int)checkBox_show_itunes->isChecked()));

    config->set(ConfigKey("[Library]","ShowTraktorLibrary"),
                ConfigValue((int)checkBox_show_traktor->isChecked()));

    config->Save();

    // Update playlist if path has changed
    if (LineEditSongfiles->text() != config->getValueString(ConfigKey("[Playlist]","Directory")))
    {
        // Check for valid directory and put up a dialog if invalid!!!

        config->set(ConfigKey("[Playlist]","Directory"), LineEditSongfiles->text());

        // Save preferences
        config->Save();

        // Emit apply signal
        emit(apply());
    }
}
