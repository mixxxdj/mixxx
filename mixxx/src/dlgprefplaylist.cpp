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
#include <QtCore>
#include <QtGui>

DlgPrefPlaylist::DlgPrefPlaylist(QWidget * parent, ConfigObject<ConfigValue> * _config) :  QWidget(parent), Ui::DlgPrefPlaylistDlg()
{
    config = _config;
    setupUi(this);
    slotUpdate();

    // Connection
    connect(PushButtonBrowsePlaylist, SIGNAL(clicked()),       this,      SLOT(slotBrowseDir()));
    connect(LineEditSongfiles,        SIGNAL(returnPressed()), this,      SLOT(slotApply()));
}

DlgPrefPlaylist::~DlgPrefPlaylist()
{
}

void DlgPrefPlaylist::slotUpdate()
{
    // Song path
    LineEditSongfiles->setText(config->getValueString(ConfigKey("[Playlist]","Directory")));
}

void DlgPrefPlaylist::slotBrowseDir()
{
    QString fd = QFileDialog::getExistingDirectory(this, "Choose music library directory", 
                                                   config->getValueString(ConfigKey("[Playlist]","Directory")));
    if (fd != "")
    {
        LineEditSongfiles->setText(fd);
        //slotApply();
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
}
