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

#include <QDesktopServices>
#include <QFileDialog>
#include <QStringList>
#include <QUrl>

#include "dlgprefplaylist.h"
#include "soundsourceproxy.h"
#ifdef __PROMO__
#include "library/promotracksfeature.h"
#endif
//#include "plugindownloader.h"

#define MIXXX_ADDONS_URL "http://www.mixxx.org/wiki/doku.php/add-ons"


DlgPrefPlaylist::DlgPrefPlaylist(QWidget * parent, ConfigObject<ConfigValue> * config,
                                 Library *pLibrary)
               : QWidget(parent), 
                 m_dirListModel(),
                 m_pconfig(config),
                 m_pLibrary(pLibrary) {
    setupUi(this);
    slotUpdate();
    checkbox_ID3_sync->setVisible(false);

    connect(this, SIGNAL(requestAddDir(QString)),
            m_pLibrary, SLOT(slotRequestAddDir(QString)));
    connect(this, SIGNAL(requestRemoveDir(QString)),
            m_pLibrary, SLOT(slotRequestRemoveDir(QString)));
    connect(this, SIGNAL(requestRelocateDir(QString,QString)),
            m_pLibrary, SLOT(slotRequestRelocateDir(QString,QString)));
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

    connect(PushButtonAddDir, SIGNAL(clicked()),
            this, SLOT(slotAddDir()));
    connect(PushButtonRemoveDir, SIGNAL(clicked()),
            this, SLOT(slotRemoveDir()));
    connect(PushButtonRelocateDir, SIGNAL(clicked()),
            this, SLOT(slotRelocateDir()));
    //connect(pushButtonM4A, SIGNAL(clicked()), this, SLOT(slotM4ACheck()));
    connect(pushButtonExtraPlugins, SIGNAL(clicked()),
            this, SLOT(slotExtraPlugins()));

    bool enablePromoGroupbox = false;
#ifdef __PROMO__
    enablePromoGroupbox = PromoTracksFeature::isSupported(config);
#endif
    if (!enablePromoGroupbox) {
        groupBoxBundledSongs->hide();
    }

    // plugins are loaded in src/main.cpp way early in boot so this is safe
    // here, doesn't need done at every slotUpdate
    QStringList plugins(SoundSourceProxy::supportedFileExtensionsByPlugins());
    if (plugins.length() > 0) {
        pluginsLabel->setText(plugins.join(", "));
    }
}

DlgPrefPlaylist::~DlgPrefPlaylist() {
}

void DlgPrefPlaylist::initialiseDirList(){
    // save which index was selected
    const QString selected = dirList->currentIndex().data().toString();
    // clear and fill model
    m_dirListModel.clear();
    QStringList dirs = m_pLibrary->getDirs();
    foreach (QString dir, dirs) {
        m_dirListModel.appendRow(new QStandardItem(dir));
    }
    dirList->setModel(&m_dirListModel);
    // reselect index if it still exists
    dirList->setCurrentIndex(m_dirListModel.index(0, 0));
    for (int i=0 ; i<m_dirListModel.rowCount() ; ++i) {
        const QModelIndex index = m_dirListModel.index(i, 0);
        if (index.data().toString() == selected) {
            dirList->setCurrentIndex(index);
            break;
        }
    }
}

void DlgPrefPlaylist::slotExtraPlugins() {
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

void DlgPrefPlaylist::slotUpdate() {
    initialiseDirList();
    //Bundled songs stat tracking
    checkBoxPromoStats->setChecked((bool)m_pconfig->getValueString(
            ConfigKey("[Promo]","StatTracking")).toInt());
    checkBox_library_scan->setChecked((bool)m_pconfig->getValueString(
            ConfigKey("[Library]","RescanOnStartup")).toInt());
    checkbox_ID3_sync->setChecked((bool)m_pconfig->getValueString(
            ConfigKey("[Library]","WriteAudioTags")).toInt());
    checkBox_use_relative_path->setChecked((bool)m_pconfig->getValueString(
            ConfigKey("[Library]","UseRelativePathOnExport")).toInt());
    checkBox_show_rhythmbox->setChecked((bool)m_pconfig->getValueString(
            ConfigKey("[Library]","ShowRhythmboxLibrary"),"1").toInt());
    checkBox_show_itunes->setChecked((bool)m_pconfig->getValueString(
            ConfigKey("[Library]","ShowITunesLibrary"),"1").toInt());
    checkBox_show_traktor->setChecked((bool)m_pconfig->getValueString(
            ConfigKey("[Library]","ShowTraktorLibrary"),"1").toInt());
}

void DlgPrefPlaylist::slotAddDir() {
    QString fd = QFileDialog::getExistingDirectory(
                        this, tr("Choose a music library directory"),
                        QDesktopServices::storageLocation(QDesktopServices::MusicLocation));
    if ( !fd.isEmpty() ) {
        emit(requestAddDir(fd));
        slotUpdate();
    }
}

void DlgPrefPlaylist::slotRemoveDir() {
    QModelIndex index = dirList->currentIndex();
    QString fd = index.data().toString();
    emit(requestRemoveDir(fd));
    slotUpdate();
}

void DlgPrefPlaylist::slotRelocateDir() {
    QModelIndex index = dirList->currentIndex();
    QString currentFd = index.data().toString();
    QString fd = QFileDialog::getExistingDirectory(
                        this, tr("Choose a  music library directory"),
                        QDesktopServices::storageLocation(QDesktopServices::MusicLocation));

    if (!fd.isEmpty()) {
        emit(requestRelocateDir(currentFd,fd));
        slotUpdate();
    }
}

void DlgPrefPlaylist::slotApply() {
    m_pconfig->set(ConfigKey("[Promo]","StatTracking"),
                ConfigValue((int)checkBoxPromoStats->isChecked()));
    m_pconfig->set(ConfigKey("[Library]","RescanOnStartup"),
                ConfigValue((int)checkBox_library_scan->isChecked()));
    m_pconfig->set(ConfigKey("[Library]","WriteAudioTags"),
                ConfigValue((int)checkbox_ID3_sync->isChecked()));
    m_pconfig->set(ConfigKey("[Library]","UseRelativePathOnExport"),
                ConfigValue((int)checkBox_use_relative_path->isChecked()));
    m_pconfig->set(ConfigKey("[Library]","ShowRhythmboxLibrary"),
                ConfigValue((int)checkBox_show_rhythmbox->isChecked()));
    m_pconfig->set(ConfigKey("[Library]","ShowITunesLibrary"),
                ConfigValue((int)checkBox_show_itunes->isChecked()));
    m_pconfig->set(ConfigKey("[Library]","ShowTraktorLibrary"),
                ConfigValue((int)checkBox_show_traktor->isChecked()));

    m_pconfig->Save();
}
