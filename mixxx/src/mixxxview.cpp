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
	playcontrol2 = new DlgPlaycontrol(this);
	channel1 = new DlgChannel(this);
	channel2 = new DlgChannel(this);
	playlist = new DlgPlaylist(this);
    master = new DlgMaster(this);

	// Layout management
	mainGrid = new QGridLayout(this,3,4); // A layout on a widget
	//hMainBox->setResizeMode(QLayout::Minimum);
	mainGrid->addMultiCellWidget(channel1,0,1,0,0);
	mainGrid->addWidget(playcontrol1,0,1);
	mainGrid->addWidget(playcontrol2,0,2);
	mainGrid->addMultiCellWidget(channel2,0,1,3,3);
	mainGrid->addMultiCellWidget(playlist,1,1,1,2);
	mainGrid->addMultiCellWidget(master,2,2,1,2);

	//let the ratio between the widths of columns 0 and 1 be 2:3.
/*	mainGrid->setColStretch( 0, 150);
	mainGrid->setColStretch( 1, 310);
	mainGrid->setRowStretch( 0, 310);
	mainGrid->setRowStretch( 1, 210);
*/
	// Add filenames in ./music/ to table
	QDir d("music");
	d.setFilter(QDir::Files);
	if (!d.exists())
	    qWarning( "Cannot find the ./music directory" );
	else {
	    int i=0;
	    const QFileInfoList *list = d.entryInfoList();
	    QFileInfoListIterator it(*list);        // create list iterator
	    QFileInfo *fi;                          // pointer for traversing
	    
	    while ((fi=it.current()) && (i<playlist->TableList->numRows())) {             // for each file...
		qDebug(fi->fileName());
		playlist->TableList->setItem(i,0,new QTableItem(playlist->TableList,QTableItem::Never,
								fi->baseName()));
		playlist->TableList->setItem(i,1,new QTableItem(playlist->TableList,QTableItem::Never,
								fi->filePath()));
		++it;                               // goto next list element
		i++;
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
