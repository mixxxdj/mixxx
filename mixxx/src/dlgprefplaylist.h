/***************************************************************************
                          dlgprefplaylist.h  -  description
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

#ifndef DLGPREFPLAYLIST_H
#define DLGPREFPLAYLIST_H

#include "ui_dlgprefplaylistdlg.h"
#include "configobject.h"

class QWidget;
class PluginDownloader;
/**
  *@author Tue & Ken Haste Andersen
  */

class DlgPrefPlaylist : public QWidget, public Ui::DlgPrefPlaylistDlg  {
    Q_OBJECT
public:
    DlgPrefPlaylist(QWidget *parent, ConfigObject<ConfigValue> *_config);
    ~DlgPrefPlaylist();
public slots:
    /** Update widget */
    void slotUpdate();
    /** Dialog to browse for music file directory */
    void slotBrowseDir();
    /** Apply changes to widget */
    void slotApply();

    /** Dialog to browse for iPod mount point */
    void slotBrowseiPodMountPoint();

    /** Attempt to guess the iPod's mount point */
    void slotDetectiPodMountPoint();

    /** Starts up the PluginDownloader if the plugin isn't present */
    void slotM4ACheck();

signals:
    void apply();
private:
    /** Pointer to config object */
    ConfigObject<ConfigValue> *config;
    /** SoundSource Plugin Downloader */
    PluginDownloader* m_pPluginDownloader;
};

#endif
