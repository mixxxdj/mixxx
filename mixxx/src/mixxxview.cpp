/***************************************************************************
                          mixxxview.cpp  -  description
                             -------------------
    begin                : Mon Feb 18 09:48:17 CET 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
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

#include "mixxxview.h"

#include <qtable.h>
#include <qdir.h>

MixxxView::MixxxView(QWidget *parent, MixxxDoc *doc) : QWidget(parent)
{
    /** connect doc with the view*/
    connect(doc, SIGNAL(documentChanged()), this, SLOT(slotDocumentChanged()));
    
    // Sub widgets
    playcontrol1 = new DlgPlaycontrol(this);
    playcontrol2 = new DlgPlaycontrol(this); playcontrol2->layoutMirror();
    channel1 = new DlgChannel(this);
    channel2 = new DlgChannel(this); channel2->layoutMirror();
    playlist = new DlgPlaylist(this);
    playlist->ListPlaylist->setColumnWidth(0,575);
    master = new DlgMaster(this);
    crossfader = new DlgCrossfader(this);
    split = new DlgSplit(this);

    // Layout management
    mainGrid = new QGridLayout(this,6,3); // A layout on a widget
    mainGrid->addWidget(playcontrol1,0,0);
    mainGrid->addWidget(channel1,0,1);
    mainGrid->addWidget(split,0,2);
    mainGrid->addWidget(channel2,0,3);
    mainGrid->addWidget(playcontrol2,0,4);
    mainGrid->addMultiCellWidget(master,0,2,5,5);
    mainGrid->addMultiCellWidget(crossfader,1,1,0,4);
    mainGrid->addMultiCellWidget(playlist,2,2,0,4);

    //let the ratio between the widths of columns 0 and 1 be 2:3.
    mainGrid->setColStretch( 0, 207);
    mainGrid->setColStretch( 1,  73);
    mainGrid->setColStretch( 2,  36);
    mainGrid->setColStretch( 3,  73);
    mainGrid->setColStretch( 4, 207);
    mainGrid->setColStretch( 5,  45);

    mainGrid->setRowStretch( 0, 230);
    mainGrid->setRowStretch( 1,  43);
    mainGrid->setRowStretch( 2, 150);

    // Add filenames in ./music/ to table
    Addfiles("music/");
}

void MixxxView::Addfiles(const char *dir_name) {
    QDir dir(dir_name);
    if (!dir.exists())
	qWarning( "Cannot find the directory %s.",dir_name);
    else {
	// First run though all directories:
	dir.setFilter(QDir::Dirs);
	const QFileInfoList dir_list = *dir.entryInfoList();
	QFileInfoListIterator dir_it(dir_list);
	QFileInfo *d;
	dir_it += 2; // Traverse past "." and ".."
	while ((d=dir_it.current())) {
	    qDebug(d->filePath());
	    Addfiles(d->filePath());
	    ++dir_it;
	}

	// ... and then all the files:
	dir.setFilter(QDir::Files);
    dir.setNameFilter("*.wav *.Wav *.WAV *.mp3 *.Mp3 *.MP3");
	const QFileInfoList *list = dir.entryInfoList();
	QFileInfoListIterator it(*list);        // create list iterator
	QFileInfo *fi;                          // pointer for traversing
	
	while ((fi=it.current()))
	{
	    //qDebug(fi->fileName());
	    playlist->ListPlaylist->insertItem(new QListViewItem(playlist->ListPlaylist,
								 fi->fileName(),fi->filePath()));
	    ++it;   // goto next list element
	}
    }
}

MixxxView::~MixxxView()
{
}

void MixxxView::slotDocumentChanged()
{
  //TODO update the view

}



