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
#include <qlineedit.h>
#include <qfiledialog.h>
#include <qwidget.h>
#include <qpushbutton.h>

DlgPrefPlaylist::DlgPrefPlaylist(QWidget *parent, ConfigObject<ConfigValue> *_config) : DlgPrefPlaylistDlg(parent,"")
{
    config = _config;

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
    QFileDialog* fd = new QFileDialog(config->getValueString(ConfigKey("[Playlist]","Directory")),QString::null, this, QString::null, TRUE );
    fd->setMode( QFileDialog::Directory );
    fd->setCaption("Choose directory with music files");
    if ( fd->exec() == QDialog::Accepted )
    {
        LineEditSongfiles->setText( fd->selectedFile() );
        slotApply();
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

        // Emit apply signal
        emit(apply(LineEditSongfiles->text()));
    }
}
