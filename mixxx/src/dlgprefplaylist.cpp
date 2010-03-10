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
#include "plugindownloader.h"
#include <QtCore>
#include <QtGui>

DlgPrefPlaylist::DlgPrefPlaylist(QWidget * parent, ConfigObject<ConfigValue> * _config) :  QWidget(parent), Ui::DlgPrefPlaylistDlg()
{
    config = _config;
    setupUi(this);
    slotUpdate();

    m_pPluginDownloader = new PluginDownloader(this);
    //If the M4A plugin is present on disk, check the checkbox.
    if (m_pPluginDownloader->checkForM4APlugin()) {
        pushButtonM4A->setChecked(true);
        pushButtonM4A->setEnabled(false);
        pushButtonM4A->setText("Installed");
    }

    // Connection
    connect(PushButtonBrowsePlaylist, SIGNAL(clicked()),       this,      SLOT(slotBrowseDir()));
    connect(LineEditSongfiles,        SIGNAL(returnPressed()), this,      SLOT(slotApply()));
    connect(pushButtonM4A, SIGNAL(clicked()), this, SLOT(slotM4ACheck()));

#ifdef __IPOD__
    // iPod related stuff
    connect(PushButtonDetectiPodMountPoint, SIGNAL(clicked()),  this,      SLOT(slotDetectiPodMountPoint()));
    connect(PushButtonBrowseiPodMountPoint, SIGNAL(clicked()), this,      SLOT(slotBrowseiPodMountPoint()));
    connect(LineEditiPodMountPoint,   SIGNAL(returnPressed()), this,      SLOT(slotApply()));
    groupBoxiPod->setVisible(true);
#endif
}

DlgPrefPlaylist::~DlgPrefPlaylist()
{
}

void DlgPrefPlaylist::slotM4ACheck()
{
    qDebug() << "slotM4ACheck";
    if (!m_pPluginDownloader->checkForM4APlugin())
    {
        m_pPluginDownloader->downloadM4APlugin();
    }
}

void DlgPrefPlaylist::slotUpdate()
{
    // Song path
    LineEditSongfiles->setText(config->getValueString(ConfigKey("[Playlist]","Directory")));
    // iPod mount point
    LineEditiPodMountPoint->setText(config->getValueString(ConfigKey("[iPod]","MountPoint")));
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

void DlgPrefPlaylist::slotDetectiPodMountPoint()
{
QString iPodMountPoint;
QFileInfoList mountpoints;
#ifdef __WINDOWS__
  // Windows iPod Detection
  mountpoints = QDir::drives();
#elif __LINUX__
  // Linux
  mountpoints = QDir("/media").entryInfoList();
  mountpoints += QDir("/mnt").entryInfoList();
#elif __OSX__
  // Mac OSX
  mountpoints = QDir("/Volumes").entryInfoList();
#endif

QListIterator<QFileInfo> i(mountpoints);
QFileInfo mp;
while (i.hasNext()) {
    mp = (QFileInfo) i.next();
    qDebug() << "mp:" << mp.filePath();
    if (QDir( QString(mp.filePath() + "/iPod_Control") ).exists() ) {
       qDebug() << "iPod found at" << mp.filePath(); 

       // Multiple iPods
       if (!iPodMountPoint.isEmpty()) {
         int ret = QMessageBox::warning(this, tr("Multiple iPods Detected"), 
                   tr("Mixxx has detected another iPod. \n\n")+
                   tr("Choose Yes to use the newly found iPod @ ")+ mp.filePath()+ 
                   tr(" or to continue to search for other iPods. \n")+
                   tr("Choose No to use the existing iPod @ ")+ iPodMountPoint+ 
                   tr( " and end detection. \n"),
                   QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
         if (ret == QMessageBox::No) {
           break;
         }
       }

       iPodMountPoint = mp.filePath() + "/";
    }
}
if (!iPodMountPoint.isEmpty()) {
  LineEditiPodMountPoint->setText(iPodMountPoint);
}
}

void DlgPrefPlaylist::slotBrowseiPodMountPoint()
{
    
    QString fd = QFileDialog::getExistingDirectory(this, tr("Choose iPod mount point"), 
                                                   config->getValueString(ConfigKey("[iPod]","MountPoint")));
    if (fd != "")
    {
        LineEditiPodMountPoint->setText(fd);
    }
}



void DlgPrefPlaylist::slotApply()
{

    // Update playlist if path has changed
    if (LineEditSongfiles->text() != config->getValueString(ConfigKey("[Playlist]","Directory")))
    {
        // Check for valid directory and put up a dialog if invalid!!!

        config->set(ConfigKey("[Playlist]","Directory"), LineEditSongfiles->text());

        // Save preferences
        config->Save();

        qDebug() << "FIXME: Probably want to clear the TrackCollection and library playlist when you" <<
                     "change library paths... (" << __FILE__ ", around line" << __LINE__;
                     

        // Emit apply signal
        emit(apply());
    }
    if (LineEditiPodMountPoint->text() != config->getValueString(ConfigKey("[iPod]","MountPoint")))
    {
        config->set(ConfigKey("[iPod]","MountPoint"), LineEditiPodMountPoint->text());
        // Save preferences
        config->Save();
        // Emit apply signal
        emit(apply());
    }
}
